// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Copyright (c) 2017 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_KEY_H
#define BITCOIN_KEY_H

#include <pubkey.h>
#include <serialize.h>
#include <support/allocators/secure.h>
#include <uint256.h>

#include <stdexcept>
#include <vector>
#pragma once

<<<<<<< HEAD
#include <dilithium/sign.h>
#include <dilithium/randombytes.h>

    /**
=======
typedef std::vector<unsigned char, secure_allocator<unsigned char>> CPrivKey;

/**
 * secure_allocator is defined in allocators.h
>>>>>>> 84ee8abc52d4b0b0dcb0fabcfbc27834666de93a
 * CPrivKey is a serialized private key, with all parameters included
 * (SIZE bytes)
 */


/** An encapsulated private key. */
class CKey
{
public:
    /**
     * secp256k1:
     */
<<<<<<< HEAD
    //static const unsigned int SIZE            = 279;
=======
    static const unsigned int SIZE = 279;
>>>>>>> 84ee8abc52d4b0b0dcb0fabcfbc27834666de93a
    static const unsigned int COMPRESSED_SIZE = 214;
    static const unsigned int SIZE = 1312;
    /**
     * see www.keylength.com
     * script supports up to 75 for single byte push
     */

    /*
    static_assert(
        SIZE >= COMPRESSED_SIZE,
        "COMPRESSED_SIZE is larger than SIZE");
        */

private:
    //! Whether this private key is valid. We check for correctness when modifying the key
    //! data, so fValid should always correspond to the actual state.
    bool fValid;

    //! Whether the public key corresponding to this private key is (to be) compressed.
    //bool fCompressed;

    //! The actual byte data
    std::vector<unsigned char, secure_allocator<unsigned char>> keydata;

    //! Check whether the 32-byte array pointed to by vch is valid keydata.
    bool static Check(const unsigned char* vch);

public:
    //! Construct an invalid private key.
    CKey() : fValid(false)//, fCompressed(false)
    {
        // Important: vch must be 32 bytes in length to not break serialization
        //keydata.resize(32);
        keydata.resize(164);
    }

    friend bool operator==(const CKey& a, const CKey& b)
    {
<<<<<<< HEAD
        return /* a.fCompressed == b.fCompressed &&*/
            a.size() == b.size() &&
            memcmp(a.keydata.data(), b.keydata.data(), a.size()) == 0;
=======
        return a.fCompressed == b.fCompressed &&
               a.size() == b.size() &&
               memcmp(a.keydata.data(), b.keydata.data(), a.size()) == 0;
>>>>>>> 84ee8abc52d4b0b0dcb0fabcfbc27834666de93a
    }

    //! Initialize using begin and end iterators to byte data.
    template <typename T>
    void Set(const T pbegin, const T pend, bool fCompressedIn)
    {
        if (size_t(pend - pbegin) != keydata.size()) {
            fValid = false;
        } else if (Check(&pbegin[0])) {
            memcpy(keydata.data(), (unsigned char*)&pbegin[0], keydata.size());
            fValid = true;
            //fCompressed = fCompressedIn;
            fConpressed = false;
        } else {
            fValid = false;
        }
    }

    //! Simple read-only vector-like interface.
    unsigned int size() const { return (fValid ? keydata.size() : 0); }
    const unsigned char* begin() const { return keydata.data(); }
    const unsigned char* end() const { return keydata.data() + size(); }

    //! Check whether this private key is valid.
    bool IsValid() const { return fValid; }

    //! Check whether the public key corresponding to this private key is (to be) compressed.
    //bool IsCompressed() const { return fCompressed; }

    //! Generate a new private key using a cryptographic PRNG.
    void MakeNewKey(bool fCompressed);

    //! Negate private key
    bool Negate();

    /**
     * Convert the private key to a CPrivKey (serialized OpenSSL private key data).
     * This is expensive.
     */
    CPrivKey GetPrivKey() const;

    /**
     * Compute the public key from a private key.
     * This is expensive.
     */
    CPubKey GetPubKey() const;

