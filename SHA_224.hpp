#ifndef _CRYPTL_SHA_224_HPP_
#define _CRYPTL_SHA_224_HPP_

#include <array>
#include <cstdint>

#include <cryptl/BitwiseINT.hpp>
#include <cryptl/SHA_256.hpp>

namespace cryptl {

////////////////////////////////////////////////////////////////////////////////
// SHA-224
//

template <typename T, typename MSG, typename U, typename F>
class SHA_224 : public SHA_256<T, MSG, U, F>
{
public:
    typedef T WordType;
    typedef U ByteType;

    typedef std::array<T, 16> MsgType;
    typedef std::array<T, 7> DigType;
    typedef std::array<U, 16 * 4> PreType;

    SHA_224()
        : m_setDigest(false)
    {}

    // overrides base class SHA-256
    const std::array<T, 7>& digest() {
        if (m_setDigest) {
            for (std::size_t i = 0; i < 7; ++i) {
                m_Hleft224[i] = this->m_H[i];
            }

            m_setDigest = false;
        }

        return m_Hleft224;
    }

    // overrides base class SHA-256
    virtual void initHashValue() {
        // set initial hash value (NIST FIPS 180-4 section 5.3.2)
        const std::array<std::uint32_t, 8> a {
            0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939,
            0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4 };

        for (std::size_t i = 0; i < 8; ++i) {
            this->m_H[i] = F::constant(a[i]);
        }
    }

    virtual void afterHash() {
        m_setDigest = true;
    }

private:
    // truncated 224-bit message digest
    std::array<T, 7> m_Hleft224;

    bool m_setDigest;
};

////////////////////////////////////////////////////////////////////////////////
// typedef
//

typedef SHA_224<std::uint32_t,
                std::uint32_t,
                std::uint8_t,
                SHA_Functions<std::uint32_t,
                              std::uint32_t,
                              BitwiseINT<std::uint32_t>>>
    SHA224;

} // namespace cryptl

#endif
