if ! clang++ -Wall -g -std=c++20 -stdlib=libc++ -o t test.cpp
then exit 1
fi
if [[ $1 == "gdb" ]]
then gdb t
else ./t
fi
rm t