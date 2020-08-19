#include "posix_clap.hpp"

namespace gnu {

template<class CharT>
struct basic_clap : protected posix::basic_clap<CharT> {
protected:
    using strv_t = std::basic_string_view<CharT>;
    using str_t = std::basic_string<CharT>;
	using base_t = posix::basic_clap<CharT>;

	using option_t = typename base_t::option_t;

    std::map<str_t, option_t> options;
    std::map<str_t, CharT> long_to_short_names;
public:
	using base_t::option;

	auto& option(strv_t long_name, auto parser) {
        options.emplace(long_name, option_t{parser});
		return *this;
    }

    auto& option(CharT name, strv_t long_name, auto parser) {
        base_t::option(name, parser);
        long_to_short_names.emplace(long_name, name);
        return *this;
    }

	auto& flag(strv_t long_name, bool& val) { return option(long_name, base_t::option_parser(val)); }
	auto& flag(CharT name, strv_t long_name, bool& val) { return option(name, long_name, base_t::option_parser(val)); }

	auto& option(strv_t long_name, str_t& str) { return option(long_name, base_t::option_parser(str)); }
	auto& option(CharT name, strv_t long_name, str_t& str) { return option(name, long_name, base_t::option_parser(str)); }
	
	using base_t::parse_operand;

    template<std::ranges::range R, class It = std::ranges::iterator_t<R>>
    void parse(
        const R& range,
        std::function<void(const It, It&, const It)> operand_parser = {}
    ) {
        parse(range.begin(), range.end(), operand_parser);
    }

    template<posix::iterator_value_convertible_to_string_view<CharT> It>
    void parse(
        const It begin,
        const It end,
       	std::function<void(const It, It&, const It)> operand_parser = {}
    ) {
        It arg = begin;
        while(arg != end) {
            char first_char = (*arg)[0];
            if(first_char == '-') {
                char second_char = (*arg)[1];
                It prev = arg;

                if(second_char != '-') 
                    base_t::parse_one_hyphen_arg(begin, arg, end);
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
            return base_t::option_by_name(long_to_short_name->second);
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
