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
        using namespace std;
        using namespace literals::string_literals;

        auto s = [&]() { return string_view{*begin}.substr(beginning); };
        auto is_end = [&](){ return begin == end; };

        auto nextWord = [&]() -> bool {
            ++begin;
            if(is_end()) return false;
            beginning = 0;
            return true;
        };

        auto skip = [&](string_index_type chars) {
            if(chars >= s().size())
                throw runtime_error {
                    "skipped too many chars ("s +
                    to_string(chars) +
                    ") for string '" +
                     string{s()} +
                    "'"
                };
            else beginning += chars;
        };

        auto nextChar = [&]() -> bool {
            if(s().size() == 1) return nextWord();
            else if(s().size() > 1) {
                skip(1);
                return true;
            }
            else throw runtime_error{"string size is zero"};
        };

        while(true) {
            string_index_type closest = string::npos;
            
            for(string_index_type index = 0; index < s().size(); index++)
                if(s()[index] == '=' || s()[index] == '{') {
                    closest = index;
                    break;
                }

            // option name, before '='
            string name = string{s().substr(0, closest)};

            auto name_to_option = options.find(name);
            if(name_to_option == options.end())
                throw runtime_error{"can't find option '"+name+"'"};
            auto& option = name_to_option->second;

            if(closest == string::npos) {
                if(!nextWord())
                    throw runtime_error{"there's no value for option '"+name+"'"};
            }
            else skip(closest);

            // '=' or '{'
            CharT ch = s().front();

            // skip '=' or '{'
            if(!nextChar())
                throw runtime_error{"unexpected end after '"+name+" "+ch+"'"};

            if(ch == '{') {
                parse_option(begin, end, get<braced_arg>(option).options, beginning);
                if(is_end() || s().front() != '}')
                    throw runtime_error{
                        "unexpected end of branced option '"+name+"'"
                    };
                if(!nextChar()) return;
            }
            else if(ch == '=') {
                auto closing_index = s().find('}');
                auto arg = s().substr(0, closing_index);
                if(arg.empty()) throw runtime_error{"value of option '"+name+"' is empty"};
                std::get<parser_with_arg<CharT>>(option) (arg);
 
                // we have '}' here. skipping and returning to parent
                if(closing_index != string::npos) {
                    skip(closing_index);
                    return;
                }
 
                // we parsed whole word as option argument, grab next
                if(!nextWord()) return;
                // it could be at the beginning of next word
                if(s().front() == '}') return;
            }
            else throw runtime_error {
                "undefined character '"s+ch+"' after option name '"+name+"'"
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