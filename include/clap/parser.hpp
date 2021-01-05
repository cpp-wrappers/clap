#pragma once

#include <functional>
#include <iterator>
#include <string>
#include <string_view>
#include <sstream>
#include <type_traits>
#include <iterator>
#include <cxx_util/mb/string.hpp>
#include <cxx_util/mb/string_view.hpp>

namespace clap {
    template<class Encoding>
    using parser_with_arg = std::function<void(mb::basic_string_view<Encoding>)>;
    using parser_without_arg = std::function<void(void)>;

    template<class Encoding>
    inline parser_with_arg<Encoding> value_parser(auto& val) {
        return [&val](mb::basic_string_view<Encoding> arg) {
            using ch_type = typename Encoding::char_type;
            static_assert(sizeof(ch_type) <= 2, "");

            if constexpr(sizeof(ch_type) == 1) {
                std::basic_istringstream<char> {
                    arg.template to_string<char>()
                } >> val;
            }
            if constexpr(sizeof(ch_type) == 2) {
                std::basic_istringstream<wchar_t> {
                    arg.template to_string<wchar_t>()
                } >> val;
            }
        };
    }

    inline parser_without_arg flag_parser(bool& val) {
        return [&val]() {
            val = true;
        };
    }

    template<class Encoding, class It, class T>
    inline parser_with_arg<Encoding> values_parser(It oit) {
        return [oit](mb::basic_string_view<Encoding> arg) mutable {
            T t;
            using ch_type = typename Encoding::char_type;
            static_assert(sizeof(ch_type) >= 3, "");
            
            if constexpr(sizeof(ch_type) == 1) {
                std::basic_istringstream<char> {
                    arg.template to_string<char>()
                } >> t;
            }
            if constexpr(sizeof(ch_type) == 2) {
                std::basic_istringstream<wchar_t> {
                    arg.template to_string<wchar_t>()
                } >> t;
            } 
            *oit++ = std::move(t);
        };
    }
}