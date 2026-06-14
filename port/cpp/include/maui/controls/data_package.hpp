#pragma once
// The DataPackage cluster — the payload carried across a drag & drop. Ported from
// src/Controls/src/Core/DragAndDrop/{DataPackage,DataPackageView,DataPackagePropertySet,
// DataPackagePropertySetView}.cs. The four types are one tightly-coupled cluster (a package, its
// read-only view, the custom-property bag, and the bag's read-only view) and share this header per
// PROFILE §3 (splitting would hurt cohesion + force forward-declaration churn for no benefit).
//
//   maui::controls::data_package                 <=  Microsoft.Maui.Controls.DataPackage
//   maui::controls::data_package_view            <=  Microsoft.Maui.Controls.DataPackageView
//   maui::controls::data_package_property_set    <=  Microsoft.Maui.Controls.DataPackagePropertySet
//   maui::controls::data_package_property_set_view <= Microsoft.Maui.Controls.DataPackagePropertySetView
//
// Surface notes / deviations (documented, port-wide):
//   - C#'s custom-property bag is Dictionary<string, object> (StringComparer.Ordinal); the port stores
//     std::any values in a std::map<std::string, std::any> — ordinal key comparison, deterministic
//     iteration, and the std::any boundary the port already uses for by-name value set (PROFILE §7).
//   - Image is an ImageSource (the i_image_source contract — image sources are reference types in C#);
//     the port models it as a shared_ptr<i_image_source> (null = unset), the same shape the image
//     control's source uses.
//   - GetImageAsync / GetTextAsync return Task<T> in C# (synchronously-completed: Task.FromResult). The
//     port has no awaited consumer here (the recognizers read the values synchronously), so the view
//     exposes plain image() / text() accessors over the cloned snapshot — the Task wrapper is dropped
//     (documented). The async-ness was never observable: both are Task.FromResult of an in-memory field.

#include <any>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/core/i_image_source.hpp"

namespace maui::controls
{
    // DataPackagePropertySet — the mutable custom-property bag (DataPackagePropertySet : IEnumerable). A
    // string-keyed object bag with ordinal key comparison. Insertion through set() mirrors C#'s indexer
    // (overwrite); add() mirrors Dictionary.Add (the port keeps add() as an alias of set() — C# throws on
    // a duplicate Add, but the only internal Add ("DragSource") happens once per package, so the
    // overwrite-vs-throw distinction is unobservable here and documented).
    class data_package_property_set
    {
    public:
        // std::less<> gives heterogeneous lookup so string_view keys probe without an allocation. The
        // entries() / view accessors expose this exact type (the C# GetEnumerator yields the same
        // KeyValuePair<string, object> node shape).
        using property_bag = std::map<std::string, std::any, std::less<>>;

        data_package_property_set() = default;

        // DataPackagePropertySet indexer get/set (`set[key]`). set() overwrites; get() returns the stored
        // value, or a null any when absent (the port's stand-in — C#'s indexer throws KeyNotFoundException,
        // but every port read goes through try_get_value / contains_key first).
        [[nodiscard]] const std::any& get(std::string_view key) const;
        void set(std::string key, std::any value);

        // DataPackagePropertySet.Add(key, value) — see the class note (alias of set() in the port).
        void add(std::string key, std::any value)
        {
            set(std::move(key), std::move(value));
        }

        // DataPackagePropertySet.Count.
        [[nodiscard]] std::size_t count() const
        {
            return bag_.size();
        }
        // DataPackagePropertySet.ContainsKey.
        [[nodiscard]] bool contains_key(std::string_view key) const;
        // DataPackagePropertySet.TryGetValue — non-null pointer to the value iff present.
        [[nodiscard]] const std::any* try_get_value(std::string_view key) const;

        // DataPackagePropertySet.Keys (deterministic, ordinal order). DataPackagePropertySet.Values
        // follows by lookup; the port exposes the entries directly for iteration.
        [[nodiscard]] const property_bag& entries() const
        {
            return bag_;
        }

    private:
        property_bag bag_;
    };

    // DataPackagePropertySetView — a read-only window onto a data_package_property_set
    // (DataPackagePropertySetView : IReadOnlyDictionary). Non-owning: it borrows the underlying set (the
    // C# view holds a reference field too). The view's lifetime is bounded by its owning data_package_view,
    // which owns the cloned package the set lives in.
    class data_package_property_set_view
    {
    public:
        explicit data_package_property_set_view(const data_package_property_set& set) : set_(&set)
        {
        }

