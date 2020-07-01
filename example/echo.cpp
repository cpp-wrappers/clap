#include "../include/clap.hpp"
#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace clap;

void exec(vector<string> args) {
    gnu_clap parser;
    parser.option('e', "echo", [](string_view v) {
        cout << v << "\n";
    });
    parser.parse(args.begin(), args.end());
}