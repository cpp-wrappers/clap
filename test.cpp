#include <vector>
#include <string>
#include <iostream>
#include "include/clap/posix_clap.hpp"
//#include "./include/clap/braced_clap.hpp"
//#include "./include/clap/gnu_clap.hpp"
#include <codecvt>
#include <cxx_util/encoding.hpp>

//using namespace std;
using namespace std::literals;

void flag(std::vector<std::string> args0) {
    std::vector<util::mb::ascii_string> args;

    for(auto a : args0) args.push_back(a);

    std::map<char, bool> flags{{'a', false}, {'b', false}, {'c', false}};

    posix::basic_clap<util::ascii_encoding> parser;

    for(auto& name_to_value : flags)
        parser.flag(name_to_value.first, name_to_value.second);
	
    parser.parse(args);
    
    for(auto& name_to_value : flags) 
        std::cout << name_to_value.first << ": " << name_to_value.second << "\n";

    //cout << 
}

/*void echo(vector<string> args) {
    std::string echo;
    gnu::clap{}
        .value('e', "echo", echo)
        .parse(args);

    cout << echo << "\n";
}

void braced(vector<string> args) {
    bool bool_val = false;
    std::string h00 = "null";
    std::string h01 = "null";
    std::string h1 = "null";
    clap::braced_clap{}
        .braced(
            "main",
            {
                { "hi", clap::value_parser<char>(h00) }
            }
        )
        .option("bool_val", clap::value_parser<char>(bool_val))
        .parse(args.begin(), args.end());

    cout << h00 << "\n";
    cout << h01 << "\n";
    cout << h1 << "\n";
    cout << bool_val << "\n";
}*/

int main() {
    flag({"-a", "-b"});
    //echo({"--echo=hello"});
    //echo({"--echo=hello"});braced({"--echo=hello"});
}