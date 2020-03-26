#pragma once
#include <string>

const std::string 

RED = "\e[31m",
GREEN = "\e[32m",
YELLOW = "\e[33m",
BLUE = "\e[34m",
PURPLE = "\e[35m",
RESET = "\e[0m",
C = ",",
N = "\n",
OQ = "\"",
CQ = "\"",
CQC = "\": ",
T = "    ";

std::string TAB(int num) {
    
    std::string str;
    for ( int i = 0; i < num; i++ )
        str += T;
    
    return str;
}