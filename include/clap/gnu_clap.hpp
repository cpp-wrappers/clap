#pragma once

#include "posix_clap.hpp"
#include <algorithm>
#include <cwchar>
#include <cxx_util/multibyte_string.hpp>
#include <functional>
#include <iterator>
#include <locale>
#include <string_view>
#include "cxx_util/encoding.hpp"

namespace clap {

namespace gnu {

template<class Encoding>
struct basic_clap : protected posix::basic_clap<Encoding> {
protected:
    using char_type = typename Encoding::char_type;
    using string_view_type = std::basic_string_view<char_type>;
    using string_type = std::basic_string<char_type>;
	using base_t = posix::basic_clap<Encoding>;

	using option_t = typename base_t::option_t;
    std::map<util::mb::basic_string<Encoding>, const option_t, std::less<>> options;
    std::map<util::mb::basic_string<Encoding>, util::mb::character<Encoding>, std::less<>> long_to_short_names;

public:
	using base_t::option;

	auto& option(util::mb::basic_string<Encoding> long_name, auto parser) {
        options.emplace(long_name, option_t{parser});
		return *this;
    }

    auto& option(util::mb::character<Encoding> name, util::mb::basic_string<Encoding> long_name, auto parser) {
        base_t::option(name, parser);
        long_to_short_names.emplace(long_name, name);
        return *this;
    }

	auto& flag(util::mb::basic_string<Encoding> long_name, bool& val) { return option(long_name, flag_parser(val)); }
	auto& flag(util::mb::character<Encoding> name, util::mb::basic_string<Encoding> long_name, bool& val) { return option(name, long_name, flag_parser(val)); }

	auto& value(util::mb::basic_string<Encoding> long_name, auto& val) {
        return option(long_name, value_parser<Encoding>(val));
    }
	auto& value(util::mb::character<Encoding> name, util::mb::basic_string<Encoding> long_name, auto& val) {
        return option(name, long_name, value_parser<Encoding>(val));
    }

    template<class T>
    auto& values(util::mb::character<Encoding> name, util::mb::basic_string<Encoding> long_name, auto output_it) {
        return option(name, long_name, values_parser<Encoding, T>(output_it));
    }
	
	using base_t::parse_operand;

    template<std::ranges::range R, class It = std::ranges::iterator_t<R>>
    void parse(
        R& range,
        std::function<void(const It, It&, const It)> operand_parser = {}
    ) const {
        parse(range.begin(), range.end(), operand_parser);
    }

    template<std::input_iterator It>
    void parse(
        const It begin,
        const It end,
       	std::function<void(const It, It&, const It)> operand_parser = {},
        std::locale locale = {}
    ) const {
        //std::ctype<ACharT>& ctype_facet = std::use_facet<std::ctype<ACharT>>(locale);

        It arg = begin;
        while(arg != end) {
            //ctype_facet.widen(arg.begin(), arg.end());
            util::mb::basic_string_view<Encoding> str {*arg};

            char first_char = (*arg)[0];
            if(first_char == '-') {
                char second_char = (*arg)[1];
                It prev = arg;

                if(second_char != '-') 
                    base_t::parse_one_hyphen_arg(begin, arg, end);
                else if( (*arg)[2] )
                    parse_two_hyphen_arg(begin, arg, end);
                else if(++arg == end) break; // skip '--'

                if(prev != arg) continue;
            }

            parse_operand(begin, arg, end, operand_parser);
        }
    }

protected:
    template<class Encoding0>
    const option_t* option_by_name(util::mb::basic_string<Encoding0> name) const {
        auto hame_to_option = options.find(name);
        if(hame_to_option == options.end()) {
            auto long_to_short_name = long_to_short_names.find(name);
            if(long_to_short_name == long_to_short_names.end())
                return nullptr;
            return base_t::option_by_name(long_to_short_name->second);
        }
        return &(hame_to_option->second);
    }

    template<class Encoding0, class It>
    void parse_two_hyphen_arg(const It begin, It& arg_it, const It e) const {
        using namespace std;
        util::mb::basic_string_view<Encoding0> arg{*arg_it};

        auto option_name_beg_pos = 2; // skip '--'
        auto option_name_beg = arg.begin()+option_name_beg_pos;
        auto eq_sign_pos = arg.find_first_of('=', option_name_beg_pos);
        bool has_eq_sign = eq_sign_pos != string::npos;
        auto option_name_end = has_eq_sign ? arg.begin()+eq_sign_pos : arg.end() ;

        util::mb::basic_string_view<Encoding0> option_name{option_name_beg, option_name_end};

        auto option = option_by_name<Encoding0>(option_name);

        //if(std::all_of(option_name.begin(), option_name.end(), [](CharT ch){ return ch <= 0x8F; }))
            //option = option_by_name(option_name);
        
        if(!option) return;

        if(!option->has_arg())
            get<parser_without_arg>(option->parser()) ();
        else {
            if(!has_eq_sign) throw runtime_error{"option '"+string{option_name}+"' must have an argument"};
            auto option_arg_beg = arg.begin()+eq_sign_pos+1; // skip '='
            util::mb::basic_string_view<Encoding0> option_arg{option_arg_beg, arg.end()};
            if(option_arg.empty())
                throw runtime_error{"argument length for option '"+string{option_name}+"' is zero"};
            get<parser_with_arg<Encoding>>(option->parser()) (option_arg);
        }

        ++arg_it;
    }
};

}

}

namespace gnu {
    using namespace clap::gnu;
}