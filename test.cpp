#include <cxx_util/encoding/encoding.hpp>
#include <cxx_util/mb/string.hpp>
#include <type_traits>
#include <vector>
#include <string>
#include <iostream>
#include "include/clap/posix_clap.hpp"
#include "include/clap/gnu_clap.hpp"
#include "include/clap/braced_clap.hpp"
//#include <codecvt>
//#include <cxx_util/encoding/encoding.hpp>

void flag(std::vector<mb::ascii_string> args) {
    std::map<char, bool> flags{{'a', false}, {'b', false}, {'c', false}};

    posix::basic_clap<enc::ascii> parser;

    for(auto& name_to_value : flags)
        parser.flag(name_to_value.first, name_to_value.second);
	
    parser.parse<enc::ascii>(args.begin(), args.end());
    
    for(auto& name_to_value : flags) 
        std::cout << name_to_value.first << ": " << name_to_value.second << std::endl;
}

void echo(std::vector<mb::utf8_string> args) {
    mb::utf8_string echo = u8"not found";

    gnu::basic_clap<enc::utf8>{}
        .value('e', u8"echo", echo)
        .parse<enc::utf8>(args.begin(), args.end());

    std::cout << echo.template to_string<char>() << std::endl;
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
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::cout << "posix: " << std::endl;
    flag({"-a", "-b"});
    std::cout << "gnu: " << std::endl;
    echo({u8"--echo=Приветики"});
    std::cout << "braced: " << std::endl;
    braced({"main{", "hi=hello_from_hi", "}", "bool_val=1"});
}