#include "../../include/hte_common.h"
#include "../testing.h"


std::vector<bool> TestBordering()
{
    std::vector<bool> testResults;

    // TODO: Add real block data test cases.
    hte::Polygon<long long> a =
        {{{0, 0},
          {0, 1},
          {1, 0}}};
    hte::Polygon<long long> b =
        {{{0, 0},
          {1, 1},
          {1, 0}}};

    // TEST CASE 1
    std::cout << hte::GetBordering(a, b) << std::endl;
    testResults.push_back(hte::GetBordering(a, b));
    return testResults;
}


int main()
{
    TestRunner geometryTestRunner({{"TestBordering", TestBordering}});
    geometryTestRunner.runTests();
}
