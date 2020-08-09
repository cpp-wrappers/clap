#include <string>
#include <map>
#include <functional>

namespace posix {

template<class CharT>
struct basic_clap {
    using str_t = std::basic_string<CharT>;
    using strv_t = std::basic_string_view<CharT>;
	
	class option_t {
		std::function<void(strv_t)> m_parser;
		bool m_has_arg;
	public:
		bool has_arg() { return m_has_arg; }
		auto& parser() { return m_parser; }

		option_t& parser(std::function<void(strv_t)> p) {
			m_parser = p;
			m_has_arg = true;
			return *this;
		}

		option_t& parser(std::function<void(void)> p) {
			m_parser = [=](strv_t){ p(); };
			m_has_arg = false;
			return *this;
		}
	};


    std::map<CharT, option_t> options;
    
public:	
	
    option_t& option(CharT name) {
        auto[iter, succes] = options.emplace(name, option_t{});
        return iter->second;
    }

	auto& flag(option_t& option, bool& val) {
		option.parser([&val]() { val = true; });
		return *this;
	}

	auto& flag(CharT name, bool& val) { return flag(option(name), val); }

    template<std::input_iterator It>
    void parse(
        const It begin,
        const It end,
        std::function<void(const It, It&, const It)> operand_parser = {}
    ) {
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
    option_t* option_by_name(CharT name) {
        auto name_to_option = options.find(name);
        return name_to_option == options.end() ? nullptr : &(name_to_option->second);
    }

    template<class It>
    void parse_operand(
        const It begin,
        It& arg,
        const It end,
        std::function<void(const It, It&, const It)> operand_parser
    ) {
        It prev = arg;
		if(operand_parser)
            operand_parser(begin, arg, end);
        if(prev == arg)
            throw std::runtime_error("operands aren't parsed: "+str_t{*arg});
    }

    template<class It>
    void parse_one_hyphen_arg(const It begin, It& arg_it, const It end) {
        strv_t arg{*arg_it};
        
        for(
            auto first_ch = arg.begin()+1, ch = first_ch;
            *ch;
            ch++
        ) {
            CharT name = *ch;
            auto option = option_by_name(name);
            if(!option) {
                if(ch == first_ch) // check if not suboption
                    return; // assuming that's operand
                else throw std::runtime_error("undefined option: "+str_t{1, name});
            }

            if(!option->has_arg()) {
                option->parser()({});
                continue;
            }

            if(++ch == arg.end()) {
                if(++arg_it==end) throw std::runtime_error("argument is required for option '"+str_t{1, name}+"'");
                option->parser()(*arg_it);
            }
            else option->parser()(strv_t{ch, arg.end()});
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
