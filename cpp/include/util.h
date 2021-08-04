#ifndef _HTE_UTIL_H
#define _HTE_UTIL_H

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

template<typename T>
double GetStdev(std::vector<T>& data);

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


#endif