        [[nodiscard]] const std::any& get(std::string_view key) const
        {
            return set_->get(key);
        }
        [[nodiscard]] std::size_t count() const
        {
            return set_->count();
        }
        [[nodiscard]] bool contains_key(std::string_view key) const
        {
            return set_->contains_key(key);
        }
        [[nodiscard]] const std::any* try_get_value(std::string_view key) const
        {
            return set_->try_get_value(key);
        }
        [[nodiscard]] const data_package_property_set::property_bag& entries() const
        {
            return set_->entries();
        }

    private:
        const data_package_property_set* set_; // non-owning (see the class note)
    };

    class data_package_view; // forward (DataPackage.View returns one)

    // DataPackage — the mutable drag payload (Text + Image + the custom Properties bag + the internal
    // PropertiesInternal bag the recognizers stamp "DragSource" into).
    class data_package
    {
    public:
        data_package() = default;

        // DataPackage.Text.
        [[nodiscard]] const std::optional<std::string>& text() const
        {
            return text_;
        }
        void set_text(std::optional<std::string> value)
        {
            text_ = std::move(value);
        }

        // DataPackage.Image (null = unset). Owns the source (ImageSource is a reference type; the package
        // is the strong owner while it lives).
        [[nodiscard]] const std::shared_ptr<maui::core::i_image_source>& image() const
        {
            return image_;
        }
        void set_image(std::shared_ptr<maui::core::i_image_source> value)
        {
            image_ = std::move(value);
        }

        // DataPackage.Properties (the public custom bag) / PropertiesInternal (the internal bag).
        [[nodiscard]] data_package_property_set& properties()
        {
            return properties_;
        }
        [[nodiscard]] const data_package_property_set& properties() const
        {
            return properties_;
        }
        [[nodiscard]] data_package_property_set& properties_internal()
        {
            return properties_internal_;
        }
        [[nodiscard]] const data_package_property_set& properties_internal() const
        {
            return properties_internal_;
        }

        // DataPackage.View => new DataPackageView(this.Clone()): a read-only view over a SNAPSHOT (deep
        // copy) of this package — later mutations of this package do NOT affect the returned view (the C#
        // DataPackageViewGettersArentTiedToInitialDataPackage test).
        [[nodiscard]] data_package_view view() const;

        // DataPackage.Clone — a deep copy (Text + Image + both property bags). std::any values are copied
        // by value (the C# bag copies object references; the port copies the any payload, which for the
        // recognizers' values — strings and the DragSource pointer — is the same observable result).
        [[nodiscard]] data_package clone() const;

    private:
        std::optional<std::string> text_;
        std::shared_ptr<maui::core::i_image_source> image_;
        data_package_property_set properties_;
        data_package_property_set properties_internal_;
    };

    // DataPackageView — a read-only view of a data_package (DataPackageView). OWNS the snapshot package it
    // wraps (C#'s DataPackage.View constructs the view over `this.Clone()`, so the view is the sole owner
    // of that clone). Copyable/movable: copying duplicates the snapshot so each view stays independent.
    class data_package_view
    {
    public:
        // DataPackageView(DataPackage) — wraps (and takes ownership of) the given snapshot package.
        explicit data_package_view(data_package package);

        data_package_view(const data_package_view& other);
        data_package_view(data_package_view&& other) noexcept;
        data_package_view& operator=(const data_package_view& other);
        data_package_view& operator=(data_package_view&& other) noexcept;
        ~data_package_view() = default;

        // DataPackageView.GetTextAsync / GetImageAsync — synchronous accessors over the snapshot (the Task
        // wrapper is dropped; see the file note). The image borrow stays valid for the view's lifetime.
        [[nodiscard]] const std::optional<std::string>& text() const
        {
            return package_.text();
        }
        [[nodiscard]] const std::shared_ptr<maui::core::i_image_source>& image() const
        {
            return package_.image();
        }

        // DataPackageView.Properties / PropertiesInternal (read-only views over the snapshot's bags).
        [[nodiscard]] data_package_property_set_view properties() const
        {
            return data_package_property_set_view(package_.properties());
        }
        [[nodiscard]] data_package_property_set_view properties_internal() const
        {
            return data_package_property_set_view(package_.properties_internal());
        }

    private:
        data_package package_; // the owned snapshot (Clone) — see the class note
    };
} // namespace maui::controls
