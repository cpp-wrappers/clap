#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "../include/clap/posix_clap.hpp"

using namespace std;

void exec(vector<string> args) {
    map<char, bool> flags{{'a', false}, {'b', false}, {'c', false}};

    posix::clap parser;

    for(auto& name_to_value : flags)
        parser.flag(name_to_value.first, name_to_value.second);
	

    parser.parse(args.begin(), args.end());
    
    for(auto& name_to_value : flags) 
        cout << name_to_value.first << ": " << name_to_value.second << "\n";
}
