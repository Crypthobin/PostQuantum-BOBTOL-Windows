// Copyright (c) 2009-2020 The Bitcoin Core developers
// Copyright (c) 2017 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pubkey.h>

#include <hash.h>
#include <secp256k1.h>
#include <secp256k1_extrakeys.h>
#include <secp256k1_recovery.h>
#include <secp256k1_schnorrsig.h>
#include <span.h>
#include <uint256.h>

#include <algorithm>
#include <cassert>

#include "dilithium/fips202.h"
#include "dilithium/pqsign.h"
#include "dilithium/randombytes.h"

namespace {
/* Global secp256k1_context object used for verification. */
secp256k1_context* secp256k1_context_verify = nullptr;
} // namespace

/** This function is taken from the libsecp256k1 distribution and implements
 *  DER parsing for ECDSA signatures, while supporting an arbitrary subset of
 *  format violations.
 *
 *  Supported violations include negative integers, excessive padding, garbage
 *  at the end, and overly long length descriptors. This is safe to use in
 *  Bitcoin because since the activation of BIP66, signatures are verified to be
 *  strict DER before being passed to this module, and we know it supports all
 *  violations present in the blockchain before that point.
 */

XOnlyPubKey::XOnlyPubKey(Span<const unsigned char> bytes)
{
    assert(bytes.size() == 32);
    std::copy(bytes.begin(), bytes.end(), m_keydata.begin());
}

std::vector<CKeyID> XOnlyPubKey::GetKeyIDs() const
{
    std::vector<CKeyID> out;
    // For now, use the old full pubkey-based key derivation logic. As it is indexed by
    // Hash160(full pubkey), we need to return both a version prefixed with 0x02, and one
    // with 0x03.
    unsigned char b[33] = {0x02};
    std::copy(m_keydata.begin(), m_keydata.end(), b + 1);
    CPubKey fullpubkey;
    fullpubkey.Set(b, b + 33);
    out.push_back(fullpubkey.GetID());
    b[0] = 0x03;
    fullpubkey.Set(b, b + 33);
    out.push_back(fullpubkey.GetID());
    return out;
}

bool XOnlyPubKey::IsFullyValid() const
{
    secp256k1_xonly_pubkey pubkey;
    return secp256k1_xonly_pubkey_parse(secp256k1_context_verify, &pubkey, m_keydata.data());
}

bool XOnlyPubKey::VerifySchnorr(const uint256& msg, Span<const unsigned char> sigbytes) const
{
    assert(sigbytes.size() == 64);
    secp256k1_xonly_pubkey pubkey;
    if (!secp256k1_xonly_pubkey_parse(secp256k1_context_verify, &pubkey, m_keydata.data())) return false;
    return secp256k1_schnorrsig_verify(secp256k1_context_verify, sigbytes.data(), msg.begin(), 32, &pubkey);
}

static const CHashWriter HASHER_TAPTWEAK = TaggedHash("TapTweak");

uint256 XOnlyPubKey::ComputeTapTweakHash(const uint256* merkle_root) const
{
    if (merkle_root == nullptr) {
        // We have no scripts. The actual tweak does not matter, but follow BIP341 here to
        // allow for reproducible tweaking.
        return (CHashWriter(HASHER_TAPTWEAK) << m_keydata).GetSHA256();
    } else {
        return (CHashWriter(HASHER_TAPTWEAK) << m_keydata << *merkle_root).GetSHA256();
    }
}

bool XOnlyPubKey::CheckTapTweak(const XOnlyPubKey& internal, const uint256& merkle_root, bool parity) const
{
    secp256k1_xonly_pubkey internal_key;
    if (!secp256k1_xonly_pubkey_parse(secp256k1_context_verify, &internal_key, internal.data())) return false;
    uint256 tweak = internal.ComputeTapTweakHash(&merkle_root);
    return secp256k1_xonly_pubkey_tweak_add_check(secp256k1_context_verify, m_keydata.begin(), parity, &internal_key, tweak.begin());
}

std::optional<std::pair<XOnlyPubKey, bool>> XOnlyPubKey::CreateTapTweak(const uint256* merkle_root) const
{
    secp256k1_xonly_pubkey base_point;
    if (!secp256k1_xonly_pubkey_parse(secp256k1_context_verify, &base_point, data())) return std::nullopt;
    secp256k1_pubkey out;
    uint256 tweak = ComputeTapTweakHash(merkle_root);
    if (!secp256k1_xonly_pubkey_tweak_add(secp256k1_context_verify, &out, &base_point, tweak.data())) return std::nullopt;
    int parity = -1;
    std::pair<XOnlyPubKey, bool> ret;
    secp256k1_xonly_pubkey out_xonly;
    if (!secp256k1_xonly_pubkey_from_pubkey(secp256k1_context_verify, &out_xonly, &parity, &out)) return std::nullopt;
    secp256k1_xonly_pubkey_serialize(secp256k1_context_verify, ret.first.begin(), &out_xonly);
    assert(parity == 0 || parity == 1);
    ret.second = parity;
    return ret;
}

