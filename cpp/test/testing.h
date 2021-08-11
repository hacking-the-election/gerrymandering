#ifndef HTE_TESTING_H
#define HTE_TESTING_H

#include <map>
#include <string>
#include <vector>
#include <functional>


/**
 * Runs unit tests.
 *
 * @param testFuncs a map of test names to function pointers of the tests.
 *                  The functions must take no arguments and return a vector of
 *                  bools, where each bool represents a failed or passed assertion.
 */
class TestRunner
{
using tfMap = std::map<std::string, std::function<std::vector<bool>()>>;

public:
    TestRunner() {}
    TestRunner(tfMap testFuncs) : testFuncs(testFuncs) {}

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
    tfMap testFuncs;
};

#endif
