#include <string>

using String = std::string;

const String RED = "\e[31m";
const String GREEN = "\e[32m";
const String YELLOW = "\e[33m";
const String BLUE = "\e[34m";
const String PURPLE = "\e[35m";
const String RESET = "\e[0m";

const String C = ",";
const String N = "\n";
const String OQ = "\"" + GREEN;
const String CQ = RESET + "\"";
const String CQC = RESET + "\": ";
const String T = "    ";

String TAB(int num) {
    string str;
    for ( int i = 0; i < num; i++ ) {
        str += T;
    }
    return str;
}