bool CPubKey::IsFullyValid() const
{
    if (!IsValid())
        return false;
    secp256k1_pubkey pubkey;
    assert(secp256k1_context_verify && "secp256k1_context_verify must be initialized to use CPubKey.");
    return secp256k1_ec_pubkey_parse(secp256k1_context_verify, &pubkey, vch, size());
}

/* static */ int ECCVerifyHandle::refcount = 0;

ECCVerifyHandle::ECCVerifyHandle()
{
    if (refcount == 0) {
        assert(secp256k1_context_verify == nullptr);
        secp256k1_context_verify = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY);
        assert(secp256k1_context_verify != nullptr);
    }
    refcount++;
}

ECCVerifyHandle::~ECCVerifyHandle()
{
    refcount--;
    if (refcount == 0) {
        assert(secp256k1_context_verify != nullptr);
        secp256k1_context_destroy(secp256k1_context_verify);
        secp256k1_context_verify = nullptr;
    }
}

bool CBOBPubKey::Verify(const uint512& hash, const std::vector<unsigned char>& vchSig) const
{
    if (!IsValid())
        return false;
    unsigned char pk[SIZE];
    unsigned char sig[SIGNATURE_SIZE];

    memcpy(pk, &(*this)[0], size());
    memcpy(sig, vchSig.data(), vchSig.size());

    int ret = crypto_sign_verify(sig, vchSig.size(), hash.begin(), hash.size(), pk);
    if (ret == 0) {
        printf("sig verifying success :)\n");
        return true;
    } else {
        printf("sig verifying fail :(\n");
        return false;
    }
}

bool CBOBPubKey::RecoverCompact(const uint512& hash, const std::vector<unsigned char>& vchSig)
{
    return true;
}

bool CBOBPubKey::IsFullyValid() const
{
    if (!IsValid())
        return false;
    return true;
}

bool CBOBPubKey::Derive(CBOBPubKey& pubkeyChild, ChainCode& ccChild, unsigned int nChild, const ChainCode& cc) const
{
    assert(IsValid());
    assert((nChild >> 31) == 0);
    assert(size() == COMPRESSED_SIZE);
    unsigned char out[64];
    BIP32Hash(cc, nChild, *begin(), begin() + 1, out);
    memcpy(ccChild.begin(), out + 32, 32);
    secp256k1_pubkey pubkey;
    assert(secp256k1_context_verify && "secp256k1_context_verify must be initialized to use CPubKey.");
    if (!secp256k1_ec_pubkey_parse(secp256k1_context_verify, &pubkey, vch, size())) {
        return false;
    }
    if (!secp256k1_ec_pubkey_tweak_add(secp256k1_context_verify, &pubkey, out)) {
        return false;
    }
    unsigned char pub[COMPRESSED_SIZE];
    size_t publen = COMPRESSED_SIZE;
    secp256k1_ec_pubkey_serialize(secp256k1_context_verify, pub, &publen, &pubkey, SECP256K1_EC_COMPRESSED);
    pubkeyChild.Set(pub, pub + publen);
    return true;
}

void CExtBOBPubKey::Encode(unsigned char code[BIP32_EXTPQKEY_SIZE]) const
{
    code[0] = nDepth;
    memcpy(code + 1, vchFingerprint, 4);
    code[5] = (nChild >> 24) & 0xFF;
    code[6] = (nChild >> 16) & 0xFF;
    code[7] = (nChild >> 8) & 0xFF;
    code[8] = (nChild >> 0) & 0xFF;
    memcpy(code + 9, chaincode.begin(), 32);
    memcpy(code + 41, pubkey.begin(), CBOBPubKey::SIZE);
}

void CExtBOBPubKey::Decode(const unsigned char code[BIP32_EXTPQKEY_SIZE])
{
    nDepth = code[0];
    memcpy(vchFingerprint, code + 1, 4);
    nChild = (code[5] << 24) | (code[6] << 16) | (code[7] << 8) | code[8];
    memcpy(chaincode.begin(), code + 9, 32);
    pubkey.Set(code + 41, code + BIP32_EXTPQKEY_SIZE);
    if ((nDepth == 0 && (nChild != 0 || ReadLE32(vchFingerprint) != 0)) || !pubkey.IsFullyValid()) pubkey = CBOBPubKey();
}

bool CExtBOBPubKey::Derive(CExtBOBPubKey& out, unsigned int _nChild) const
{
    out.nDepth = nDepth + 1;
    CKeyID id = pubkey.GetID();
    memcpy(out.vchFingerprint, &id, 4);
    out.nChild = _nChild;
    return pubkey.Derive(out.pubkey, out.chaincode, _nChild, chaincode);
}
