/*=======================================
 sandbox.cpp:                   k-vernooy
 last modified:                Fri, Jun 19
 
 A simple testing environment with the
 hacking-the-election library
========================================*/

#include <chrono>
#include <iostream>
#include <boost/filesystem.hpp>

#include "../include/hte.h"
#include "../lib/rapidjson/include/rapidjson/document.h"
#include "../lib/rapidjson/include/rapidjson/writer.h"
#include "../lib/rapidjson/include/rapidjson/stringbuffer.h"

using namespace boost::filesystem;
using namespace rapidjson;
using namespace std;
using namespace hte::Geometry;
using namespace hte::Graphics;
using namespace hte::Algorithm;
using namespace hte::Util;
using namespace hte::Data;


/**
    A loaded environment with included namespaces ready for 
    graphics programs testing, compiling random binaries, and other scripts
*/
int main(int argc, char* argv[]) {
    srand(time(NULL));
    cout << "hello world" << endl;
    return 0;
}
