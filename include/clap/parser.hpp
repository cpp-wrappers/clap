#pragma once

#include <functional>
#include <iterator>
#include <string>
#include <string_view>
#include <sstream>
#include <type_traits>
#include "cxx_util/string.hpp"
#include <iterator>
#include <cxx_util/multibyte_string.hpp>

namespace clap {
    template<class Encoding>
    using parser_with_arg = std::function<void(util::mb::basic_string_view<Encoding>)>;
    using parser_without_arg = std::function<void(void)>;

    template<class Encoding>
    inline parser_with_arg<Encoding> value_parser(auto& val) {
        return [&val](util::mb::basic_string_view<Encoding> arg) {
            if constexpr(util::is_assignable_from_string_view_v<Encoding::char_type, decltype(val)>)
                val = arg.to_string_view();
            else
                std::basic_istringstream<typename Encoding::char_type>{
                    arg.to_string()/*sorry for that*/
                } >> val;
        };
    }

    inline parser_without_arg flag_parser(bool& val) {
        return [&val]() {
            val = true;
        };
    }

    template<class Encoding, class T>
    inline parser_with_arg<Encoding> values_parser(std::output_iterator<T> auto oit) {
        return [oit](util::mb::basic_string_view<Encoding> arg) mutable {
            if constexpr(util::is_constructible_from_string_view_v<Encoding::char_type, T>)
                *oit++ = T{arg};
            else {
                T t;
                std::basic_istringstream<typename Encoding::char_type>{
                    arg.to_string()/*sorry for that*/
                } >> t;
                *oit++ = std::move(t);
            }
        };
    }
}