#ifndef _HTE_UTIL_H
#define _HTE_UTIL_H


#include <iostream>
#include <exception>
#include <vector>

#include "../lib/Clipper/cpp/clipper.hpp"


namespace hte
{

// class Exceptions {
//     struct LinearRingOpen : public std::exception {
//         const char* what() const throw() {
//             return "Points LinearRing do not form closed ring";
//         }
//     };
// };

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

template<typename T>
double GetStdev(std::vector<T>& data);


// TODO: I think ostream operator overloading would be better suited for the following

template<typename T>
void PrintVec(const T& el)
{
    std::cout << el;
};


template<typename T>
void PrintVec(const hte::LinearRing<T>& vec)
{
    std::cout << "{";
    for (size_t i = 0; i < vec.size() - 1; i++)
    {
        PrintVec(vec[i]);
        std::cout << ", ";
    }

    PrintVec(vec.back());
    std::cout << "}";
};


template<typename T>
void PrintVec(const hte::Polygon<T>& vec)
{
    std::cout << "{";
    for (size_t i = 0; i < vec.size() - 1; i++)
    {
        PrintVec(vec[i]);
        std::cout << ", ";
    }

    PrintVec(vec.back());
    std::cout << "}";
};


template<typename T>
void PrintVec(const std::vector<T>& vec)
{
    std::cout << "{";
    for (size_t i = 0; i < vec.size() - 1; i++)
    {
        PrintVec(vec[i]);
        std::cout << ", ";
    }

    PrintVec(vec.back());
    std::cout << "}";
    std::cout.flush();
};

}

#endif
