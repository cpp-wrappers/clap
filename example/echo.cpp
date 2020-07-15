#include <vector>
#include <string>
#include <iostream>
#include "../include/clap/gnu_clap.hpp"

using namespace std;

void exec(vector<string> args) {
    gnu::clap parser;
    parser.option('e', "echo", [](string_view v) {
        cout << v << "\n";
    });
    parser.parse(args.begin(), args.end());
}