#include <string>
#include <map>
#include <functional>

namespace clap {

template<class CharT>
struct basic_posix_clap {
    using strv_t = std::basic_string_view<CharT>;

    using handler_with_arg = std::function<void(strv_t)>;
    using handler_without_arg = std::function<void()>;

    std::map<char, handler_with_arg> handlers_with_arg;
    std::map<char, handler_without_arg> handlers_without_arg;

    void flag(char name, bool& ref) {
        option(name, [&](){ ref = true; });
    }

    void option(char name, handler_with_arg handler) {
        handlers_with_arg.emplace(name, handler);
    }

    void option(char name, handler_without_arg handler) {
        handlers_without_arg.emplace(name, handler);
    }

    template<class Iter>
    void parse(Iter begin, Iter end) {
        for(auto arg = begin; arg != end; arg++) {
            auto first_char = arg->at(0);
            if(first_char != '-') throw std::runtime_error("undefined argument: " + *arg);
            parse_option(arg, end);
        }
    }

protected:
    template<class Iter>
    void parse_option(Iter& b, Iter& e) {
        for(int i = 1; i < b->size(); i++) {
            char ch = b -> at(i);

            auto without_arg = handlers_without_arg.find(ch);

            if(without_arg != handlers_without_arg.end()) {
                without_arg->second();
                continue;
            }

            auto with_arg = handlers_with_arg[ch];

            if(std::distance(b->begin()+i+1, b->end()) == 0) {
                b++;
                with_arg(strv_t{b->begin(), b->end()});
            }
            else with_arg(strv_t{b->begin()+i+1, b->end()});
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