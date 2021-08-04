#include <exception>
#include <string>
#include <vector>

#include "../lib/Clipper/cpp/clipper.hpp"

// class Exceptions {
//     struct LinearRingOpen : public std::exception {
//         const char* what() const throw() {
//             return "Points LinearRing do not form closed ring";
//         }
//     };
// };

namespace hte { typedef signed long long Coord; }

bool IsNumber(std::string token);
void WriteFile(std::string contents, std::string path);
std::string ReadFile(std::string path);

int RandInt(int start, int end);
double RandUnitInterval();

template<class T>
double GetStdev(std::vector<T>& data);

void PrintPaths(const ClipperLib::Paths& paths);
