#include "posix_clap.hpp"

namespace clap {

template<class CharT>
struct basic_gnu_clap : basic_posix_clap<CharT> {
    using str_t = std::basic_string<CharT>;

    using typename basic_posix_clap<CharT>::handler_with_arg;
    using typename basic_posix_clap<CharT>::handler_without_arg;
    using typename basic_posix_clap<CharT>::strv_t;

    std::map<str_t, handler_with_arg> handlers_with_arg;
    std::map<str_t, handler_without_arg> handlers_without_arg;

    void option(char name, str_t long_name, handler_without_arg handler) {
        basic_posix_clap<CharT>::option(name, handler);
        option(long_name, handler);
    }

    void option(char name, str_t long_name, handler_with_arg handler) {
        basic_posix_clap<CharT>::option(name, handler);
        option(long_name, handler);
    }

    void option(str_t long_name, handler_with_arg handler) {
        handlers_with_arg.emplace(long_name, handler);
    }

    void option(str_t long_name, handler_without_arg handler) {
        handlers_without_arg.emplace(long_name, handler);
    }

    template<class Iter>
    void parse(Iter begin, Iter end) {
        for(auto arg = begin; arg != end; arg++) {
            char first_char = arg->at(0);
            if(first_char != '-') throw std::runtime_error("undefined argument: " + *arg);
            char second_char = arg->at(1);
            if(second_char != '-') basic_posix_clap<CharT>::parse_option(arg, end);
            else parse_option(arg, end);
        }
    }

protected:
    template<class Iter>
    void parse_option(Iter& arg, Iter& e) {
        auto eq_sign_pos = arg->find_first_of('=', 2);
        bool has_eq_sign = eq_sign_pos != str_t::npos;
        auto option_end = has_eq_sign ? arg->begin()+eq_sign_pos : arg->end();
        str_t option{arg->begin()+2, option_end};

        auto without_arg = handlers_without_arg.find(option);
        if(without_arg != handlers_without_arg.end()) {
            without_arg->second();
            return;
        }

        auto with_arg = handlers_with_arg[option];
        if(!has_eq_sign) throw std::runtime_error("option \'"+option+"\' must have an argument");
        auto arg_begin = option_end+1;
        if(std::distance(arg_begin, arg->end()) == 0)
            throw std::runtime_error("argument length for option \'" + option + "\' is zero");
        with_arg(strv_t{arg_begin, arg->end()});
    }
};

using gnu_clap = basic_gnu_clap<char>;
using gnu_wclap = basic_gnu_clap<wchar_t>;
using gnu_u8clap = basic_gnu_clap<char8_t>;
using gnu_u16clap = basic_gnu_clap<char16_t>;
using gnu_u32clap = basic_gnu_clap<char32_t>;

}