#include "../include/hte_common.h"
#include <fstream>

using namespace rapidjson;
using namespace hte;


int main(int argc, const char** argv)
{
    // parse all data into state object
    State state;

    // define locations for each expected filetype
    std::unordered_map<FileType, std::string> locations =
    {
        {FileType::BLOCK_GEO, argv[1]},
        {FileType::BLOCK_DEMOGRAPHICS, argv[2]}
    };

    // parse data from locations
    DataParser p(locations);
    p.parseToState(state);
}