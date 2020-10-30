#pragma once

#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sstream>
#include <map>
#include <ranges>
#include <variant>
#include "parser.hpp"
#include "iterator_util.hpp"

namespace clap {

template<class CharT>
struct basic_braced_clap {
    using string = std::basic_string<CharT>;
    using string_view = std::basic_string_view<CharT>;
    using string_index_type = typename string::size_type;
    class braced_arg;
    using option_t = std::variant<parser_with_arg<CharT>, braced_arg>;
    using options_map = std::map<string, option_t>;

    struct braced_arg {
        options_map options;

        braced_arg(std::initializer_list<std::pair<string, option_t>> il) {
            for(auto& p : il) options.insert(p);
        }
    };

protected:
    options_map options;
public:

    auto option(string_view name, parser_with_arg<CharT> parser) {
        options.insert({ string{name}, parser });
        return *this;
    }

    auto option(string_view name, braced_arg braced) {
        options.insert({ string{name}, braced });
        return *this;
    }

    void parse(const std::ranges::range auto& r) {
        parse(std::ranges::begin(r), std::ranges::end(r));
    }

    template<iterator_value_convertible_to_string_view<CharT> I>
    void parse(I begin, I end) {
        string_index_type i = 0;
        parse_option(begin, end, options, i);
    }

protected:
    template<iterator_value_convertible_to_string_view<CharT> I>
    static inline void
    parse_option(I& begin, I end, options_map& options, string_index_type& beginning) {
        using namespace std::literals::string_literals;

        auto str = [&]() {
            string_view s{*begin};
            s = s.substr(beginning);
            return s;
        };

        auto nextWord = [&]() {
            if(begin >= end) throw std::runtime_error("skipped end");
            begin++;
            beginning = 0;
        };

        auto nextChar = [&]() {
            if(str().size() <= 1) nextWord();
            else beginning++;
        };

        auto skipOrNextWord = [&](string_index_type chars) {
            if(chars > str().size())
                throw std::runtime_error(
                    "skipped too many chars ("s +
                    std::to_string(chars) +
                    ") for string '"s +
                     string{str()} +
                    "'"s
                );
            else if (chars == str().size()) nextWord();
            else beginning += chars;
        };

        while(begin != end) {
           auto eq_index = str().find('=');
           // option name, before '='
           string_view name = str().substr(0, eq_index);

           auto name_to_option = options.find(string{name});
           if(name_to_option == options.end()) throw std::runtime_error("can't find option with name '"+string{name}+"'");
           auto option = name_to_option->second;

           // skipping to '='
           if(eq_index == string::npos) {
               nextWord();

               if(str().front() != '=')
                   throw std::runtime_error("expected '=' after '"+string{name}+"'");
           }
           else skipOrNextWord(eq_index);

           // skip '='
           nextChar();

           // that's closure? diving in.
           if(str().front() == '{') {
               // skip '{'
               nextChar();

               parse_option(begin, end, std::get<braced_arg>(option).options, beginning);
               // skip '}'
               nextChar();
           }
           else {
               auto closing_index = str().find('}');
               auto arg = str().substr(0, closing_index);
               std::get<parser_with_arg<CharT>>(option) (arg);

               // we have '}' here, skipping it
               if(closing_index != string::npos) {
                   skipOrNextWord(closing_index);
                   return;
               }

               // we parsed arg, move on
               nextWord();
               // yeah, it could be in next word
               if(str().front() == '}') return;
           }
        }
    }

};

using braced_clap = basic_braced_clap<char>;
using wbraced_clap = basic_braced_clap<wchar_t>;
using u8braced_clap = basic_braced_clap<char8_t>;
using u16braced_clap = basic_braced_clap<char16_t>;
using u32braced_clap = basic_braced_clap<char32_t>;

}