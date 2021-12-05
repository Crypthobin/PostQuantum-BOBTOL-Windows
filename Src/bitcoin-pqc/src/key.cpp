#include <key.h>

#include <crypto/common.h>
#include <crypto/hmac_sha512.h>
#include <hash.h>
#include <random.h>
#include <stdio.h>
#include <secp256k1.h>

// 헤더파일 추가 <by. Crypthobin>
#include "dilithium/randombytes.h"
#include "dilithium/pqsign.h"
#include "dilithium/fips202.h"

static secp256k1_context* secp256k1_context_sign = nullptr;

void CBOBKey::MakeNewKey()
{
    do {
        GetStrongRandBytes(keydata.data(), keydata.size());
    } while (!Check(keydata.data()));
    fValid = true;
    fCompressed = false;
}

bool Dilithium_InitSanityCheck()
{
    CBOBKey key;
    key.MakeNewKey();
    CBOBPubKey pubkey = key.GetPubKey();
    return key.VerifyPubKey(pubkey);
}

bool CBOBKey::Check(const unsigned char* vch)
{
    return true;
}

bool CBOBKey::Negate()
{
    return false;
}

CPrivKey CBOBKey::GetPrivKey() const
{
    assert(fValid);
    CPrivKey seed;
    int ret = 1;
    size_t seedlen;
    unsigned char* seeddata;
    seed.resize(SIZE);
    seedlen = SIZE;
    seeddata = seed.data();
    memcpy(seeddata, begin(), 32);
    assert(ret);
    seed.resize(seedlen);
    return seed;
}

CBOBPubKey CBOBKey::GetPubKey() const
{
    assert(fValid);

    uint8_t pk[CRYPTO_PUBLICKEYBYTES] = {0};
    uint8_t sk[CRYPTO_SECRETKEYBYTES] = {0};
    uint8_t seedbuf[2 * SEEDBYTES + CRHBYTES] = {0};

    size_t clen = CBOBPubKey::SIZE;

    CBOBPubKey result;

    shake256(seedbuf, 2 * SEEDBYTES + CRHBYTES, begin(), SEEDBYTES);

    crypto_sign_keypair(seedbuf, pk, sk);
    
    memcpy((unsigned char*)result.begin(), pk, clen);

    assert(result.size() == clen);
    assert(result.IsValid());

    return result;
}

bool CBOBKey::Sign(const uint512& hash, std::vector<unsigned char>& vchSig, bool grind, uint32_t test_case) const
{
    if (!fValid)
        return false;
    vchSig.resize(CBOBPubKey::SIGNATURE_SIZE);

    uint8_t sig[CBOBPubKey::SIGNATURE_SIZE] = {0};
    size_t sig_len = 0;
    uint8_t pk[CRYPTO_PUBLICKEYBYTES] = {0};
    uint8_t sk[CRYPTO_SECRETKEYBYTES] = {0};
    uint8_t seedbuf[2 * SEEDBYTES + CRHBYTES] = {0};

    shake256(seedbuf, 2 * SEEDBYTES + CRHBYTES, begin(), SEEDBYTES);

    crypto_sign_keypair(seedbuf, pk, sk);
    crypto_sign_signature(sig, &sig_len, hash.begin(), hash.size(), sk);

    memcpy(vchSig.data(), sig, sig_len);
    vchSig.resize(sig_len);

    return true;
}

bool CBOBKey::VerifyPubKey(const CBOBPubKey& pubkey) const
{
    unsigned char rnd[8];
    std::string str = "Bitcoin key verification\n";
    GetRandBytes(rnd, sizeof(rnd));

    uint512 hash;
    CHash512().Write(MakeUCharSpan(str)).Write(rnd).Finalize(hash);
    std::vector<unsigned char> vchSig;
    Sign(hash, vchSig);
    return pubkey.Verify(hash, vchSig);
}

