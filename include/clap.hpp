#pragma once

#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <map>
#include <stdexcept>

namespace clap {

using handler_t = std::function<void(std::string_view)>;

struct posix_clap {
    struct option_t {
        char name;
        bool has_argument;
        handler_t parser;
    };

    std::map<char, option_t> name_to_option;

    void flag(char name, bool& ref) {
        name_to_option.emplace(name, option_t{name, false, [&](std::string_view) {
            ref = true;
        }});
    }

    void option(char name, handler_t handler) {
        name_to_option.emplace(name, option_t{name, true, handler});
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
            option_t& op = name_to_option[ch];
            if(op.has_argument) {
                if(std::distance(b->begin()+i+1, b->end()) == 0) {
                    b++;
                    op.parser(std::string_view{b->begin(), b->end()});
                }
                else op.parser(std::string_view{b->begin()+i+1, b->end()});
                break;
            }
            else op.parser(std::string_view{});
        }
    }
};

struct gnu_clap : posix_clap {
    struct long_option_t {
        std::string name;
        bool has_argument;
        handler_t parser;
    };
    
    std::map<std::string, long_option_t> long_name_to_option;

    void option(char name, std::string long_name, handler_t handler) {
        posix_clap::option(name, handler);
        option(long_name, handler);
    }

    void option(std::string long_name, handler_t handler) {
        long_name_to_option.emplace(long_name, long_option_t{long_name, true, handler});
    }

    template<class Iter>
    void parse(Iter begin, Iter end) {
        for(auto arg = begin; arg != end; arg++) {
            char first_char = arg->at(0);
            if(first_char != '-') throw std::runtime_error("undefined argument: " + *arg);
            char second_char = arg->at(1);
            if(second_char != '-') posix_clap::parse_option(arg, end);
            else parse_option(arg, end);
        }
    }

    protected:
    template<class Iter>
    void parse_option(Iter& arg, Iter& e) {
        auto eq_sign_pos = arg->find_first_of('=', 2);
        bool has_eq_sign = eq_sign_pos != std::string::npos;
        auto option_end = has_eq_sign ? arg->begin()+eq_sign_pos : arg->end();
        std::string option{arg->begin()+2, option_end};
        long_option_t& op = long_name_to_option.at(option);
        if(op.has_argument) {
            bool has_eq_sign = option_end != arg->end() && *option_end == '=';
            if(!has_eq_sign) throw std::runtime_error("option \'"+option+"\' must have an argument");
            auto arg_begin = option_end+1;
            if(std::distance(arg_begin, arg->end()) == 0)
                throw std::runtime_error("argument length for option \'" + option + "\' is zero");
            op.parser(std::string_view{arg_begin, arg->end()});
        }  
        else op.parser(std::string_view{});    
    }
};

}