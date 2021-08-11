#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

#include "testing.h"


std::vector<bool> TestRunner::runTest(std::string testName) const
{
    std::cout << testName << ": ";
    std::vector<bool> results = testFuncs.at(testName)();
    
    for (bool passed : results)
        std::cout << (passed) ? "PASS " : "FAIL ";

    std::cout << std::endl;
    return results;
}


std::vector<std::vector<bool>> TestRunner::runTests() const
{
    std::vector<std::vector<bool>> testResults;
    
    for (const auto& [testName, testFunc] : testFuncs)
        testResults.push_back(runTest(testName));

    return testResults;
}
