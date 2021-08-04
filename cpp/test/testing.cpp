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
        if (passed) std::cout << "PASS "; else std::cout << "FAIL ";
    std::cout << std::endl;
    return results;
}


std::vector<std::vector<bool>> TestRunner::runTests() const
{
    std::vector<std::vector<bool>> testResults;
    for (const auto& it : testFuncs)
    {
        testResults.push_back(runTest(it.first));
    }
    return testResults;
}
