// The cross-platform half of the text_to_speech facade: the lazily-created implementation slot
// behind TextToSpeech.Default / TextToSpeech.SetDefault, plus the internal SplitSpeak chunker
// (TextToSpeech.shared.cs). The implementation itself is the per-backend partial
// (src/platform/<backend>/essentials_text_to_speech.{cpp,mm}), reached through
// detail::make_text_to_speech() - the C# `defaultImplementation ??= new TextToSpeechImplementation()`.

#include "maui/essentials/text_to_speech.hpp"

#include <cctype>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace maui::media
{
    namespace
    {
        std::shared_ptr<i_text_to_speech>& text_to_speech_storage()
        {
            static std::shared_ptr<i_text_to_speech> storage;
            return storage;
        }

        // char.IsWhiteSpace / char.IsPunctuation, evaluated per UTF-8 byte. The chunker only needs
        // to find an ASCII break point (spaces/punctuation) to split long text, so a byte-wise test
        // is faithful for the contract; multi-byte code points are never break points (their bytes
        // are >= 0x80, neither space nor punct), which matches "keep the word together".
        bool is_break_char(char c)
        {
            const auto byte = static_cast<unsigned char>(c);
            return std::isspace(byte) != 0 || std::ispunct(byte) != 0;
        }
    } // namespace

    namespace detail
    {
        // Ported 1:1 from TextToSpeech.SplitSpeak (TextToSpeech.shared.cs): split text into parts of
        // at most `max` chars, preferring to break on the last whitespace/punctuation within the
        // window; if no break is found in the window, take the whole window.
        std::vector<std::string> split_speak(std::string_view text, std::size_t max)
        {
            std::vector<std::string> parts;
            if (text.length() <= max)
            {
                parts.emplace_back(text);
                return parts;
            }

            std::size_t position_begin = 0;
            std::size_t position_end = max;
            std::size_t position = position_begin;

            std::string p;
            while (position != text.length())
            {
                while (position_end > position_begin)
                {
                    if (position_end >= text.length())
                    {
                        // Just the rest of it.
                        p = std::string(text.substr(position_begin, text.length() - position_begin));
                        parts.push_back(p);
                        return parts;
                    }

                    const char ch = text[position_end];
                    if (is_break_char(ch))
                    {
                        p = std::string(text.substr(position_begin, position_end - position_begin));
                        break;
                    }
                    if (position_end == position_begin)
                    {
                        // No whitespace or punctuation found - grab the whole window (max).
                        p = std::string(text.substr(position_begin, max));
                        break;
                    }

                    --position_end;
                }

                position_begin = position_begin + p.length() + 1;
                position_end = position_begin + max;
                position = position_begin;

                parts.push_back(p);
            }

            return parts;
        }
    } // namespace detail

    i_text_to_speech& text_to_speech::default_()
    {
        auto& storage = text_to_speech_storage();
        if (storage == nullptr)
        {
            storage = detail::make_text_to_speech();
        }
        return *storage;
    }

    void text_to_speech::set_default(std::shared_ptr<i_text_to_speech> implementation)
    {
        text_to_speech_storage() = std::move(implementation);
    }
} // namespace maui::media
