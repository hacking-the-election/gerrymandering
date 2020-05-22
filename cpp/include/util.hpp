/*=======================================
 shape.hpp:                     k-vernooy
 last modified:               Wed, Feb 19
 
 Declarations for utility functions -
 vector modification, reading/writing 
 files, assertions.
========================================*/

#pragma once
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <numeric>
#include <cmath>

// file manipulation
std::string readf(std::string path);
void writef(std::string contents, std::string path);

// vector manipulation
std::vector<std::string> split(std::string str, std::string del);
std::string join(std::vector<std::string> str, std::string del);

// assertions (type equality)
bool is_number(std::string token);
int rand_num(int start, int end);

double get_stdev(std::vector<int>& data);
double get_stdev(std::vector<double>& data);
