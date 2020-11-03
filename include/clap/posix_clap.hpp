#pragma once

#include <string>
#include <map>
#include <functional>
#include <iterator>
#include <ranges>
#include <sstream>
#include <variant>
#include "parser.hpp"
#include "iterator_util.hpp"


namespace clap {

namespace posix {

template<class CharT>
struct basic_clap {
    using string = std::basic_string<CharT>;
    using string_view = std::basic_string_view<CharT>;
	
	class option_t {
		const std::variant<clap::parser_with_arg<CharT>, parser_without_arg> m_parser;
	public:
		bool has_arg() const {
            return std::holds_alternative<parser_with_arg<CharT>>(m_parser);
        }

		auto& parser() const { return m_parser; }

		option_t(parser_with_arg<CharT> p)
		:m_parser{p} {}

		option_t(parser_without_arg p)
		:m_parser{p} {}
	};

protected:
    std::map<CharT, option_t> options;
public:	
	
    auto& option(CharT name, auto parser) {
        options.emplace(name, option_t{parser});
        return *this;
    }
	
	auto& flag(CharT name, bool& val) { return option(name, clap::flag_parser(val)); }
	auto& value(CharT name, auto& val) { return option(name, clap::value_parser<CharT>(val)); }
	
    template<std::ranges::range R, class It = std::ranges::iterator_t<R>>
    void parse(
        R& range,
        std::function<void(const It, It&, const It)> operand_parser = {}
    ) const {
        parse(range.begin(), range.end(), operand_parser);
    }

	template<iterator_value_convertible_to_string_view<CharT> It>
    void parse(
        const It begin,
        const It end,
        std::function<void(const It, It&, const It)> operand_parser = {}
    ) const {
        It arg = begin;
        while(arg != end) {
            auto first_char = (*arg)[0];
            auto second_char = (*arg)[1];
            if(first_char != '-') break;

            if(second_char != '-') {
                It prev = arg;
                parse_one_hyphen_arg(begin, arg, end);
                if(prev == arg)
                    break;
            }
            else if((*arg)[2] == 0){
                arg++; // skip '--', point to operands beg
                break;
            }
        }

        while(arg != end)
            parse_operand(begin, arg, end, operand_parser);
	}

protected:
    const option_t* option_by_name(CharT name) const {
        auto name_to_option = options.find(name);
        return name_to_option == options.end() ? nullptr : &(name_to_option->second);
    }

    template<class It>
    void parse_operand(
        const It begin,
        It& arg,
        const It end,
        std::function<void(const It, It&, const It)> operand_parser
    ) const {
        It prev = arg;
		if(operand_parser)
            operand_parser(begin, arg, end);
        if(prev == arg)
            throw std::runtime_error{"operands aren't parsed: "+string{*arg}};
    }

    template<class It>
    void parse_one_hyphen_arg(const It begin, It& arg_it, const It end) const {
        using namespace std;
        using namespace std::literals;
        string_view arg{*arg_it};
        
        for(
            auto first_ch = arg.begin()+1, ch = first_ch;
            *ch;
            ch++
        ) {
            CharT name = *ch;
            const option_t* option = option_by_name(name);
            if(!option) {
                if(ch == first_ch) // check if not suboption
                    return; // assuming that's operand
                else throw runtime_error{"undefined option: '"s+name+"'"s};
            }

            if(!option->has_arg()) {
                get<parser_without_arg>(option->parser()) ();
                continue;
            }

            parser_with_arg<CharT> parser = get<parser_with_arg<CharT>>(option->parser());

            if(++ch == arg.end()) {
                if(++arg_it==end) throw runtime_error{"argument is required for option '"s+name+"'"s};
                parser(*arg_it);
            }
            else parser(string_view{ch, arg.end()});
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

}

namespace posix {
    using namespace clap::posix;
}