#include <string>
#include <map>
#include <functional>

namespace clap {

template<class CharT>
struct basic_posix_clap {
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

    std::map<char, handler_t> handlers;
    
public:

    template<class Handler>
    void option(char name, Handler handler) {
        handlers.emplace(name, handler_t{handler, false});
    }

    template<class Handler>
    void required_option(char name, Handler handler) {
        handlers.emplace(name, handler_t{handler, true});
    }

    void flag(char name, bool& ref) {
        option(name, [&](){ ref = true; });
    }

    template<class Iter>
    void parse(Iter begin, Iter end) {
        for(auto arg = begin; arg != end; arg++) {
            auto first_char = (*arg)[0];
            if(first_char != '-') throw std::runtime_error("undefined argument: " + *arg);
            parse_option(arg, end);
        }

        std::for_each(handlers.begin(), handlers.end(), [](auto& pair_) {
            auto& [name, h] = pair_;
            if(h.required && !h.parsed)
                throw std::runtime_error("option \'"+str_t(1, name)+"\' is required");
            h.parsed=false;
        });
    }

protected:
    template<class Iter>
    void parse_option(Iter& b, Iter& e) {
        str_t arg{*b};
        for(int i = 1; i < arg.size(); i++) {
            char ch = arg.at(i);

            auto& handler = handlers.find(ch)->second;

            if(!handler.has_arg) {
                handler.parse(strv_t{});
                continue;
            }

            if(std::distance(arg.begin()+i+1, arg.end()) == 0) {
                b++;
                handler.parse(strv_t{arg.begin(), arg.end()});
            }
            else handler.parse(strv_t{arg.begin()+i+1, arg.end()});
            break;
        }
    }
};

using posix_clap = basic_posix_clap<char>;
using posix_wclap = basic_posix_clap<wchar_t>;
using posix_u8clap = basic_posix_clap<char8_t>;
using posix_u16clap = basic_posix_clap<char16_t>;
using posix_u32clap = basic_posix_clap<char32_t>;

}