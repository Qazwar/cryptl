#ifndef _CRYPTL_SHA_384_HPP_
#define _CRYPTL_SHA_384_HPP_

#include <array>
#include <cstdint>

#include <cryptl/BitwiseINT.hpp>
#include <cryptl/SHA_512.hpp>

namespace cryptl {

////////////////////////////////////////////////////////////////////////////////
// SHA-384
//

template <typename T, typename MSG, typename U, typename F>
class SHA_384 : public SHA_512<T, MSG, U, F>
{
public:
    typedef T WordType;
    typedef U ByteType;

    typedef std::array<T, 16> MsgType;
    typedef std::array<T, 6> DigType;
    typedef std::array<U, 16 * 8> PreType;

    SHA_384()
        : m_setDigest(false)
    {}

    // overrides base class SHA-512
    const std::array<T, 6>& digest() {
        if (m_setDigest) {
            for (std::size_t i = 0; i < 6; ++i) {
                m_Hleft384[i] = this->m_H[i];
            }

            m_setDigest = false;
        }

        return m_Hleft384;
    }

    // overrides base class SHA-512
    virtual void initHashValue() {
        // set initial hash value (NIST FIPS 180-4 section 5.3.4)
        const std::array<uint64_t, 8> a {
            0xcbbb9d5dc1059ed8, 0x629a292a367cd507,
            0x9159015a3070dd17, 0x152fecd8f70e5939,

            0x67332667ffc00b31, 0x8eb44a8768581511,
            0xdb0c2e0d64f98fa7, 0x47b5481dbefa4fa4 };

        for (std::size_t i = 0; i < 8; ++i) {
            this->m_H[i] = F::constant(a[i]);
        }
    }

    virtual void afterHash() {
        m_setDigest = true;
    }

private:
    // truncated 384-bit message digest
    std::array<T, 6> m_Hleft384;

    bool m_setDigest;
};

////////////////////////////////////////////////////////////////////////////////
// typedef
//

typedef SHA_384<std::uint64_t,
                std::uint64_t,
                std::uint8_t,
                SHA_Functions<std::uint64_t,
                              std::uint64_t,
                              BitwiseINT<std::uint64_t>>>
    SHA384;

} // namespace cryptl

#endif
