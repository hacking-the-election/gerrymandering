#ifndef _HTE_UTIL_H
#define _HTE_UTIL_H

#include <iostream>
#include <exception>
#include <vector>

#include "../lib/Clipper/cpp/clipper.hpp"

// class Exceptions {
//     struct LinearRingOpen : public std::exception {
//         const char* what() const throw() {
//             return "Points LinearRing do not form closed ring";
//         }
//     };
// };
namespace hte
{

// Forward declarations.
template<typename T>
struct Point2d;

template<typename T>
class LinearRing;

bool IsNumber(std::string token);
void WriteFile(std::string contents, std::string path);
std::string ReadFile(std::string path);

int RandInt(int start, int end);
double RandUnitInterval();

template<class T>
double GetStdev(std::vector<T>& data);

template<typename T>
void PrintLinearRing(const LinearRing<T>& ring)
{
    for (const Point2d<T>& point : ring)
        std::cout << "(" << point.x << ", " << point.y << ") ";
    std::cout << std::endl;
}

void PrintPath(const ClipperLib::Path& path_);
void PrintPaths(const ClipperLib::Paths& paths);

}

#endif
