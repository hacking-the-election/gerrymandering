#include <sstream>
#include <string>
#include <fstream>
#include <vector>

std::string readf(std::string path);
void writef(std::string contents, std::string path);

std::vector<std::string> split(std::string str, std::string del);
std::string join(std::vector<std::string> str, std::string del);
std::string join(std::vector<int> str, std::string del);

bool is_number(std::string token);