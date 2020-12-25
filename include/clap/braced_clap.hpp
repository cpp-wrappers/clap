#pragma once

#include <initializer_list>
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
class basic_braced_clap;

template<class CharT>
struct basic_braced_arg {
    using braced_arg = basic_braced_arg<CharT>;
    using string = std::basic_string<CharT>;
    using string_view = std::basic_string_view<CharT>;
    using option_t = std::variant<parser_with_arg<CharT>, braced_arg>;
    using options_map = std::map<string, option_t, std::less<>>;
    using raw_options = std::initializer_list<std::pair<string_view, option_t>>;

    options_map options;

    basic_braced_arg(){}

    auto& option(string_view name, parser_with_arg<CharT>&& parser) {
        options.emplace(name, std::move(parser));
        return *this;
    }

    auto& option(string_view name, braced_arg&& braced) {
        options.emplace(name, std::move(braced));
        return *this;
    }

    auto& value(string_view name, auto& value) {
        return option(name, value_parser<CharT>(value));
    }

    auto& braced(string_view name, raw_options options) {
        braced_arg arg;
        for(auto& a : options) { arg.options.emplace(a.first, std::move(a.second)); }
        return option(name, std::move(arg));
    }
};

using braced_arg = basic_braced_arg<char>;
using wbraced_arg = basic_braced_arg<wchar_t>;
using u8braced_arg = basic_braced_arg<char8_t>;
using u16braced_arg = basic_braced_arg<char16_t>;
using u32braced_arg = basic_braced_arg<char32_t>;

template<class CharT>
struct basic_braced_clap {
    using braced_arg = basic_braced_arg<CharT>;
    using string = std::basic_string<CharT>;
    using string_view = std::basic_string_view<CharT>;
    using option_t = typename braced_arg::option_t;
    using options_map = typename braced_arg::options_map;
    using raw_options = typename std::initializer_list<std::pair<string_view, option_t>>;
    using string_index_type = typename string::size_type;

    auto& option(string_view name, parser_with_arg<CharT>&& parser) {
        root.option(name, std::move(parser));
        return *this;
    }

    auto& option(string_view name, braced_arg&& braced) {
        root.option(name, std::move(braced));
        return *this;
    }

    auto& braced(string_view name, raw_options options) {
        root.braced(name, options);
        return * this;
    }

    void parse(const std::ranges::range auto& r) {
        parse(std::ranges::begin(r), std::ranges::end(r));
    }

    template<iterator_value_convertible_to_string_view<CharT> I>
    void parse(I begin, I end) {
        string_index_type i = 0;
        parse_option(begin, end, root.options, i);
        if(begin != end) throw std::runtime_error{"unexpected '}'"};
    }

protected:
    braced_arg root;

    template<iterator_value_convertible_to_string_view<CharT> I>
    static inline void
    parse_option(I& begin, I end, options_map& options, string_index_type& beginning) {
        using namespace std;
        using namespace literals::string_literals;

        auto s = [&]() { return string_view{*begin}.substr(beginning); };
        auto is_end = [&](){ return begin == end; };

        auto next_word = [&]() -> bool {
            ++begin;
            if(is_end()) return false;
            beginning = 0;
            return true;
        };

        auto skip = [&](string_index_type chars) {
            if(chars >= s().size())
                throw runtime_error {
                    "skipped too many characters ("s +
                    to_string(chars) +
                    ") for string '" +
                     string{s()} +
                    "'"
                };
            else beginning += chars;
        };

        auto next_char = [&]() -> bool {
            if(s().size() == 1) return next_word();
            else if(s().size() > 1) {
                skip(1);
                return true;
            }
            else throw runtime_error{"string size is zero"};
        };

        while(begin != end) {
            if(s().front() == '}') return;

            string_index_type closest_index = string::npos;
            
            for(string_index_type index = 0; index < s().size(); index++)
                if(s()[index] == '=' || s()[index] == '{') {
                    closest_index = index;
                    break;
                }

            // option name, before '='
            string_view name = s().substr(0, closest_index);
            if(name.empty()) throw runtime_error { "option name is empty" };

            auto name_to_option = options.find(name);
            if(name_to_option == options.end())
                throw runtime_error{"can't find option '"+string{name}+"'"};
            auto& option = name_to_option->second;

            if(closest_index == string::npos) {
                if(!next_word())
                    throw runtime_error{"there's no value for option '"+string{name}+"'"};
            }
            else skip(closest_index);

            // '=' or '{'
            CharT ch = s().front();

            // skip '=' or '{'
            if(!next_char())
                throw runtime_error{"unexpected end after '"+string{name}+" "+ch+"'"};

            if(ch == '{') {
                parse_option(begin, end, get<braced_arg>(option).options, beginning);
                if(is_end() || s().front() != '}')
                    throw runtime_error{
                        "unexpected end of branced option '"+string{name}+"'"
                    };
                if(!next_char()) return;
            }
            else if(ch == '=') {
                auto closing_index = s().find('}');
                auto arg = s().substr(0, closing_index);
                if(arg.empty()) throw runtime_error{"value of option '"+string{name}+"' is empty"};
                std::get<parser_with_arg<CharT>>(option) (arg);
 
                // we have '}' here. skipping and returning to parent
                if(closing_index != string::npos) {
                    skip(closing_index);
                    return;
                }
 
                // we parsed whole word as option argument, grab next
                if(!next_word()) return;
                // it could be at the beginning of next word
            }
            else throw runtime_error {
                "undefined character '"s+ch+"' after option name '"+string{name}+"'"
            };
        }
    }

};

using braced_clap = basic_braced_clap<char>;
using wbraced_clap = basic_braced_clap<wchar_t>;
using u8braced_clap = basic_braced_clap<char8_t>;
using u16braced_clap = basic_braced_clap<char16_t>;
using u32braced_clap = basic_braced_clap<char32_t>;

}