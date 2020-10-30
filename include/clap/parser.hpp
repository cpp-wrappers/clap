#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <sstream>
#include <type_traits>

#include "iterator_util.hpp"

namespace clap {
    template<class CharT>
    using parser_with_arg = std::function<void(std::basic_string_view<CharT>)>;
    using parser_without_arg = std::function<void(void)>;

    template<class CharT>
    inline parser_with_arg<CharT> value_parser(auto& val) {
        return [&val](std::basic_string_view<CharT> arg) {
            if constexpr(is_string_view_assignable_v<decltype(val), CharT>)
                val = arg;
            else
                std::istringstream{std::string{arg}/*sorry for that*/} >> val;
        };
    }

    inline parser_without_arg flag_parser(bool& val) {
        return [&val](){ val = true; };
    }
}