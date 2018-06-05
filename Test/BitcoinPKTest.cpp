// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <fstream>
#include <catch.hpp>

#include "Wallet_Tool.h"
#include "BTCBase58.h"

using namespace Elastos::SDK;

TEST_CASE("bitcoin keystore with special phrase and no passphrase", "[bitcoin test]") {

	const char *pPhrase = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";

	CMemBlock<char> phrase;
	phrase.SetMemFixed(pPhrase, strlen(pPhrase) + 1);
	bool isValid = Wallet_Tool::EnPhraseIsValid(phrase);

	std::string prikey_encode_base58 = Wallet_Tool::getDeriveKey_base58(phrase, "");

	CMemBlock<unsigned char> prikey = BTCBase58::DecodeBase58(prikey_encode_base58);
	std::vector<uint8_t> arr_prikey;
	for (size_t i = 0; i < prikey.GetSize(); i++) {
		arr_prikey.push_back(uint8_t(prikey[i]));
	}

	uint8_t destination[] = {
		94, 176, 11, 189, 220, 240, 105, 8, 72, 137, 168, 171, 145, 85, 86, 129, 101, 245, 196, 83, 204, 184, 94, 112,
		129, 26, 174, 214, 246, 218, 95, 193, 154, 90, 196, 11, 56, 156, 211, 112, 208, 134, 32, 109, 236, 138, 166,
		196, 61, 174, 166, 105, 15, 32, 173, 61, 141, 72, 178, 210, 206, 158, 56, 228
	};

	const uint8_t *pSrc = arr_prikey.data();
	size_t szSrc = arr_prikey.size();
	REQUIRE(0 == memcmp(pSrc, destination, szSrc));

	int i = 0;
}

TEST_CASE("bitcoin keystore with entropy", "[bitcoin test]") {
	CMemBlock<uint8_t> seed = Wallet_Tool::GenerateSeed256();
	std::string entropySource = Wallet_Tool::GenerateEntropySource(seed);
	std::string prikey_base58 = Wallet_Tool::getDeriveKeyFromEntropySource_base58(entropySource);

	int i = 0;
}

TEST_CASE("bitcoin keystore with special entropy", "[bitcoin test]") {
	const char *entropySource = "c4dd4e848e5e2909dbb6eb80db222b2a4f716569e63c0b8605918f050b3002dc";
	std::string prikey_base58 = Wallet_Tool::getDeriveKeyFromEntropySource_base58(entropySource);

	int i = 0;
}

TEST_CASE("unicode to utf8 convert", "[bitcoin test]") {

	char in_utf8[] = u8"哈";
	char16_t in_utf16[] = u"哈";

	CMemBlock<char> cb_in_u8;
	cb_in_u8.SetMemFixed(in_utf8, sizeof(in_utf8));
	CMemBlock<char> u16 = Wallet_Tool::U8ToU16(cb_in_u8);

	CMemBlock<char> cb_in_u16;
	cb_in_u16.SetMemFixed((char *) in_utf16, sizeof(in_utf16));
	CMemBlock<char> u8 = Wallet_Tool::U16ToU8(cb_in_u16);
	printf("utf16-->utf8 out=%s\n", (char *) u8);
	bool bEqual = 0 == memcmp(u8, in_utf8, strlen(in_utf8));

	int pause = 0;
}

TEST_CASE("Seed serialize/deserialize", "[bitcoin test]") {

	CMemBlock<uint8_t> seed = Wallet_Tool::GenerateSeed256();
	std::string str_seed = Wallet_Tool::getStringFromSeed(seed);
	CMemBlock<uint8_t> seed_re = Wallet_Tool::getSeedFromString(str_seed);
	REQUIRE(0 == memcmp(seed_re, seed, seed.GetSize()));

	int pause = 0;
}

TEST_CASE("bitcoin keystore with generating phrase", "[bitcoin test]") {

	CMemBlock<uint8_t> seed = Wallet_Tool::GenerateSeed128();

	CMemBlock<char> phrase = Wallet_Tool::GenerateEnPhraseFromSeed(seed);

	bool bValid = Wallet_Tool::EnPhraseIsValid(phrase);
	std::string str_phrase = (const char *) phrase;

	CMemBlock<uint8_t> seed_re = Wallet_Tool::getSeedFromEnPhrase(phrase);
	REQUIRE(0 == memcmp(seed_re, seed, seed.GetSize()));

	std::string prikey = Wallet_Tool::getDeriveKey_base58(phrase, "");

	int i = 0;
}

TEST_CASE("bitcoin keystore in chinese with generating phrase and no passphrase", "[bitcoin test]") {

	CMemBlock<uint8_t> seed = Wallet_Tool::GenerateSeed128();

	CMemBlock<char> phrase = Wallet_Tool::GenerateChPhraseFromSeed(seed);

	bool bValid = Wallet_Tool::ChPhraseIsValid(phrase);
	std::string str_phrase = (const char *) phrase;

	CMemBlock<uint8_t> seed_re = Wallet_Tool::getSeedFromChPhrase(phrase);
	REQUIRE(0 == memcmp(seed_re, seed, seed.GetSize()));

	std::string prikey = Wallet_Tool::getDeriveKey_base58(phrase, "");

	int i = 0;
}