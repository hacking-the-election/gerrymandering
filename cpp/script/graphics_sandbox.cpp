#include <iostream>
#include <unistd.h>


#include "../include/hte_common.h"


using namespace std;
using namespace hte;
using namespace hte::gl;


int main(int argc, char* argv[]) {
    Canvas* ctx = Canvas::GetInstance();
    
    std::vector<Polygon<double>> polygons = {{
        {
            {{0, 0}, {10, 0}, {10, 10}, {0, 10}},
            {{1, 1}, {5, 1}, {5, 5}, {1, 5}},
        }
    }};
    
    PrintVec(polygons);
    std::cout << endl;

    for (const auto& p : polygons)
    {
        ctx->pushGeometry(p, Style{1, false, true});
    }


    double lastTime = glfwGetTime();
    int nbFrames = 0;



    while (!glfwWindowShouldClose(ctx->getWindow()))
    {
        double currentTime = glfwGetTime();
        nbFrames++;
        if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1 sec ago
            // printf and reset timer
            printf("%f ms/frame\n", 1000.0/double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }
        ctx->renderCurrent();
    }

    return 0;
}