    /**
     * Create a DER-serialized signature.
     * The test_case parameter tweaks the deterministic nonce.
     */
    //bool Sign(const uint256& hash, std::vector<unsigned char>& vchSig, bool grind = true, uint32_t test_case = 0) const;
    bool Sign(const uint512& hash, std::vector<unsigned char>& vchSig, bool grind = true, uint32_t test_case = 0) const;

    /**
     * Create a compact signature (65 bytes), which allows reconstructing the used public key.
     * The format is one header byte, followed by two times 32 bytes for the serialized r and s values.
     * The header byte: 0x1B = first key with even y, 0x1C = first key with odd y,
     *                  0x1D = second key with even y, 0x1E = second key with odd y,
     *                  add 0x04 for compressed keys.
     */
<<<<<<< HEAD

    //bool SignCompact(const uint256& hash, std::vector<unsigned char>& vchSig) const;
=======
    /*bool SignCompact(const uint256& hash, std::vector<unsigned char>& vchSig) const;*/
>>>>>>> 84ee8abc52d4b0b0dcb0fabcfbc27834666de93a

    /**
     * Create a BIP-340 Schnorr signature, for the xonly-pubkey corresponding to *this,
     * optionally tweaked by *merkle_root. Additional nonce entropy can be provided through
     * aux.
     *
     * merkle_root is used to optionally perform tweaking of the private key, as specified
     * in BIP341:
     * - If merkle_root == nullptr: no tweaking is done, sign with key directly (this is
     *                              used for signatures in BIP342 script).
     * - If merkle_root->IsNull():  sign with key + H_TapTweak(pubkey) (this is used for
     *                              key path spending when no scripts are present).
     * - Otherwise:                 sign with key + H_TapTweak(pubkey || *merkle_root)
     *                              (this is used for key path spending, with specific
     *                              Merkle root of the script tree).
     */
<<<<<<< HEAD

    //bool SignSchnorr(const uint256& hash, Span<unsigned char> sig, const uint256* merkle_root = nullptr, const uint256* aux = nullptr) const;

=======
    bool SignSchnorr(const uint256& hash, Span<unsigned char> sig, const uint256* merkle_root = nullptr, const uint256* aux = nullptr) const;
   
>>>>>>> 84ee8abc52d4b0b0dcb0fabcfbc27834666de93a
    //! Derive BIP32 child key.
    bool Derive(CKey& keyChild, ChainCode& ccChild, unsigned int nChild, const ChainCode& cc) const;

    /**
     * Verify thoroughly whether a private key and a public key match.
     * This is done using a different mechanism than just regenerating it.
     */
    bool VerifyPubKey(const CPubKey& vchPubKey) const;

    //! Load private key and check that public key matches.
    bool Load(const CPrivKey& privkey, const CPubKey& vchPubKey, bool fSkipCheck);
};

struct CExtKey {
    unsigned char nDepth;
    unsigned char vchFingerprint[4];
    unsigned int nChild;
    ChainCode chaincode;
    CKey key;

    friend bool operator==(const CExtKey& a, const CExtKey& b)
    {
        return a.nDepth == b.nDepth &&
               memcmp(a.vchFingerprint, b.vchFingerprint, sizeof(vchFingerprint)) == 0 &&
               a.nChild == b.nChild &&
               a.chaincode == b.chaincode &&
               a.key == b.key;
    }

    void Encode(unsigned char code[BIP32_EXTKEY_SIZE]) const;
    void Decode(const unsigned char code[BIP32_EXTKEY_SIZE]);
    bool Derive(CExtKey& out, unsigned int nChild) const;
    CExtPubKey Neuter() const;
    void SetSeed(const unsigned char* seed, unsigned int nSeedLen);
};

/** Initialize the elliptic curve support. May not be called twice without calling ECC_Stop first. */
//void ECC_Start();

/** Deinitialize the elliptic curve support. No-op if ECC_Start wasn't called first. */
//void ECC_Stop();

/** Check that required EC support is available at runtime. */
//bool ECC_InitSanityCheck();

#endif // BITCOIN_KEY_H









