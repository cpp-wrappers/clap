#pragma once

#include <bits/iterator_concepts.h>
#include <cwchar>
#include <cxx_util/encoding.hpp>
#include <cxx_util/string.hpp>
#include <locale>
#include <stdexcept>
#include <string>
#include <map>
#include <functional>
#include <iterator>
#include <ranges>
#include <sstream>
#include <string_view>
#include <variant>
#include <wchar.h>
#include "parser.hpp"
#include "cxx_util/multibyte_string.hpp"

namespace clap {

namespace posix {

template<class Encoding>
struct basic_clap {
    template<class Encoding0>
    using character = mb::character<Encoding0>;
    template<class Encoding0>
    using character_view = mb::character_view<Encoding0>;

    template<class Encoding0>
    using string = mb::basic_string<Encoding0>;
    template<class Encoding0>
    using string_view = mb::basic_string_view<Encoding0>;

    template<class It>
    using operands_parser_t = std::function<void(const It, It&, const It)>;
	
	class option_t {
		const std::variant<clap::parser_with_arg<Encoding>, parser_without_arg> m_parser;
	public:
		bool has_arg() const {
            return std::holds_alternative<parser_with_arg<Encoding>>(m_parser);
        }

		auto& parser() const { return m_parser; }

		option_t(parser_with_arg<Encoding> p) : m_parser{p} {}

		option_t(parser_without_arg p) : m_parser{p} {}
	};

protected:
    std::map<character<Encoding>, option_t> options;
public:
    auto& option(const character<Encoding>& name, auto parser) {;
        options.emplace(std::move(name), option_t{parser});
        return *this;
    }
	
	auto& flag(auto name, bool& val) { return option(name, clap::flag_parser(val)); }
	auto& value(auto name, auto& val) { return option(name, clap::value_parser<Encoding>(val)); }

    template<
        std::input_iterator It
    >
    requires mb::string<std::iter_value_t<It>>
    void parse(
        const It begin,
        const It end,
        operands_parser_t<It> operand_parser = {}
    ) const {
        parse< It, typename std::iter_value_t<It>::encoding_type >(begin, end);
    } 

	template<std::input_iterator It, class Encoding0>
    void parse(
        const It begin,
        const It end,
        operands_parser_t<It> operand_parser = {}
    ) const {
        It arg = begin;
        while(arg != end) {
            auto& str {*arg};

            auto first_char = str[0];
            auto second_char = str[1];

            if(first_char != '-') break;

            if(second_char != '-') {
                It prev = arg;
                parse_one_hyphen_arg<Encoding0>(begin, arg, end);
                if(prev == arg)
                    break;
            }
            else if(str.size() < 2) {
                arg++; // skip '--', point to operands beg
                break;
            }
        }

        while(arg != end)
            parse_operand(begin, arg, end, operand_parser);
	}

    template<std::ranges::range R, class It = std::ranges::iterator_t<R>>
    void parse(R& range, operands_parser_t<It> operand_parser = {}) const {
        parse<std::ranges::iterator_t<R>>(range.begin(), range.end(), operand_parser);
    }

protected:
    template<class Encoding0>
    const option_t* option_by_name(character_view<Encoding0> name) const {
        auto converted = name.template convert<Encoding>();
        auto name_to_option = options.find(converted);
        return name_to_option == options.end() ? nullptr : &(name_to_option->second);
    }

    template<class It>
    void parse_operand(
        const It begin,
        It& arg,
        const It end,
        operands_parser_t<It> operand_parser
    ) const {
        It prev = arg;
		if(operand_parser) {
            operand_parser(begin, arg, end);
        }

        if(prev == arg)
            throw std::runtime_error{"operands aren't parsed: "+((*arg).template to_string<enc::ascii>()) };
    }

    template<class Encoding0, class It>
    void parse_one_hyphen_arg(const It begin, It& arg_it, const It end) const {
        using namespace std;
        using namespace std::literals;
        string_view<Encoding0> arg{*arg_it};

        for(
            auto first_ch = ++arg.begin(), ch = first_ch;
            ch != arg.end();
            ++ch
        ) {
            auto name = *ch;
            const option_t* option = option_by_name<Encoding0>(name);

            if(!option) {
                if(ch == first_ch) // check if not suboption
                    return; // assuming that's operand
                else throw runtime_error{
                    "undefined option: '"+name.template to_string<enc::ascii>()+"'"
                };
            }

            if(not option->has_arg()) {
                get<parser_without_arg>(option->parser()) ();
                continue;
            }

            auto parser = get<parser_with_arg<Encoding>>(option->parser());

            if(++ch == arg.end()) {
                if(++arg_it == end) throw runtime_error{
                    "argument is required for option '"+name.template to_string<enc::ascii>()+"'"
                };
                parser((*arg_it).template convert<Encoding>());
            }
            else parser(string_view<Encoding0>{ch, arg.end()}.template convert<Encoding>());
            break;
        }

        arg_it++;
    }
};

}

}

namespace posix {
    using namespace clap::posix;
}