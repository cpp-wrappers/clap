#include <vector>
#include <string>
#include <iostream>
#include "../include/clap/gnu_clap.hpp"

using namespace std;

void exec(vector<string> args) {
    std::string echo;
    gnu::clap{}
        .value('e', "echo", echo)
        .parse(args);

    cout << echo << "\n";
}
