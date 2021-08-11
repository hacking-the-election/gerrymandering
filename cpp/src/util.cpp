#include <iostream>
#include <regex>
#include "../include/hte_common.h"


namespace hte {

    // std::string GetProgressBar(double progress) {
    //     int length = 30;
    //     std::string bar = "[";
    //     int num = ceil(progress * (double) length);
    //     for (int i = 0; i < num && i < length; i++) bar += "\e[32m=\e[0m";
    //     for (int i = num; i < length; i++) bar += " ";
    //     return bar + "]";
    // }

    // std::string ReadFile(std::string path) {
    //     // initiate stream and buffer
    //     std::ifstream f(path);
    //     std::stringstream buffer;
    //     // add the file to the buffer
    //     buffer << f.rdbuf();
    //     // return buffer string
    //     return buffer.str();
    // };


    // void WriteFile(std::string contents, std::string path) {
    //     std::ofstream of(path);
    //     of << contents;
    //     of.close();
    // }


    // std::vector<std::string> Split(std::string str, std::string del) {
    //     std::vector<std::string> array; // to be returned
    //     int pos = 0;
    //     std::string token;

    //     while ((pos = str.find(del)) != std::string::npos) {
    //         // loop through string, remove parts as it iterates
    //         token = str.substr(0, pos);
    //         array.push_back(token);
    //         str.erase(0, pos + del.length());
    //     }

    //     array.push_back(str); // add what remains of the string
    //     return array;
    // }


    // std::string Join(std::vector<std::string> str, std::string del) {
    //     if (str.size() == 0)
    //         return "";
            
    //     std::string ret = "";
    //     for (int i = 0; i < str.size() - 1; i++)
    //         ret += str[i] + del;

    //     ret += str[str.size() - 1];
    //     return ret;
    // }


    // bool IsNumber(std::string token) {
    //     // checks if a string is any number type
    //     return regex_match(token, std::regex(("((\\+|-)?[[:digit:]]+)(\\.(([[:digit:]]+)?))?")));
    // }


    // int RandInt(int start, int end) {
    //     if (end == 0) return 0;
    //     return (rand() % end + start);
    // }


    // double RandUnitInterval() {
    //     return (static_cast<double>(RandInt(0, 100000)) / 100000.0);
    // }


    // double GetStdev(std::vector<int>& data) {
    //     int sum = std::accumulate(data.begin(), data.end(), 0.0);
    //     double mean = sum / (double)data.size();
    //     std::vector<double> diff(data.size());
    //     std::transform(data.begin(), data.end(), diff.begin(), [mean](int x) { return x - mean; });
    //     double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    //     return (std::sqrt(sq_sum / (double)data.size()));
    // }


    // double GetStdev(std::vector<double>& data) {
    //     double sum = std::accumulate(data.begin(), data.end(), 0.0);
    //     double mean = sum / (double)data.size();
    //     std::vector<double> diff(data.size());
    //     std::transform(data.begin(), data.end(), diff.begin(), [mean](double x) { return x - mean; });
    //     double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    //     return (std::sqrt(sq_sum / (double)data.size()));
    // }
}
