// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERPUBKEY_H__
#define __ELASTOS_SDK_MASTERPUBKEY_H__

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "BRBIP32Sequence.h"

#include "Wrapper.h"
#include "Key.h"
#include "CMemBlock.h"

namespace Elastos {
	namespace ElaWallet {

		class MasterPubKey :
			public Wrapper<BRMasterPubKey> {
		public:

			MasterPubKey();

			MasterPubKey(const std::string &phrase, const std::string &phrasePassword = "");

			MasterPubKey(const CMBlock &pubKey, const UInt256 &chainCode);

			MasterPubKey(const BRKey &key, const UInt256 &chainCode);

			virtual std::string toString() const;

			virtual BRMasterPubKey *getRaw() const;

			CMBlock serialize() const;

			void deserialize(const CMBlock &data);

			CMBlock getPubKey() const;

			boost::shared_ptr<Key> getPubKeyAsKey() const;

			const UInt256 &getChainCode() const;

		public:
			static CMBlock bip32BitIDKey(const CMBlock &seed, int index, const std::string &uri);

			static bool validateRecoveryPhrase(const std::vector<std::string> &words, const std::string &phrase);

			static std::string generatePaperKey(const UInt128 &seed, const std::vector<std::string> &words);

			static size_t BIP32PubKey(uint8_t *pubKey, size_t pubKeyLen, BRMasterPubKey mpk, uint32_t chain,
			                          uint32_t index);

		private:
			boost::shared_ptr<BRMasterPubKey> _masterPubKey;
		};

		typedef boost::shared_ptr<MasterPubKey> MasterPubKeyPtr;

	}
}

#endif //__ELASTOS_SDK_MASTERPUBKEY_H__
