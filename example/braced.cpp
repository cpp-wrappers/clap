#include <vector>
#include <string>
#include <iostream>
#include "../include/clap/braced_clap.hpp"

using namespace std;
using namespace literals;

void exec(vector<string> args) {
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
}