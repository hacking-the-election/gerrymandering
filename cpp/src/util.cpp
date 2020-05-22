/*=======================================
 util.cpp:                      k-vernooy
 last modified:                Sat, Feb 8

 Funciton definitions for simple array,
 string, and other utilities.
========================================*/

#include <regex>
#include <random>
#include <iostream>
#include "../include/util.hpp"

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


std::string join(std::vector<std::string> str, std::string del) {
    if (str.size() == 0)
        return "";
        
    string ret = "";
    for (int i = 0; i < str.size() - 1; i++)
        ret += str[i] + del;

    ret += str[str.size() - 1];
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

    // std::cout << end << ", " << start << std::endl;
    if (end == 0) return 0;
    return (rand() % end + start);
    // std::random_device rd; 
    // std::mt19937 eng(rd()); 
    // uniform_int_distribution<int> distr(start, end);
    // return distr(eng);
}

double get_stdev(std::vector<int>& data) {
    /*
        Determine standard deviation of a list of numbers.
    */

    int sum = std::accumulate(data.begin(), data.end(), 0.0);
    double mean = sum / (double)data.size();

    std::vector<double> diff(data.size());
    std::transform(data.begin(), data.end(), diff.begin(), [mean](int x) { return x - mean; });
    double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    return (std::sqrt(sq_sum / (double)data.size()));
}


double get_stdev(std::vector<double>& data) {
    /*
        Determine standard deviation of a list of numbers.
    */

    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    double mean = sum / (double)data.size();

    std::vector<double> diff(data.size());
    std::transform(data.begin(), data.end(), diff.begin(), [mean](double x) { return x - mean; });
    double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    return (std::sqrt(sq_sum / (double)data.size()));
}
