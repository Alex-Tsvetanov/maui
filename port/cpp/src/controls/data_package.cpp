// maui::controls — out-of-line definitions for the DataPackage cluster (data_package.hpp): the property
// bag's lookup/mutation, the deep Clone, and DataPackage.View. Ported from
// src/Controls/src/Core/DragAndDrop/{DataPackage,DataPackagePropertySet,DataPackageView}.cs.

#include <any>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/data_package.hpp"

namespace maui::controls
{
    // ---- data_package_property_set ----
    namespace
    {
        // The empty-any returned for an absent key (the port's stand-in for C#'s indexer throw — see the
        // header note; every port read probes with try_get_value / contains_key first).
        const std::any& null_any()
        {
            static const std::any value;
            return value;
        }
    } // namespace

    const std::any& data_package_property_set::get(std::string_view key) const
    {
        const auto it = bag_.find(key);
        return it != bag_.end() ? it->second : null_any();
    }

    void data_package_property_set::set(std::string key, std::any value)
    {
        bag_.insert_or_assign(std::move(key), std::move(value));
    }

    bool data_package_property_set::contains_key(std::string_view key) const
    {
        return bag_.contains(key);
    }

    const std::any* data_package_property_set::try_get_value(std::string_view key) const
    {
        const auto it = bag_.find(key);
        return it != bag_.end() ? &it->second : nullptr;
    }

    // ---- data_package ----
    data_package data_package::clone() const
    {
        // DataPackage.Clone: copy Text + Image, then re-add every public + internal property.
        data_package copy;
        copy.text_ = text_;
        copy.image_ = image_;
        for (const auto& [key, value] : properties_.entries())
        {
            copy.properties_.set(key, value);
        }
        for (const auto& [key, value] : properties_internal_.entries())
        {
            copy.properties_internal_.set(key, value);
        }
        return copy;
    }

    data_package_view data_package::view() const
    {
        // DataPackage.View => new DataPackageView(this.Clone()).
        return data_package_view(clone());
    }

    // ---- data_package_view ----
    data_package_view::data_package_view(data_package package) : package_(std::move(package))
    {
    }

    data_package_view::data_package_view(const data_package_view& other) : package_(other.package_.clone())
    {
    }

    data_package_view::data_package_view(data_package_view&& other) noexcept : package_(std::move(other.package_))
    {
    }

    data_package_view& data_package_view::operator=(const data_package_view& other)
    {
        if (this != &other)
        {
            package_ = other.package_.clone();
        }
        return *this;
    }

    data_package_view& data_package_view::operator=(data_package_view&& other) noexcept
    {
        if (this != &other)
        {
            package_ = std::move(other.package_);
        }
        return *this;
    }
} // namespace maui::controls
