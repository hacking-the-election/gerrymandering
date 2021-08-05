#include "../../include/hte_common.h"
#include "../testing.h"


std::vector<bool> TestBordering()
{
    std::vector<bool> testResults;

    // TODO: Add real block data test cases.
    hte::Polygon<hte::ClipperCoord> a =
        {{{0, 0},
          {0, 1},
          {1, 0}}};
    hte::Polygon<hte::ClipperCoord> b =
        {{{0, 0},
          {1, 1},
          {1, 0}}};

    hte::Polygon<hte::ClipperCoord> c =
        {{{1, 0},
          {2, 1},
          {2, 0}}};

    // TEST CASE 1
    testResults.push_back(hte::GetBordering(a, b));

    // TEST CASE 2
    testResults.push_back(!hte::GetBordering(a, b));

    return testResults;
}


int main()
{
    TestRunner geometryTestRunner({{"TestBordering", TestBordering}});
    geometryTestRunner.runTests();
}
