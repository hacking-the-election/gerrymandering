/*=======================================
 shape.hpp:                     k-vernooy
 last modified:               Fri, Jun 19
 
 Declarations for utility functions -
 vector modification, reading/writing 
 files, assertions.
========================================*/

#ifndef UTIL_H
#define UTIL_H

#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <numeric>
#include <cmath>

namespace hte {
    namespace Util {

        /**
         * Custom exceptions for geometric or algorithmic
         * errors and recursion breakouts for graphics.
        */
        class Exceptions {
            public:
                struct PrecinctNotInGroup : public std::exception {
                    const char* what() const throw() {
                        return "No precinct in this precinct group matches the provided argument";
                    }
                };

                struct LinearRingOpen : public std::exception {
                    const char* what() const throw() {
                        return "Points LinearRing do not form closed ring";
                    }
                };
        };
        

        // useful type assertions and manipulations,
        // file I/O, and random number generation

        bool                      IsNumber(std::string token);
        void                      WriteFile(std::string contents, std::string path);
        std::string               ReadFile(std::string path);
        std::string               Join(std::vector<std::string> str, std::string del);
        std::string               GetProgressBar(double progress);
        std::vector<std::string>  Split(std::string str, std::string del);

        int RandInt(int start, int end);
        double RandUnitInterval();

        double GetStdev(std::vector<int>& data);
        double GetStdev(std::vector<double>& data);
    }
}

#endif
