#include <exception>
#include <string>
#include <vector>


// class Exceptions {
//     struct LinearRingOpen : public std::exception {
//         const char* what() const throw() {
//             return "Points LinearRing do not form closed ring";
//         }
//     };
// };

bool IsNumber(std::string token);
void WriteFile(std::string contents, std::string path);
std::string ReadFile(std::string path);

int RandInt(int start, int end);
double RandUnitInterval();

template<class T>
double GetStdev(std::vector<T>& data);