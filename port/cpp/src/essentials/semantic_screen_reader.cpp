// The cross-platform half of the semantic_screen_reader facade: the lazily-created implementation slot
// behind SemanticScreenReader.Default / SetDefault. The implementation itself is the per-backend
// partial (src/platform/<backend>/essentials_semantic_screen_reader.{cpp,mm}) via
// detail::make_semantic_screen_reader().

#include "maui/essentials/semantic_screen_reader.hpp"

#include <memory>
#include <utility>

namespace maui::accessibility
{
    namespace
    {
        std::shared_ptr<i_semantic_screen_reader>& semantic_screen_reader_storage()
        {
            static std::shared_ptr<i_semantic_screen_reader> storage;
            return storage;
        }
    } // namespace

    i_semantic_screen_reader& semantic_screen_reader::default_()
    {
        auto& storage = semantic_screen_reader_storage();
        if (storage == nullptr)
        {
            storage = detail::make_semantic_screen_reader();
        }
        return *storage;
    }

    void semantic_screen_reader::set_default(std::shared_ptr<i_semantic_screen_reader> implementation)
    {
        semantic_screen_reader_storage() = std::move(implementation);
    }
} // namespace maui::accessibility
