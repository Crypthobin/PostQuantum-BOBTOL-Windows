// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_HASH_H
#define BITCOIN_HASH_H

#include <attributes.h>
#include <crypto/common.h>
#include <crypto/ripemd160.h>
#include <crypto/sha256.h>
#include <crypto/sha512.h> // 추가
#include <prevector.h>
#include <serialize.h>
#include <uint256.h>
#include <version.h>

#include <string>
#include <vector>

typedef uint256 ChainCode;
typedef uint512 PQChainCode; // 추가

/** A hasher class for Bitcoin's 256-bit hash (double SHA-256). */
class CHash256
{
private:
    CSHA256 sha;

public:
    static const size_t OUTPUT_SIZE = CSHA256::OUTPUT_SIZE;

    void Finalize(Span<unsigned char> output)
    {
        assert(output.size() == OUTPUT_SIZE);
        unsigned char buf[CSHA256::OUTPUT_SIZE];
        sha.Finalize(buf);
        sha.Reset().Write(buf, CSHA256::OUTPUT_SIZE).Finalize(output.data());
    }

    CHash256& Write(Span<const unsigned char> input)
    {
        sha.Write(input.data(), input.size());
        return *this;
    }

    CHash256& Reset()
    {
        sha.Reset();
        return *this;
    }
};


/** A hasher class for Bitcoin's 160-bit hash (SHA-256 + RIPEMD-160). */
class CHash160
{
private:
    CSHA256 sha;

public:
    static const size_t OUTPUT_SIZE = CRIPEMD160::OUTPUT_SIZE;

    void Finalize(Span<unsigned char> output)
    {
        assert(output.size() == OUTPUT_SIZE);
        unsigned char buf[CSHA256::OUTPUT_SIZE];
        sha.Finalize(buf);
        CRIPEMD160().Write(buf, CSHA256::OUTPUT_SIZE).Finalize(output.data());
    }

    void Finalize2(unsigned char hash[OUTPUT_SIZE])
    {
        unsigned char buf[CSHA256::OUTPUT_SIZE];
        sha.Finalize(buf);
        CRIPEMD160().Write(buf, CSHA256::OUTPUT_SIZE).Finalize(hash);
    }

    CHash160& Write(Span<const unsigned char> input)
    {
        sha.Write(input.data(), input.size());
        return *this;
    }

    CHash160& Write2(const unsigned char* data, size_t len)
    {
        sha.Write(data, len);
        return *this;
    }

    CHash160& Reset()
    {
        sha.Reset();
        return *this;
    }
};

/** Compute the 256-bit hash of an object. */
template <typename T>
inline uint256 Hash(const T& in1)
{
    uint256 result;
    CHash256().Write(MakeUCharSpan(in1)).Finalize(result);
    return result;
}

/** Compute the 256-bit hash of the concatenation of two objects. */
template <typename T1, typename T2>
inline uint256 Hash(const T1& in1, const T2& in2)
{
    uint256 result;
    CHash256().Write(MakeUCharSpan(in1)).Write(MakeUCharSpan(in2)).Finalize(result);
    return result;
}

///** Compute the 160-bit hash an object. */
template <typename T1>
inline uint160 Hash160(const T1& in1)
{
    uint160 result;
    CHash160().Write(MakeUCharSpan(in1)).Finalize(result);
    return result;
}

/** Compute the 160-bit hash of the concatenation of two objects. */
//template <typename T1>
//inline uint160 Hash160(const T1& in1, const T1& in2)
//{
//    uint160 result;
//    CHash160().Write(MakeUCharSpan(in1)).Write(MakeUCharSpan(in2)).Finalize(result);
//    return result;
//}

template <typename T1>
inline uint160 Hash160(const T1 pbegin, const T1 pend)
{
    static unsigned char pblank[1] = {};
    uint160 result;
    CHash160().Write2(pbegin == pend ? pblank : (unsigned char*)&pbegin[0], (pend - pbegin) * sizeof(pbegin[0])).Finalize2((unsigned char*)&result);
    return result;
}

//template <unsigned int N>
//inline uint160 Hash160(const prevector<N, unsigned char>& vch)
//{
//    return Hash160(vch.begin(), vch.end());
//}

/** A writer stream (for serialization) that computes a 256-bit hash. */
class CHashWriter
{
private:
    CSHA256 ctx;

    const int nType;
    const int nVersion;

public:
    CHashWriter(int nTypeIn, int nVersionIn) : nType(nTypeIn), nVersion(nVersionIn) {}

    int GetType() const { return nType; }
    int GetVersion() const { return nVersion; }

    void write(const char* pch, size_t size)
    {
        ctx.Write((const unsigned char*)pch, size);
    }

    /** Compute the double-SHA256 hash of all data written to this object.
     *
     * Invalidates this object.
     */
    uint256 GetHash()
    {
        uint256 result;
        ctx.Finalize(result.begin());
        ctx.Reset().Write(result.begin(), CSHA256::OUTPUT_SIZE).Finalize(result.begin());
        return result;
    }

    /** Compute the SHA256 hash of all data written to this object.
     *
     * Invalidates this object.
     */
    uint256 GetSHA256()
    {
        uint256 result;
        ctx.Finalize(result.begin());
        return result;
    }

    /**
     * Returns the first 64 bits from the resulting hash.
     */
    inline uint64_t GetCheapHash()
    {
        uint256 result = GetHash();
        return ReadLE64(result.begin());
    }

    template <typename T>
    CHashWriter& operator<<(const T& obj)
    {
        // Serialize to this stream
        ::Serialize(*this, obj);
        return (*this);
    }
};

