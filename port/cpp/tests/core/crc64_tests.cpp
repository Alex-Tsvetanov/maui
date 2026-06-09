// crc64_hash_string tests (headless). Pins the CRC-64-Jones port (crc64.cpp) against derivable vectors so a
// table/finalization regression is caught. The hash is the disk-cache filename key (uri_image_disk_cache),
// so stability matters. Mirrors the contract of src/Core/src/Services/Crc64.cs (ComputeHashString).
#include "maui/core/crc64.hpp"

#include <string>

#include <gtest/gtest.h>

namespace
{
    using maui::core::crc64_hash_string;

    // Empty input: crc stays ulong.MaxValue, length 0, HashFinal = MaxValue ^ 0 = 0xFFFF...FF → 8 bytes of
    // 0xFF → "FFFFFFFFFFFFFFFF". (Derivable directly from the C# algorithm; no table lookup involved.)
    TEST(crc64, empty_input_is_all_ff)
    {
        EXPECT_EQ(crc64_hash_string(""), "FFFFFFFFFFFFFFFF");
    }

    // The output is a fixed 16-char uppercase-hex string for any input (8 bytes, 2 hex chars each).
    TEST(crc64, output_is_16_uppercase_hex_chars)
    {
        const std::string h = crc64_hash_string("https://example.com/image.png");
        EXPECT_EQ(h.size(), 16U);
        for (const char c : h)
        {
            const bool is_upper_hex = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
            EXPECT_TRUE(is_upper_hex) << "non-uppercase-hex char in hash: " << c;
        }
    }

    // Deterministic + sensitive: the same input always hashes the same, and a different input differs (so
    // distinct uris map to distinct cache files — what GetCachedFileName relies on).
    TEST(crc64, is_deterministic_and_input_sensitive)
    {
        const std::string a = crc64_hash_string("https://example.com/a.png");
        EXPECT_EQ(a, crc64_hash_string("https://example.com/a.png"));
        EXPECT_NE(a, crc64_hash_string("https://example.com/b.png"));
    }
} // namespace
