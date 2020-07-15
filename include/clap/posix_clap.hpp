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
        auto cl_arg = begin;
        for(; cl_arg != end; cl_arg++) {
            auto first_char = (*cl_arg)[0];
            auto second_char = (*cl_arg)[1];
            if(first_char != '-') break;

            if(second_char != '-') {
                It prev = cl_arg;
                parse_option(begin, cl_arg, end);
                if(prev == cl_arg)
                    break;
            }
            else if(cl_arg->size()==2){
                cl_arg++; // skip '--', point to operands beg
                break;
            }
        }

        for(; cl_arg != end; cl_arg++) {
            It prev = cl_arg;
            operands_handler(begin, cl_arg, end);
            if(prev == cl_arg)
                throw std::runtime_error("operand isn't parsed: "+*cl_arg);
        }

        for(auto& [name, h] : handlers) {
            if(h.required && !h.parsed)
                throw std::runtime_error("option \'"+str_t{1, name}+"\' is required");
            h.parsed=false;
        }
    }

protected:
    template<class It>
    void parse_option(const It being, It& cl_arg, const It end) {
        auto first_ch = cl_arg->c_str()+1; // skip '-'
        for(auto ch = first_ch;*ch; ch++) {
            char name = *ch;
            auto pair = handlers.find(name);
            if(pair == handlers.end()) {
                if(ch == first_ch)
                    return; // assuming that's operand
                else throw std::runtime_error("undefined option: "+str_t{1, name});
            }
            auto& handler = pair->second;

            if(!handler.has_arg) {
                handler.parse({});
                continue;
            }

            ++ch; // to end or option argument beginning
            bool next_arg = !*ch;

            if(next_arg) {
                if(++cl_arg==end)
                    throw std::runtime_error("argument is required for option '"+str_t{1, name}+"'");
                ch = cl_arg->c_str();
            }

            handler.parse({ch});

            return;
        }
    }
};

using clap = basic_clap<char>;
using wclap = basic_clap<wchar_t>;
using u8clap = basic_clap<char8_t>;
using u16clap = basic_clap<char16_t>;
using u32clap = basic_clap<char32_t>;

}