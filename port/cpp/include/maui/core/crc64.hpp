#pragma once
// maui::core::crc64_hash_string  <=  Microsoft.Maui.Crc64 / Crc64HashAlgorithm
//
// The CRC-64 (Jones / crc-64-jones, poly 0xad93d23594c935a9) hash MAUI uses to derive a stable cache-file
// name from an image URI. Ported from src/Core/src/Services/Crc64.cs + Crc64HashAlgorithm.cs:
//   crc starts at ulong.MaxValue; HashCore folds each byte through the precomputed table; HashFinal returns
//   (crc XOR length) as 8 little-endian bytes; ComputeHashString hex-encodes those bytes (the X2 "%02X"
//   lookup → an UPPERCASE hex string).
//
// Used only for the disk-cache filename (uri_image_disk_cache): a collision-resistant, deterministic key,
// not a security primitive. Kept faithful to the C# table + finalization so the on-disk layout matches
// MAUI's MauiUriImages cache exactly.

#include <string>
#include <string_view>

namespace maui::core
{
    // CRC-64-Jones of `input`'s UTF-8 bytes, hex-encoded as an uppercase string (C# Crc64.ComputeHashString).
    [[nodiscard]] std::string crc64_hash_string(std::string_view input);
} // namespace maui::core
