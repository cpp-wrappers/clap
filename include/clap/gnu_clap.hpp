#include "posix_clap.hpp"

namespace gnu {

template<class CharT>
struct basic_clap : protected posix::basic_clap<CharT> {
    using this_t = basic_clap<CharT>;
    using typename posix::basic_clap<CharT>::str_t;
    using typename posix::basic_clap<CharT>::strv_t;
    using typename posix::basic_clap<CharT>::handler_t;

    std::map<strv_t, handler_t> handlers;

    this_t option(strv_t long_name, auto handler) {
        handlers.emplace(long_name, handler_t{handler, false});
        return *this;
    }

    this_t required_option(strv_t long_name, auto handler) {
        handlers.emplace(long_name, {handler, true});
        return *this;
    }

    this_t option(CharT name, strv_t long_name, auto handler) {
        posix::basic_clap<CharT>::option(name, handler);
        option(long_name, handler);
        return *this;
    }

    this_t required_option(CharT name, strv_t long_name, auto handler) {
        posix::basic_clap<CharT>::required_option(name, handler);
        required_option(long_name, handler);
        return *this;
    }

    this_t flag(strv_t long_name, bool& ref) {
        option(long_name, [&](){ ref = true; });
        return *this;
    }

    template<class It>
    void parse(
        const It begin,
        const It end,
        std::function<void(const It, It&, const It)> operands_handler = [](const It, It&, const It){}
    ) {
        It cl_arg = begin;
        for(; cl_arg != end; cl_arg++) {
            char first_char = (*cl_arg)[0];
            if(first_char == '-') {
                char second_char = (*cl_arg)[1];
                if(second_char != '-') {
                    posix::basic_clap<CharT>::parse_option(begin, cl_arg, end);
                    continue;
                }
                else if(cl_arg->size()>2){
                    It prev = cl_arg;
                    parse_option(begin, cl_arg, end);
                    if(prev != cl_arg)
                        continue;
                }
                else
                    if(++cl_arg == end) break; // skip '--'
            }

            It prev = cl_arg;
            operands_handler(begin, cl_arg, end);
            if(prev == cl_arg)
                    throw std::runtime_error("operand isn't parsed: "+*cl_arg);
        }
        for(auto& [name, h] : handlers) {
            if(h.required && !h.parsed)
                throw std::runtime_error("option \'"+str_t{name}+"\' is required");
            h.parsed=false;
        }
    }

protected:
    template<class It>
    void parse_option(const It begin, It& arg, const It e) {
        auto option_beg_pos = 2; // skip '--'
        auto option_beg = arg->begin()+option_beg_pos;
        auto eq_sign_pos = arg->find_first_of('=', option_beg_pos);
        bool has_eq_sign = eq_sign_pos != str_t::npos;
        auto option_end = has_eq_sign ? arg->begin()+eq_sign_pos : arg->end();

        strv_t option{option_beg, option_end};

        auto pair = handlers.find(option);
        if(pair == handlers.end()) return;
        auto& handler = pair->second;

        if(!handler.has_arg) {
            handler.parse({});
            return;
        }

        if(!has_eq_sign) throw std::runtime_error("option '"+str_t{option}+"' must have an argument");
        auto arg_beg = option_end+1; // skip '='
        strv_t op_arg{arg_beg, arg->end()};
        if(op_arg.empty())
            throw std::runtime_error("argument length for option '"+str_t{option}+"' is zero");
        handler.parse(op_arg);
    }
};

using clap = basic_clap<char>;
using wclap = basic_clap<wchar_t>;
using u8clap = basic_clap<char8_t>;
using u16clap = basic_clap<char16_t>;
using u32clap = basic_clap<char32_t>;

}