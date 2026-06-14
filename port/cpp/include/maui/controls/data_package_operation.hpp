#pragma once
// maui::controls::data_package_operation  <=  Microsoft.Maui.Controls.DataPackageOperation
//
// The operation a drop target permits / a drop completes with. Ported 1:1 from
// src/Controls/src/Core/DragAndDrop/DataPackageOperation.cs — a [Flags] enum copied from UWP's
// Windows.ApplicationModel.DataTransfer.DataPackageOperation (None = 0, Copy = 1). DragEventArgs
// .AcceptedOperation defaults to Copy. The flag operators combine through std::bit_cast (same rationale
// as buttons_mask / swipe_direction: a future combined value is valid but matches no single enumerator).

#include <bit>
#include <cstdint>

namespace maui::controls
{
    enum class data_package_operation : std::uint8_t
    {
        none = 0, // DataPackageOperation.None
        copy = 1, // DataPackageOperation.Copy
    };

    [[nodiscard]] constexpr data_package_operation operator|(data_package_operation lhs, data_package_operation rhs)
    {
        return std::bit_cast<data_package_operation>(
            static_cast<std::uint8_t>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs)));
    }

    [[nodiscard]] constexpr data_package_operation operator&(data_package_operation lhs, data_package_operation rhs)
    {
        return std::bit_cast<data_package_operation>(
            static_cast<std::uint8_t>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs)));
    }

    constexpr data_package_operation& operator|=(data_package_operation& lhs, data_package_operation rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }

    // Whether `mask` includes every flag in `flag` — the C# `(mask & flag) == flag` idiom.
    [[nodiscard]] constexpr bool contains(data_package_operation mask, data_package_operation flag)
    {
        return (mask & flag) == flag;
    }
} // namespace maui::controls
