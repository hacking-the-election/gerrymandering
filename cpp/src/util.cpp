/*=======================================
 util.cpp:                      k-vernooy
 last modified:                Sat, Feb 8

 Funciton definitions for simple array,
 string, and other utilities.
========================================*/

#include "../include/util.hpp"
#include <regex> // for is_number()
#include <random>

using namespace std;

string readf(string path) {
   
    /*
        read and return file as string
        arguments: path - abs path to the file
    */

    // initiate stream and buffer
    ifstream f(path);
    stringstream buffer;

    // add the file to the buffer
    buffer << f.rdbuf();

    // return buffer string
    return buffer.str();
};

void writef(string contents, string path) {
    /*
        Write a string to a file. This method appends
        to the file rather than overwriting contents just in case.
    */

    ofstream of(path);
    of << contents;
    of.close();
}

vector<string> split(string str, string del) {
    
    /*
        split a string into a vector of strings by a delimiter
        arguments: 
            str - string to be split
            del - the delimiter by which to split
    */

    vector<string> array; // to be returned

    int pos = 0;
    string token;

    while ((pos = str.find(del)) != string::npos) {
        // loop through string, remove parts as it iterates
        token = str.substr(0, pos);
        array.push_back(token);
        str.erase(0, pos + del.length());
    }

    array.push_back(str); // add what remains of the string
    return array;
}


string join(vector<string> str, string del) {

    /*
        join a vector of strings into a string by a delimiter
        arguments: 
            str - vector of strings to be joined
            del - the delimiter by which to join
    */

    string ret;

    for (int i = 0; i < str.size() - 1; i++)
        ret += str[i] + del; // add string with delimiter

    ret += str[str.size() - 1]; // add last string
    return ret;
}



string join(vector<int> str, string del) {
    string ret;

    for (int i = 0; i < str.size() - 1; i++)
        ret += to_string(str[i]) + del; // add string with delimiter

    ret += to_string(str[str.size() - 1]); // add last string
    return ret;
}


bool is_number(string token) {

    // checks if a string is any number type

    return regex_match(token, regex( ( "((\\+|-)?[[:digit:]]+)(\\.(([[:digit:]]+)?))?" ) ));
}

int rand_num(int start, int end) {
    /*
        @def: gets random number from `start` to `end`
        @params: `start`, `end`: range of numbers to generate
        @return: `int` random number
    */

    std::random_device rd; 
    std::mt19937 eng(rd()); 
    uniform_int_distribution<> distr(start, end);
    return distr(eng) - 1;
}