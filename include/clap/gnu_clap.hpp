#pragma once

#include "posix_clap.hpp"
#include <algorithm>
#include <cwchar>
#include <cxx_util/mb/string.hpp>
#include <functional>
#include <iterator>
#include <string_view>
#include "cxx_util/encoding/encoding.hpp"

namespace clap {

namespace gnu {

template<enc::encoding Encoding>
struct basic_clap : protected posix::basic_clap<Encoding> {
protected:
    template<enc::encoding Encoding0>
    using character = mb::character<Encoding0>;
    template<enc::encoding Encoding0>
    using character_view = mb::character_view<Encoding0>;
    
    template<enc::encoding Encoding0>
    using string = mb::basic_string<Encoding0>;
    template<enc::encoding Encoding0>
    using string_view = mb::basic_string_view<Encoding0>;

    template<class It>
    using operands_parser_t = std::function<void(const It, It&, const It)>;

	using base_t = posix::basic_clap<Encoding>;

	using option_t = typename base_t::option_t;
    
    std::map<string<Encoding>, const option_t, std::less<>> options;
    std::map<string<Encoding>, character<Encoding>, std::less<>> long_to_short_names;

public:
	using base_t::option;

	auto& option(string<Encoding> long_name, auto parser) {
        options.emplace(long_name, option_t{parser});
		return *this;
    }

    auto& option(character<Encoding> name, string<Encoding> long_name, auto parser) {
        base_t::option(name, parser);
        options.emplace(long_name, option_t{parser});
        long_to_short_names.emplace(long_name, name);
        return *this;
    }

	auto& flag(string<Encoding> long_name, bool& val) { return option(long_name, flag_parser(val)); }
	auto& flag(character<Encoding> name, string<Encoding> long_name, bool& val) { return option(name, long_name, flag_parser(val)); }

	auto& value(string<Encoding> long_name, auto& val) {
        return option(long_name, value_parser<Encoding>(val));
    }
	auto& value(character<Encoding> name, string<Encoding> long_name, auto& val) {
        return option(name, long_name, value_parser<Encoding>(val));
    }

    template<class T>
    auto& values(character<Encoding> name, string<Encoding> long_name, auto output_it) {
        return option(name, long_name, values_parser<Encoding, T>(output_it));
    }
	
	using base_t::parse_operand;

    /*template<enc::encoding Encoding0, std::ranges::range R, class It = std::ranges::iterator_t<R>>
    void parse(
        R& range,
        operands_parser_t<It> operand_parser = {}
    ) const {
        parse<Encoding0, It>(range.begin(), range.end(), operand_parser);
    }*/

    template<enc::encoding Encoding0, class InputIt>
    void parse(
        const InputIt begin,
        const InputIt end,
       	operands_parser_t<InputIt> operand_parser = {}
    ) const {
        InputIt arg = begin;
        while(arg != end) {
            string_view<Encoding> str {*arg};

            auto first_char = str[0];
            if(first_char == '-') {
                auto second_char = str[1];
                InputIt prev = arg;

                if(second_char != '-') 
                    base_t::template parse_one_hyphen_arg<Encoding0, InputIt>(begin, arg, end);
                else if( str.size() > 2 )
                    parse_two_hyphen_arg<Encoding0, InputIt>(begin, arg, end);
                else if(++arg == end) break; // skip '--'

                if(prev != arg) continue;
            }

            parse_operand(begin, arg, end, operand_parser);
        }
    }

protected:
    template<enc::encoding Encoding0>
    const option_t* option_by_name(string_view<Encoding0> name) const {
        auto hame_to_option = options.find(name.template convert<Encoding>());
        if(hame_to_option == options.end()) {
            auto long_to_short_name = long_to_short_names.find(name);
            if(long_to_short_name == long_to_short_names.end())
                return nullptr;
            return base_t::template option_by_name<Encoding0>(long_to_short_name->second);
        }
        return &(hame_to_option->second);
    }

    template<enc::encoding Encoding0, class It>
    void parse_two_hyphen_arg(const It begin, It& arg_it, const It e) const {
        using namespace std;
        string_view<Encoding0> arg{*arg_it};

        auto option_name_beg_pos = 2; // skip '--'
        auto option_name_beg = arg.begin()+option_name_beg_pos;
        auto eq_sign_pos = arg.find_first_of('=', option_name_beg_pos);
        bool has_eq_sign = eq_sign_pos != string_view<Encoding0>::npos;
        auto option_name_end = has_eq_sign ? arg.begin()+eq_sign_pos : arg.end() ;

        string_view<Encoding0> option_name{option_name_beg, option_name_end};
        
        auto option = option_by_name<Encoding0>(option_name);
        
        if(!option) return;

        if(!option->has_arg())
            get<parser_without_arg>(option->parser()) ();
        else {
            if(!has_eq_sign) throw runtime_error{
                "option '"
                +option_name.template to_string<enc::utf8>().template to_string<char>()
                +"' must have an argument"
            };
            auto option_arg_beg = arg.begin()+eq_sign_pos+1; // skip '='
            string_view<Encoding0> option_arg{option_arg_beg, arg.end()};
            if(option_arg.empty())
                throw runtime_error {
                    "argument length for option '"
                    +option_name.template to_string<enc::utf8>().template to_string<char>()
                    +"' is zero"
                };
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