// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/scoped_ptr.hpp>

#include "BRKey.h"
#include "BRArray.h"

#include "SubWallet.h"
#include "MasterWallet.h"
#include "SubWalletCallback.h"

namespace fs = boost::filesystem;

#define DB_FILE_EXTENSION ".db"

namespace Elastos {
	namespace SDK {

		SubWallet::SubWallet(const CoinInfo &info,
							 const ChainParams &chainParams,
							 const std::string &payPassword,
							 MasterWallet *parent) :
				_parent(parent),
				_info(info) {

			fs::path subWalletDbPath = _parent->_dbRoot;
			subWalletDbPath /= _parent->_name + info.getChainId() + DB_FILE_EXTENSION;

			BRKey key;
			UInt256 chainCode;
			deriveKeyAndChain(&key, chainCode, payPassword);
			MasterPubKeyPtr masterPubKey(new MasterPubKey(key, chainCode));

			_walletManager = WalletManagerPtr(
					new WalletManager(masterPubKey, subWalletDbPath, _info.getEarliestPeerTime(),
									  _info.getSingleAddress(), _info.getForkId(), chainParams));
			_walletManager->registerWalletListener(this);
		}

		SubWallet::~SubWallet() {

		}

		nlohmann::json SubWallet::GetBalanceInfo() {
			//todo complete me
			return nlohmann::json();
		}

		uint64_t SubWallet::GetBalance() {
			return _walletManager->getWallet()->getBalance();
		}

		std::string SubWallet::CreateAddress() {
			return _walletManager->getWallet()->getReceiveAddress();
		}

		nlohmann::json SubWallet::GetAllAddress(uint32_t start,
												uint32_t count) {
			std::vector<std::string> addresses = _walletManager->getWallet()->getAllAddresses();
			std::vector<std::string> results(addresses.begin() + start, addresses.begin() + start + count);
			nlohmann::json j;
			j["addresses"] = addresses;
			return j;
		}

		double SubWallet::GetBalanceWithAddress(const std::string &address) {
			//todo complete me
			return 0;
		}

		void SubWallet::AddCallback(ISubWalletCallback *subCallback) {
			_callbacks.push_back(subCallback);
		}

		void SubWallet::RemoveCallback(ISubWalletCallback *subCallback) {
			_callbacks.erase(std::remove(_callbacks.begin(), _callbacks.end(), subCallback), _callbacks.end());
		}

		std::string
		SubWallet::SendTransaction(const std::string &fromAddress, const std::string &toAddress, double amount,
								   double fee, const std::string &payPassword, const std::string &memo) {
			boost::scoped_ptr<TxParam> txParam(
					TxParamFactory::createTxParam(fromAddress, toAddress, amount, fee, memo));
			TransactionPtr transaction = _walletManager->createTransaction(*txParam);

			BRTransaction *rawTransaction = transaction->convertToRaw();
			signTransaction(rawTransaction, _info.getForkId(), payPassword);
			_walletManager->getPeerManager()->publishTransaction(transaction);

			Transaction txForHash(rawTransaction);
			return std::string((char *) txForHash.getHash().u8, 32);
		}

		nlohmann::json SubWallet::GetAllTransaction(uint32_t start, uint32_t count, const std::string &addressOrTxid) {
			return nlohmann::json();
		}

		std::string SubWallet::Sign(const std::string &message, const std::string &payPassword) {
			return "";
		}

		nlohmann::json
		SubWallet::CheckSign(const std::string &address, const std::string &message, const std::string &signature) {
			return nlohmann::json();
		}

		void SubWallet::balanceChanged(uint64_t balance) {

		}

		void SubWallet::onTxAdded(Transaction *transaction) {
			//todo add confirm count
			std::for_each(_callbacks.begin(), _callbacks.end(), [transaction](ISubWalletCallback *callback) {
				callback->OnTransactionStatusChanged(std::string((char *) transaction->getHash().u8, 32),
													 SubWalletCallback::convertToString(SubWalletCallback::Added),
													 nlohmann::json(), 0);
			});
		}

		void SubWallet::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			//todo add confirm count
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&hash, blockHeight, timeStamp](ISubWalletCallback *callback) {
							  callback->OnTransactionStatusChanged(hash, SubWalletCallback::convertToString(
									  SubWalletCallback::Updated), nlohmann::json(), 0);
						  });
		}

		void SubWallet::onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {
			//todo add confirm count
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&hash, notifyUser, recommendRescan](ISubWalletCallback *callback) {
							  callback->OnTransactionStatusChanged(hash, SubWalletCallback::convertToString(
									  SubWalletCallback::Added), nlohmann::json(), 0);
						  });
		}

		void SubWallet::recover(int limitGap) {
			_walletManager->recover(limitGap);
		}

		void SubWallet::deriveKeyAndChain(BRKey *key, UInt256 &chainCode, const std::string &payPassword) {
			UInt512 seed = _parent->deriveSeed(payPassword);
			Key::deriveKeyAndChain(key, chainCode, &seed, sizeof(seed), 3, 44, _info.getIndex(), 0);
		}

		void SubWallet::signTransaction(BRTransaction *transaction, int forkId, const std::string &payPassword) {
			BRKey masterKey;
			UInt256 chainCode;
			deriveKeyAndChain(&masterKey, chainCode, payPassword);
			BRWallet *wallet = _walletManager->getWallet()->getRaw();

			uint32_t j, internalIdx[transaction->inCount], externalIdx[transaction->inCount];
			size_t i, internalCount = 0, externalCount = 0;
			int r = 0;

			assert(wallet != nullptr);
			assert(transaction != nullptr);

			pthread_mutex_lock(&wallet->lock);

			for (i = 0; i < transaction->inCount; i++) {
				for (j = (uint32_t) array_count(wallet->internalChain); j > 0; j--) {
					if (BRAddressEq(transaction->inputs[i].address, &wallet->internalChain[j - 1]))
						internalIdx[internalCount++] = j - 1;
				}

				for (j = (uint32_t) array_count(wallet->externalChain); j > 0; j--) {
					if (BRAddressEq(transaction->inputs[i].address, &wallet->externalChain[j - 1]))
						externalIdx[externalCount++] = j - 1;
				}
			}

			pthread_mutex_unlock(&wallet->lock);

			BRKey keys[internalCount + externalCount];
			Key::calculatePrivateKeyList(keys, internalCount, &masterKey.secret, &chainCode,
										 SEQUENCE_INTERNAL_CHAIN, internalIdx);
			Key::calculatePrivateKeyList(&keys[internalCount], externalCount, &masterKey.secret, &chainCode,
										 SEQUENCE_EXTERNAL_CHAIN, externalIdx);

			BRTransactionSign(transaction, forkId, keys, internalCount + externalCount);
			for (i = 0; i < internalCount + externalCount; i++) BRKeyClean(&keys[i]);
		}

		std::string SubWallet::CreateMultiSignAddress(const nlohmann::json &multiPublicKeyJson, uint32_t totalSignNum,
													  uint32_t requiredSignNum) {
			//todo complete me
			return "";
		}

		nlohmann::json
		SubWallet::GenerateMultiSignTransaction(const std::string &fromAddress, const std::string &toAddress,
												uint64_t amount, uint64_t fee, const std::string &payPassword,
												const std::string &memo) {
			//todo complete me
			return nlohmann::json();
		}

		std::string
		SubWallet::SendRawTransaction(const nlohmann::json &transactionJson, const nlohmann::json &signJson) {
			//todo complete me
			return "";
		}

	}
}