bool CBOBKey::Load(CPrivKey& seckey, CBOBPubKey& vchPubKey, bool fSkipCheck = false)
{
    memcpy((unsigned char*)begin(), seckey.data(), seckey.size());

    fValid = true;

    if (fSkipCheck)
        return true;

    return VerifyPubKey(vchPubKey);
}

bool CBOBKey::Derive(CBOBKey& keyChild, ChainCode& ccChild, unsigned int nChild, const ChainCode& cc) const
{
    assert(IsValid());
    std::vector<unsigned char, secure_allocator<unsigned char>> vout(64);
    if ((nChild >> 31) == 0) {
        CBOBPubKey pubkey = GetPubKey();

        BIP32Hash(cc, nChild, *pubkey.begin(), pubkey.begin() + 1, vout.data());
    } else {
        assert(size() == 32);
        BIP32Hash(cc, nChild, 0, begin(), vout.data());
    }
    memcpy(ccChild.begin(), vout.data() + 32, 32);
    memcpy((unsigned char*)keyChild.begin(), begin(), 32);
    bool ret = secp256k1_ec_seckey_tweak_add(secp256k1_context_sign, (unsigned char*)keyChild.begin(), vout.data());

    keyChild.fValid = ret;

    return ret;
}

bool CExtBOBKey::Derive(CExtBOBKey& out, unsigned int _nChild) const
{
    out.nDepth = nDepth + 1;
    CKeyID id = key.GetPubKey().GetID();
    memcpy(out.vchFingerprint, &id, 4);
    out.nChild = _nChild;
    return key.Derive(out.key, out.chaincode, _nChild, chaincode);
}

void CExtBOBKey::SetSeed(const unsigned char* seed, unsigned int nSeedLen)
{
    static const unsigned char hashkey[] = {'B', 'i', 't', 'c', 'o', 'i', 'n', ' ', 's', 'e', 'e', 'd'};
    std::vector<unsigned char, secure_allocator<unsigned char>> vout(64);
    CHMAC_SHA512(hashkey, sizeof(hashkey)).Write(seed, nSeedLen).Finalize(vout.data());
    key.Set(vout.data(), vout.data() + 32, true);
    memcpy(chaincode.begin(), vout.data() + 32, 32);
    nDepth = 0;
    nChild = 0;
    memset(vchFingerprint, 0, sizeof(vchFingerprint));
}

CExtBOBPubKey CExtBOBKey::Neuter() const
{
    CExtBOBPubKey ret;
    ret.nDepth = nDepth;
    memcpy(ret.vchFingerprint, vchFingerprint, 4);
    ret.nChild = nChild;
    ret.pubkey = key.GetPubKey();
    ret.chaincode = chaincode;
    return ret;
}

void CExtBOBKey::Encode(unsigned char code[BIP32_EXTPQKEY_SIZE]) const
{
    code[0] = nDepth;
    memcpy(code + 1, vchFingerprint, 4);
    code[5] = (nChild >> 24) & 0xFF;
    code[6] = (nChild >> 16) & 0xFF;
    code[7] = (nChild >> 8) & 0xFF;
    code[8] = (nChild >> 0) & 0xFF;
    memcpy(code + 9, chaincode.begin(), 32);
    code[41] = 0;
    assert(key.size() == 32);
    memcpy(code + 42, key.begin(), 32);
}

void CExtBOBKey::Decode(const unsigned char code[BIP32_EXTPQKEY_SIZE])
{
    nDepth = code[0];
    memcpy(vchFingerprint, code + 1, 4);
    nChild = (code[5] << 24) | (code[6] << 16) | (code[7] << 8) | code[8];
    memcpy(chaincode.begin(), code + 9, 32);
    key.Set(code + 42, code + BIP32_EXTKEY_SIZE, true);
    if ((nDepth == 0 && (nChild != 0 || ReadLE32(vchFingerprint) != 0)) || code[41] != 0) key = CBOBKey();
}
