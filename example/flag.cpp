#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "../include/clap/posix_clap.hpp"

using namespace std;

void exec(vector<string> args) {
    map<char, bool> flags{{'a', false}, {'b', false}, {'c', false}};

    clap::basic_posix_clap<char> parser;

    for_each(flags.begin(), flags.end(), [&](auto& p){
        parser.flag(p.first, p.second);
    });

    parser.parse(args.begin(), args.end());
    
    for_each(flags.begin(), flags.end(), [&](auto& p){
        cout << p.first << ": " << p.second << "\n";
    });
}