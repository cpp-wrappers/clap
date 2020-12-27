#include <cxx_util/multibyte_string.hpp>
#include <type_traits>
#include <vector>
#include <string>
#include <iostream>
#include "include/clap/posix_clap.hpp"
#include "include/clap/braced_clap.hpp"
#include "include/clap/gnu_clap.hpp"
#include <codecvt>
#include <cxx_util/encoding.hpp>

void flag(std::vector<mb::ascii_string> args) {
    std::map<char, bool> flags{{'a', false}, {'b', false}, {'c', false}};

    posix::basic_clap<enc::ascii> parser;

    for(auto& name_to_value : flags)
        parser.flag(name_to_value.first, name_to_value.second);
	
    parser.parse(args);
    
    for(auto& name_to_value : flags) 
        std::cout << name_to_value.first << ": " << name_to_value.second << std::endl;
}

void echo(std::vector<mb::ascii_string> args) {
    std::string echo;
    gnu::basic_clap<enc::ascii>{}
        .value('e', "echo", echo)
        .parse<enc::ascii>(args);
    std::cout << echo << std::endl;
}

void braced(std::vector<mb::ascii_string> args) {
    bool bool_val = false;
    std::string h00 = "null";
    std::string h01 = "null";
    std::string h1 = "null";
    clap::basic_braced_clap<enc::ascii>{}
        .braced(
            "main",
            {
                { "hi", clap::value_parser<enc::ascii>(h00) }
            }
        )
        .option("bool_val", clap::value_parser<enc::ascii>(bool_val))
        .parse<enc::ascii>(args.begin(), args.end());

    std::cout << h00 << "\n";
    std::cout << h01 << "\n";
    std::cout << h1 << "\n";
    std::cout << bool_val << "\n";
}

#ifdef _WIN32
#include "windows.h"
#endif

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(1201);
#endif
    std::cout << "posix: " << std::endl;
    flag({"-a", "-b"});
    std::cout << "gnu: " << std::endl;
    echo({"--echo=Приветики"});
    std::cout << "braced: " << std::endl;
    braced({"main{", "hi=hello_from_hi", "}", "bool_val=1"});
}