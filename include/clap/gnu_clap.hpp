#include "posix_clap.hpp"

namespace gnu {

template<class CharT>
struct basic_clap : protected posix::basic_clap<CharT> {
    using strv_t = std::basic_string_view<CharT>;
    using str_t = std::basic_string<CharT>;

    using posix::basic_clap<CharT>::parse_operand;
	using option_t = typename posix::basic_clap<CharT>::option_t;

    std::map<str_t, option_t> options;
    std::map<str_t, CharT> long_to_short_names;
	
	using posix::basic_clap<CharT>::option;

	option_t& option(strv_t long_name) {
        auto [iter, success] = options.emplace(long_name, option_t{});
		return iter->second;
    }

    option_t& option(CharT name, strv_t long_name) {
        option_t& o = posix::basic_clap<CharT>::option(name);
        long_to_short_names.emplace(long_name, name);
        return o;
    }

	using posix::basic_clap<CharT>::flag;

	auto flag(strv_t long_name, bool& ref) {
		return posix::basic_clap<CharT>::flag(option(long_name), ref);
    }

    template<std::input_iterator It>
    void parse(
        const It begin,
        const It end,
        std::function<void(const It, It&, const It)> operand_parser = [](const It, It&, const It){}
    ) {
        It arg = begin;
        while(arg != end) {
            char first_char = (*arg)[0];
            if(first_char == '-') {
                char second_char = (*arg)[1];
                It prev = arg;

                if(second_char != '-') 
                    posix::basic_clap<CharT>::parse_one_hyphen_arg(begin, arg, end);
                else if( (*arg)[2] )
                    parse_two_hyphen_arg(begin, arg, end);
                else
                    if(++arg == end) break; // skip '--'

                if(prev != arg)
                        continue;
            }

            parse_operand(begin, arg, end, operand_parser);
        }
    }

protected:
    option_t* option_by_name(strv_t name) {
        auto hame_to_option = options.find(str_t{name});
        if(hame_to_option == options.end()) {
            auto long_to_short_name = long_to_short_names.find(str_t{name});
            if(long_to_short_name == long_to_short_names.end())
                return nullptr;
            return posix::basic_clap<CharT>::option_by_name(long_to_short_name->second);
        }
        return &(hame_to_option->second);
    }

    template<class It>
    void parse_two_hyphen_arg(const It begin, It& arg_it, const It e) {
        strv_t arg{*arg_it};

        auto option_name_beg_pos = 2; // skip '--'
        auto option_name_beg = arg.begin()+option_name_beg_pos;
        auto eq_sign_pos = arg.find_first_of('=', option_name_beg_pos);
        bool has_eq_sign = eq_sign_pos != str_t::npos;
        auto option_name_end = has_eq_sign ? arg.begin()+eq_sign_pos : arg.end() ;

        strv_t option_name{option_name_beg, option_name_end};

        auto option = option_by_name(option_name);
        if(!option) return;

        if(!option->has_arg())
            option->parser()({});
        else {
            if(!has_eq_sign) throw std::runtime_error("option '"+str_t{option_name}+"' must have an argument");
            auto option_arg_beg = arg.begin()+eq_sign_pos+1; // skip '='
            strv_t option_arg{option_arg_beg, arg.end()};
            if(option_arg.empty())
                throw std::runtime_error("argument length for option '"+str_t{option_name}+"' is zero");
            option->parser()(option_arg);
        }

        ++arg_it;
    }
};

using clap = basic_clap<char>;
using wclap = basic_clap<wchar_t>;
using u8clap = basic_clap<char8_t>;
using u16clap = basic_clap<char16_t>;
using u32clap = basic_clap<char32_t>;

}
