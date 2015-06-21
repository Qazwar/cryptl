#ifndef _CRYPTL_SHA_HPP_
#define _CRYPTL_SHA_HPP_

#include <cassert>
#include <climits>
#include <cstdint>
#include <ostream>
#include <vector>

namespace cryptl {

////////////////////////////////////////////////////////////////////////////////
// FIPS PUB 180-4, NIST March 2012
//
// Algorithm     Message Size   Block Size   Word Size   Message Digest
//               (bits)         (bits)       (bits)      Size (bits)
// SHA-1         < 2^64         512          32          160
// SHA-224       < 2^64         512          32          224
// SHA-256       < 2^64         512          32          256
// SHA-384       < 2^128        1024         64          384
// SHA-512       < 2^128        1024         64          512
// SHA-512/224   < 2^128        1024         64          224
// SHA-512/256   < 2^128        1024         64          256
//

enum class SHA_BlockSize {
    BLOCK_512, // for: SHA-1, SHA-224, SHA-256
    BLOCK_1024 // for: SHA-384, SHA-512, SHA-512/224, SHA-512/256
};

template <typename CRTP, SHA_BlockSize BLK, typename MSG>
class SHA_Base
{
public:
    virtual ~SHA_Base() = default;

    static void padMessage(std::ostream& os, std::size_t& lengthBits) {
        // message size is limited to < 2^64 bits in this implementation
        const std::uint64_t msgLengthBits = lengthBits;

        // append bit "1" to end of message
        append(os, lengthBits, 0x80);

        // keep padding zero bits to the length block at the end
        const std::size_t stopPadBits = blockSizeBits() - 2 * wordSizeBits();
        while (stopPadBits != lengthBits % blockSizeBits()) {
            append(os, lengthBits, 0x00);
        }

        // append length of message
        if (SHA_BlockSize::BLOCK_1024 == BLK) {
            for (std::size_t i = 0; i < 8; ++i) {
                append(os, lengthBits, 0x00);
            }
        }
        for (int i = 7; i >= 0; --i) {
            append(os, lengthBits, (msgLengthBits >> i * CHAR_BIT) & 0xff);
        }
    }

    static bool padNeeded(const std::size_t lengthBits) {
        // non-empty message must be even number of message input blocks
        return (0 == lengthBits) || (0 != lengthBits % blockSizeBits());
    }

    // append (possibly lazy) word to message
    template <typename T>
    void msgInput(const T& a) {
        m_message.emplace_back(a);
    }

    // append array of words to message
    template <typename T, std::size_t N>
    void msgInput(const std::array<T, N>& a) {
        for (const auto& b : a)
            msgInput(b);
    }

    // append vector of words to message
    template <typename T>
    void msgInput(const std::vector<T>& a) {
        for (const auto& b : a)
            msgInput(b);
    }

    void clearMessage() {
        m_message.clear();
    }

    void computeHash() {
#ifdef USE_ASSERT
        assert(inputOK());
#endif

        auto* ptr = static_cast<CRTP*>(this);

        ptr->initHashValue();

        std::size_t msgIndex = 0;
        while (msgIndex < m_message.size()) {
            ptr->prepMsgSchedule(msgIndex);
            ptr->initWorkingVars();
            ptr->workingLoop();
            ptr->updateHash();
        }

        ptr->afterHash();
    }

protected:
    SHA_Base() = default;

    // note: reference not const so assignment can unbox laziness
    MSG& msgWord(std::size_t& index) {
        return m_message[index++];
    }

private:
    static void append(std::ostream& os,
                       std::size_t& lengthBits,
                       const char c) {
        os << c;
        lengthBits += CHAR_BIT;
    }

    static std::size_t blockSizeBits() {
        switch (BLK) {
        case (SHA_BlockSize::BLOCK_512) : return 512;
        case (SHA_BlockSize::BLOCK_1024) : return 1024;
        }
    }

    static std::size_t wordSizeBits() {
        switch (BLK) {
        case (SHA_BlockSize::BLOCK_512) : return 32;
        case (SHA_BlockSize::BLOCK_1024) : return 64;
        }
    }

    bool inputOK() const {
        // non-empty message must be even number of message input blocks
        const std::size_t msgSizeBits = m_message.size() * wordSizeBits();
        return !m_message.empty() && 0 == msgSizeBits % blockSizeBits();
    }

    std::vector<MSG> m_message;
};

////////////////////////////////////////////////////////////////////////////////
// SHA common functions
//

template <typename T, typename U, typename BITWISE>
class SHA_Functions : public BITWISE
{
public:
    static U Ch(const T& x, const T& y, const T& z) {
        return
            BITWISE::XOR(
                BITWISE::_AND(x, y),
                BITWISE::_AND(BITWISE::_CMPLMNT(x), z));
    }

    static U Parity(const T& x, const T& y, const T& z) {
        return
            BITWISE::XOR(
                BITWISE::_XOR(x, y),
                z);
    }

    static U Maj(const T& x, const T& y, const T& z) {
        return
            BITWISE::XOR(
                BITWISE::_XOR(
                    BITWISE::_AND(x, y),
                    BITWISE::_AND(x, z)),
                BITWISE::_AND(y, z));
    }

    static U f(const T& x, const T& y, const T& z, const std::size_t round) {
        if (round < 20) {
            return Ch(x, y, z);
        } else if (round < 40) {
            return Parity(x, y, z);
        } else if (round < 60) {
            return Maj(x, y, z);
        } else {
            return Parity(x, y, z);
        }
    }

    static U SIGMA_256_0(const T& x) { return SIGMA(x, 2, 13, 22); }
    static U SIGMA_256_1(const T& x) { return SIGMA(x, 6, 11, 25); }
    static U sigma_256_0(const T& x) { return sigma(x, 7, 18, 3); }
    static U sigma_256_1(const T& x) { return sigma(x, 17, 19, 10); }
    static U SIGMA_512_0(const T& x) { return SIGMA(x, 28, 34, 39); }
    static U SIGMA_512_1(const T& x) { return SIGMA(x, 14, 18, 41); }
    static U sigma_512_0(const T& x) { return sigma(x, 1, 8, 7); }
    static U sigma_512_1(const T& x) { return sigma(x, 19, 61, 6); }

private:
    static U SIGMA(const T& x,
                   const unsigned int a,
                   const unsigned int b,
                   const unsigned int c) {
        return
            BITWISE::XOR(
                BITWISE::_XOR(
                    BITWISE::_ROTR(x, a),
                    BITWISE::_ROTR(x, b)),
                BITWISE::_ROTR(x, c));
    }

    static U sigma(const T& x,
                   const unsigned int a,
                   const unsigned int b,
                   const unsigned int c) {
        return
            BITWISE::XOR(
                BITWISE::_XOR(
                    BITWISE::_ROTR(x, a),
                    BITWISE::_ROTR(x, b)),
                BITWISE::_SHR(x, c));
    }
};

} // namespace cryptl

#endif