class CBOBKey
{
public:
    /**
	* ZZang
	* NTRU:
	* private key는 실제 seed 값임. 
	* 전자서명시 seed 값을 이용하여 진짜 개인키와 공개키를 생성 해야함.
	* seed값 인 개인키 길이: 32byte (256 bit)
	* 실제 개인키 길이: 2604 byte
	* 공개키 길이: 2065 byte
	*/
    static const unsigned int SIZE = 279;
    static const unsigned int COMPRESSED_SIZE = 214;
    /**
     * see www.keylength.com
     * script supports up to 75 for single byte push
     */
    static_assert(
        SIZE >= COMPRESSED_SIZE,
        "COMPRESSED_SIZE is larger than SIZE");

private:
    //! Whether this private key is valid. We check for correctness when modifying the key
    //! data, so fValid should always correspond to the actual state.
    bool fValid;

    //! Whether the public key corresponding to this private key is (to be) compressed.
    //bool fCompressed;

    //! The actual byte data
    std::vector<unsigned char, secure_allocator<unsigned char>> keydata;

    //! Check whether the 32-byte array pointed to by vch is valid keydata.
    bool static Check(const unsigned char* vch);

    bool Negate();

    bool fCompressed;

public:
    //! Construct an invalid private key.
    CBOBKey() : fValid(false) /*, fCompressed(false)*/
    {
        // Important: vch must be 32 bytes in length to not break serialization
        keydata.resize(32);
    }

    friend bool operator==(const CBOBKey& a, const CBOBKey& b)
    {
        return /*a.fCompressed == b.fCompressed &&*/
            a.size() == b.size() &&
            memcmp(a.keydata.data(), b.keydata.data(), a.size()) == 0;
    }

    //! Initialize using begin and end iterators to byte data.
    template <typename T>
    void Set(const T pbegin, const T pend, bool fCompressedIn)
    {
        if (size_t(pend - pbegin) != keydata.size()) {
            fValid = false;
        } else if (Check(&pbegin[0])) {
            memcpy(keydata.data(), (unsigned char*)&pbegin[0], keydata.size());
            fValid = true;
            /*fCompressed = fCompressedIn;*/
        } else {
            fValid = false;
        }
    }

    //! Simple read-only vector-like interface.
    unsigned int size() const { return (fValid ? keydata.size() : 0); }
    const unsigned char* begin() const { return keydata.data(); }
    const unsigned char* end() const { return keydata.data() + size(); }

    //! Check whether this private key is valid.
    bool IsValid() const { return fValid; }

    //! Check whether the public key corresponding to this private key is (to be) compressed.
    bool IsCompressed() const { return 1; /*fCompressed;*/ }

    //! Generate a new private key using a cryptographic PRNG.
    void MakeNewKey(/*bool fCompressed*/);

    /**
	* Convert the private key to a CPrivKey (serialized OpenSSL private key data).
	* This is expensive.
	*/
    CPrivKey GetPrivKey() const;

    /**
	* Compute the public key from a private key.
	* This is expensive.
	*/
    CBOBPubKey GetPubKey() const;

    /**
	* Create a DER-serialized signature.
	* The test_case parameter tweaks the deterministic nonce.
	*/
    bool Sign(const uint256& hash, std::vector<unsigned char>& vchSig, bool grind = true, uint32_t test_case = 0) const;
    /**
	* Create a compact signature (65 bytes), which allows reconstructing the used public key.
	* The format is one header byte, followed by two times 32 bytes for the serialized r and s values.
	* The header byte: 0x1B = first key with even y, 0x1C = first key with odd y,
	*                  0x1D = second key with even y, 0x1E = second key with odd y,
	*                  add 0x04 for compressed keys.
	*/
    /*bool SignCompact(const uint256& hash, std::vector<unsigned char>& vchSig) const;*/

    //! Derive BIP32 child key.
    bool Derive(CBOBKey& keyChild, ChainCode& ccChild, unsigned int nChild, const ChainCode& cc) const;

    /**
	* Verify thoroughly whether a private key and a public key match.
	* This is done using a different mechanism than just regenerating it.
	*/
    bool VerifyPubKey(const CBOBPubKey& vchPubKey) const;

    //! Load private key and check that public key matches.
    bool Load(CPrivKey& privkey, CBOBPubKey& vchPubKey, bool fSkipCheck);
};
