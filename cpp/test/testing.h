#ifndef HTE_TESTING_H
#define HTE_TESTING_H

#include <map>
#include <string>
#include <vector>


/**
 * Runs unit tests.
 *
 * @param testFuncs a map of test names to function pointers of the tests.
 *                  The functions must take no arguments and return a vector of
 *                  bools, where each bool represents a failed or passed assertion.
 */
class TestRunner
{
public:
    TestRunner() {}
    TestRunner(std::map<std::string, std::vector<bool>(*)()> testFuncs) : testFuncs(testFuncs) {}

    /**
     * Runs a single test.
     *
     * @param testName the name of a test.
     */
    std::vector<bool> runTest(std::string testName) const;

    /**
     * Runs all the tests stored as function pointers in the TestRunner object.
     */
    std::vector<std::vector<bool>> runTests() const;
private:
    std::map<std::string, std::vector<bool>(*)()> testFuncs;
};

#endif
