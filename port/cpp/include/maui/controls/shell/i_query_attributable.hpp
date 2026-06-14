#pragma once
// maui::controls::i_query_attributable  <=  Microsoft.Maui.Controls.IQueryAttributable
//
// The reflection-free query-parameter contract: a page (or view-model object) that wants the query
// parameters of a shell navigation implements this and receives the merged dictionary. This is the
// ONLY parameter-delivery channel in the port — C#'s [QueryProperty] attribute path is reflection
// (Type.GetCustomAttributes + PropertyInfo.SetValue) and is intentionally not ported (PROFILE §6);
// implement apply_query_attributes and read the values instead.

#include "maui/controls/shell/shell_route_parameters.hpp"

namespace maui::controls
{
    class i_query_attributable
    {
    public:
        virtual ~i_query_attributable() = default;
        virtual void apply_query_attributes(const shell_route_parameters& query) = 0;

    protected:
        i_query_attributable() = default;
        i_query_attributable(const i_query_attributable&) = default;
        i_query_attributable(i_query_attributable&&) = default;
        i_query_attributable& operator=(const i_query_attributable&) = default;
        i_query_attributable& operator=(i_query_attributable&&) = default;
    };
} // namespace maui::controls
