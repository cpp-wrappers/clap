#include <vector>
#include <string>
#include <iostream>
#include "include/clap/posix_clap.hpp"
#include "include/clap/braced_clap.hpp"
#include "include/clap/gnu_clap.hpp"
#include <codecvt>
#include <cxx_util/encoding.hpp>

//using namespace std;
using namespace std::literals;

void flag(std::vector<std::string> args0) {
    std::vector<util::mb::ascii_string> args;

    for(auto a : args0) args.push_back(a);

    std::map<char, bool> flags{{'a', false}, {'b', false}, {'c', false}};

    posix::basic_clap<util::enc::ascii> parser;

    for(auto& name_to_value : flags)
        parser.flag(name_to_value.first, name_to_value.second);
	
    parser.parse(args);
    
    for(auto& name_to_value : flags) 
        std::cout << name_to_value.first << ": " << name_to_value.second << std::endl;
}

void echo(std::vector<std::string> args0) {
    std::vector<util::mb::ascii_string> args;

    for(auto a : args0) args.push_back(a);

    std::string echo;
    gnu::basic_clap<util::enc::ascii>{}
        .value('e', "echo", echo)
        .parse<util::enc::ascii>(args);

    std::cout << echo << std::endl;
}

void braced(std::vector<std::string> args) {
    bool bool_val = false;
    std::string h00 = "null";
    std::string h01 = "null";
    std::string h1 = "null";
    clap::basic_braced_clap<util::enc::ascii>{}
        .braced(
            "main",
            {
                { "hi", clap::value_parser<util::enc::ascii>(h00) }
            }
        )
        .option("bool_val", clap::value_parser<util::enc::ascii>(bool_val))
        .parse<util::enc::ascii>(args.begin(), args.end());

    std::cout << h00 << "\n";
    std::cout << h01 << "\n";
    std::cout << h1 << "\n";
    std::cout << bool_val << "\n";
}

int main() {
    std::cout << "posix: " << std::endl;
    flag({"-a", "-b"});
    std::cout << "gnu: " << std::endl;
    echo({"--echo=hello"});
    std::cout << "braced: " << std::endl;
    braced({"main{", "hi=hello_from_hi", "}", "bool_val=1"});
}