/** Reads data from an underlying stream, while hashing the read data. */
//template<typename Source>
//class CHashVerifier : public CHashWriter
//{
//private:
//    Source* source;
//
//public:
//    explicit CHashVerifier(Source* source_) : CHashWriter(source_->GetType(), source_->GetVersion()), source(source_) {}
//
//    void read(char* pch, size_t nSize)
//    {
//        source->read(pch, nSize);
//        this->write(pch, nSize);
//    }
//
//    void ignore(size_t nSize)
//    {
//        char data[1024];
//        while (nSize > 0) {
//            size_t now = std::min<size_t>(nSize, 1024);
//            read(data, now);
//            nSize -= now;
//        }
//    }
//
//    template<typename T>
//    CHashVerifier<Source>& operator>>(T&& obj)
//    {
//        // Unserialize from this stream
//        ::Unserialize(*this, obj);
//        return (*this);
//    }
//};

/** Compute the 256-bit hash of an object's serialization. */
template <typename T>
uint256 SerializeHash(const T& obj, int nType = SER_GETHASH, int nVersion = PROTOCOL_VERSION)
{
    CHashWriter ss(nType, nVersion);
    ss << obj;
    return ss.GetHash();
}

/** Single-SHA256 a 32-byte input (represented as uint256). */
[[nodiscard]] uint256 SHA256Uint256(const uint256& input);

unsigned int MurmurHash3(unsigned int nHashSeed, Span<const unsigned char> vDataToHash);

void BIP32Hash(const ChainCode& chainCode, unsigned int nChild, unsigned char header, const unsigned char data[32], unsigned char output[64]);

/** Return a CHashWriter primed for tagged hashes (as specified in BIP 340).
 *
 * The returned object will have SHA256(tag) written to it twice (= 64 bytes).
 * A tagged hash can be computed by feeding the message into this object, and
 * then calling CHashWriter::GetSHA256().
 */
CHashWriter TaggedHash(const std::string& tag);


//// 추가해준 부분 *************

// class CHash512 추가 완료********
/** A hasher class for Bitcoin's 512-bit hash. */
class CHash512
{
private:
    CSHA512 sha;

public:
    static const size_t OUTPUT_SIZE = CSHA512::OUTPUT_SIZE;

    void Finalize(Span<unsigned char> output)
    {
        assert(output.size() == OUTPUT_SIZE);
        unsigned char buf[CSHA512::OUTPUT_SIZE];
        sha.Finalize(buf);
        sha.Reset().Write(buf, CSHA512::OUTPUT_SIZE).Finalize(output.data());
    }

    CHash512& Write(Span<const unsigned char> input)
    {
        sha.Write(input.data(), input.size());
        return *this;
    }

    CHash512& Reset()
    {
        sha.Reset();
        return *this;
    }
};


// Hash512() 추가
/** Compute the 512-bit hash of an object. */
template <typename T>
inline uint512 Hash512(const T& in1)
{
    uint512 result;
    CHash512().Write(MakeUCharSpan(in1)).Finalize(result);
    return result;
}

/** Compute the 512-bit hash of the concatenation of two objects. */
template <typename T1, typename T2>
inline uint512 Hash512(const T1& in1, const T2& in2)
{
    uint512 result;
    CHash512().Write(MakeUCharSpan(in1)).Write(MakeUCharSpan(in2)).Finalize(result);
    return result;
}

/** A writer stream (for serialization) that computes a 512-bit hash. */
class CPQHashWriter
{
private:
    CSHA512 ctx;

    const int nType;
    const int nVersion;

public:
    CPQHashWriter(int nTypeIn, int nVersionIn) : nType(nTypeIn), nVersion(nVersionIn) {}

    int GetType() const { return nType; }
    int GetVersion() const { return nVersion; }

    void write(const char* pch, size_t size)
    {
        ctx.Write((const unsigned char*)pch, size);
    }

    /** Compute the double-SHA512 hash of all data written to this object.
     *
     * Invalidates this object.
     */
    uint512 GetHash()
    {
        uint512 result;
        ctx.Finalize(result.begin());
        ctx.Reset().Write(result.begin(), CSHA512::OUTPUT_SIZE).Finalize(result.begin());
        return result;
    }

    /** Compute the SHA512 hash of all data written to this object.
     *
     * Invalidates this object.
     */
    uint512 GetSHA512()
    {
        uint512 result;
        ctx.Finalize(result.begin());
        return result;
    }

    /**
     * Returns the first 64 bits from the resulting hash. /// 이부분
     */
    /*inline uint64_t GetCheapHash()
    {
        uint256 result = GetHash();
        return ReadLE64(result.begin());
    }*/

    template <typename T>
    CPQHashWriter& operator<<(const T& obj)
    {
        // Serialize to this stream
        ::Serialize(*this, obj);
        return (*this);
    }
};

/** Reads data from an underlying stream, while hashing the read data. */
template <typename Source>
class CHashVerifier : public CHashWriter
{
private:
    Source* source;

public:
    explicit CHashVerifier(Source* source_) : CHashWriter(source_->GetType(), source_->GetVersion()), source(source_) {}

    void read(char* pch, size_t nSize)
    {
        source->read(pch, nSize);
        this->write(pch, nSize);
    }

    void ignore(size_t nSize)
    {
        char data[1024];
        while (nSize > 0) {
            size_t now = std::min<size_t>(nSize, 1024);
            read(data, now);
            nSize -= now;
        }
    }

    template <typename T>
    CHashVerifier<Source>& operator>>(T&& obj)
    {
        // Unserialize from this stream
        ::Unserialize(*this, obj);
        return (*this);
    }
};

CPQHashWriter TaggedHash512(const std::string& tag);

#endif // BITCOIN_HASH_H
