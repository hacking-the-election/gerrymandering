#include <sstream>
#include <string>
#include <fstream>
#include <vector>

using namespace std;

string readf(string path);
void writef(string contents, string path);

vector<string> split(string str, string del);
string join(vector<string> str, string del);
string join(vector<int> str, string del);

bool is_number(string token);