#pragma once
// maui::core::type_tag — a stable, RTTI-free type identity (PROFILE §6).
//
// MAUI keys handler discovery and DI on `System.Type`; C++23 has no reflection, so the port keys on a
// `type_tag` instead: a value-type wrapper around a unique-per-type address (the address of a
// function-local static, which the linker folds to one instance program-wide). `type_tag::of<T>()` is
// cheap, needs no `typeid`/RTTI, and is usable as an unordered_map key (a std::hash specialization is
// provided). It underpins both the handler_registry (view-type → handler factory) and the
// service_registry (service-type → instance).

#include <cstddef>
#include <functional>

namespace maui::core
{
    class type_tag
    {
    public:
        // The identity of T. Equal for the same T, distinct across types; stable across TUs.
        template <class T> [[nodiscard]] static type_tag of() noexcept
        {
            return type_tag{anchor_of<T>()};
        }

        [[nodiscard]] bool operator==(const type_tag&) const noexcept = default;

        [[nodiscard]] std::size_t hash() const noexcept
        {
            return std::hash<const void*>{}(id_);
        }

    private:
        explicit constexpr type_tag(const void* id) noexcept : id_(id)
        {
        }

        // One unique, stable address per instantiation (vague linkage folds it to a single object).
        template <class T> [[nodiscard]] static const void* anchor_of() noexcept
        {
            static const char anchor{};
            return &anchor;
        }

        const void* id_;
    };
} // namespace maui::core

template <> struct std::hash<maui::core::type_tag>
{
    [[nodiscard]] std::size_t operator()(const maui::core::type_tag& tag) const noexcept
    {
        return tag.hash();
    }
};
