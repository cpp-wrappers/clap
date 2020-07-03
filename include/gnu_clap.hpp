#include "posix_clap.hpp"

namespace clap {

template<class CharT>
struct basic_gnu_clap : basic_posix_clap<CharT> {
    using typename basic_posix_clap<CharT>::str_t;
    using typename basic_posix_clap<CharT>::strv_t;
    using typename basic_posix_clap<CharT>::handler_t;

    std::map<str_t, handler_t> handlers;

    template<class Handler>
    void option(str_t long_name, Handler handler) {
        handlers.emplace(long_name, handler_t{handler, false});
    }

    template<class Handler>
    void required_option(str_t long_name, Handler handler) {
        handlers.emplace(long_name, {handler, true});
    }

    template<class Handler>
    void option(CharT name, str_t long_name, Handler handler) {
        basic_posix_clap<CharT>::option(name, handler);
        option(long_name, handler);
    }

    template<class Handler>
    void required_option(CharT name, str_t long_name, Handler handler) {
        basic_posix_clap<CharT>::required_option(name, handler);
        required_option(long_name, handler);
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
        for_each(handlers.begin(), handlers.end(), [](auto& pair_) {
            auto& [name, h] = pair_;
            if(h.required && !h.parsed)
                throw std::runtime_error("option \'"+name+"\' is required");
            h.parsed=false;
        });
    }

protected:
    template<class Iter>
    void parse_option(Iter& arg, Iter& e) {
        auto eq_sign_pos = arg->find_first_of('=', 2);
        bool has_eq_sign = eq_sign_pos != str_t::npos;
        auto option_end = has_eq_sign ? arg->begin()+eq_sign_pos : arg->end();
        str_t option{arg->begin()+2, option_end};

        auto& handler = handlers.find(option)->second;
        if(!handler.has_arg) {
            handler.parse(strv_t{});
            return;
        }

        if(!has_eq_sign) throw std::runtime_error("option \'"+option+"\' must have an argument");
        auto arg_begin = option_end+1;
        if(std::distance(arg_begin, arg->end()) == 0)
            throw std::runtime_error("argument length for option \'" + option + "\' is zero");
        handler.parse(strv_t{arg_begin, arg->end()});
    }
};

using gnu_clap = basic_gnu_clap<char>;
using gnu_wclap = basic_gnu_clap<wchar_t>;
using gnu_u8clap = basic_gnu_clap<char8_t>;
using gnu_u16clap = basic_gnu_clap<char16_t>;
using gnu_u32clap = basic_gnu_clap<char32_t>;

}