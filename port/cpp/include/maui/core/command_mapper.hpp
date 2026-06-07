#pragma once
// maui::core::command_mapper  <=  Microsoft.Maui.CommandMapper / ICommandMapper
//
// The command analogue of property_mapper: maps a command NAME to an action that also receives a
// payload (`object?` in C# → `std::any` here). Commands are one-shot operations rather than property
// pushes — e.g. InvalidateMeasure / Frame / Focus / Unfocus. Ported from src/Core/src/CommandMapper.cs.
// Chaining is a single fallback mapper (C#'s CommandMapper.Chained is one mapper, not an array).
//
// Same two-layer shape as property_mapper: a non-generic command_mapper_base (storage/chaining/dispatch
// over the base types) and a typed command_mapper<Virtual,Handler> authoring surface that down-casts at
// the boundary. The std::any payload is the boundary-confined erasure PROFILE §7 endorses.

#include <any>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "maui/core/i_element.hpp"
#include "maui/core/i_element_handler.hpp"

namespace maui::core
{
    class command_mapper_base
    {
    public:
        using action = std::function<void(i_element_handler&, i_element&, const std::any&)>;

        command_mapper_base() = default;
        virtual ~command_mapper_base() = default;
        command_mapper_base(const command_mapper_base&) = default;
        command_mapper_base(command_mapper_base&&) = default;
        command_mapper_base& operator=(const command_mapper_base&) = default;
        command_mapper_base& operator=(command_mapper_base&&) = default;

        // C# Chained (single fallback mapper). Non-owning.
        void set_chained(command_mapper_base* chained);

        // C# GetCommand: this mapper's own action for `key`, else the chained mapper's, else null.
        [[nodiscard]] const action* get_command(std::string_view key) const;

        // C# Invoke.
        void invoke(i_element_handler& handler, i_element& view, std::string_view command, const std::any& args) const;

    protected:
        void set_command_core(std::string key, action act);

    private:
        std::vector<std::pair<std::string, action>> entries_;
        command_mapper_base* chained_ = nullptr;
    };

    template <class Virtual, class Handler> class command_mapper : public command_mapper_base
    {
        static_assert(std::is_base_of_v<i_element, Virtual>, "Virtual must derive maui::core::i_element");
        static_assert(std::is_base_of_v<i_element_handler, Handler>,
                      "Handler must derive maui::core::i_element_handler");

    public:
        using typed_action = std::function<void(Handler&, Virtual&, const std::any&)>;
        using entry = std::pair<std::string_view, typed_action>;

        command_mapper() = default;

        explicit command_mapper(std::initializer_list<entry> entries)
        {
            add_all(entries);
        }

        command_mapper(command_mapper_base& chained, std::initializer_list<entry> entries)
        {
            set_chained(&chained);
            add_all(entries);
        }

        void add(std::string key, typed_action action)
        {
            set_command_core(std::move(key), [typed = std::move(action)](i_element_handler& handler, i_element& view,
                                                                         const std::any& args) {
                if (auto* typed_view = dynamic_cast<Virtual*>(&view))
                {
                    typed(dynamic_cast<Handler&>(handler), *typed_view, args);
                }
            });
        }

    private:
        void add_all(std::initializer_list<entry> entries)
        {
            for (const auto& [key, action] : entries)
            {
                add(std::string(key), action);
            }
        }
    };
} // namespace maui::core
