#include <string>
#include <map>
#include <functional>

namespace posix {

template<class CharT>
struct basic_clap {
    using this_t = basic_clap<CharT>;
    using str_t = std::basic_string<CharT>;
    using strv_t = std::basic_string_view<CharT>;
    using parse_a_t = std::function<void(strv_t)>;
    using parse_t = std::function<void()>;
protected:

    struct handler_t {
        parse_a_t on_parse;
        bool has_arg;

        bool required = false;
        bool parsed = false;

        handler_t(
            parse_a_t parse,
            bool required
        )
        :on_parse{parse},has_arg{true},required{required}{}

        handler_t(
            parse_t parse,
            bool required
        )
        :on_parse{[=](strv_t){ parse(); }},has_arg{false},required{required}{}

        void parse(strv_t arg) {
            on_parse(arg);
            parsed = true;
        }
    };

    std::map<CharT, handler_t> handlers;
    
public:

    this_t option(CharT name, auto handler) {
        handlers.emplace(name, handler_t{handler, false});
        return *this;
    }

    this_t required_option(CharT name, auto handler) {
        handlers.emplace(name, handler_t{handler, true});
        return *this;
    }

    this_t flag(CharT name, bool& ref) {
        option(name, [&](){ ref = true; });
        return *this;
    }

    template<class It>
    void parse(
        const It begin,
        const It end,
        std::function<void(const It, It&, const It)> operands_handler = [](const It, It&, const It){}
    ) {
        It arg = begin;
        while(arg != end) {
            auto first_char = (*arg)[0];
            auto second_char = (*arg)[1];
            if(first_char != '-') break;

            if(second_char != '-') {
                It prev = arg;
                parse_option(begin, arg, end);
                if(prev == arg)
                    break;
            }
            else if((*arg)[2] == 0){
                arg++; // skip '--', point to operands beg
                break;
            }
        }

        while(arg != end) 
            parse_operand(begin, arg, end, operands_handler);

        on_parse_end();
    }

protected:

    void on_parse_end() {
        for(auto& [name, h] : handlers) {
            if(h.required && !h.parsed)
                throw std::runtime_error("option \'"+str_t{1, name}+"\' is required");
            h.parsed=false;
        }
    }

    handler_t* find(CharT name) {
        auto pair = handlers.find(name);
        return pair == handlers.end() ? nullptr : &(pair->second);
    }

    template<class It>
    void parse_operand(
        const It begin,
        It& arg,
        const It end,
        std::function<void(const It, It&, const It)> operands_handler
    ) {
        It prev = arg;
        operands_handler(begin, arg, end);
        if(prev == arg)
            throw std::runtime_error("operand isn't parsed: "+str_t{*arg});
    }

    template<class It>
    void parse_option(const It begin, It& arg_it, const It end) {
        strv_t arg{*arg_it};

        auto first_ch = arg.begin()+1; // skip '-'

        for(auto ch = first_ch;*ch; ch++) {
            char name = *ch;
            auto handler = find(name);
            if(!handler) {
                if(ch == first_ch)
                    return; // assuming that's operand
                else throw std::runtime_error("undefined option: "+str_t{1, name});
            }

            if(!handler->has_arg) {
                handler->parse({});
                continue;
            }

            ++ch; // to end or option argument beginning
            bool next_arg = ch == arg.end();

            if(next_arg) {
                if(++arg_it==end)
                    throw std::runtime_error("argument is required for option '"+str_t{1, name}+"'");
                arg = *arg_it;
                ch = arg.begin();
            }

            handler->parse({ch});

            break;
        }

        arg_it++;
    }
};

using clap = basic_clap<char>;
using wclap = basic_clap<wchar_t>;
using u8clap = basic_clap<char8_t>;
using u16clap = basic_clap<char16_t>;
using u32clap = basic_clap<char32_t>;

}