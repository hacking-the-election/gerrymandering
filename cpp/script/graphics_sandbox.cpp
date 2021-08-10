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
        },
        {
            {{26, 15}, {31, 15}, {20, 20}, {15, 20}},
        },
        {
            {{29, 5}, {37, 11}, {20, 8}, {17, 2}},
            {{28, 6}, {31, 8}, {22, 7}, {21, 4}}
        }
    }};
    
    PrintVec(polygons);
    std::cout << endl;

    // for (const auto& p : polygons)
    // {
    ctx->pushGeometry(polygons[0], Style{1, true, {1.0, 0.0, 0.0}, true, {0.0, 1.0, 0.0}});
    ctx->pushGeometry(polygons[1], Style{1, true, {0.0, 1.0, 0.0}, true, {0.0, 0.0, 1.0}});
    ctx->pushGeometry(polygons[2], Style{1, true, {0.0, 0.0, 1.0}, true, {1.0, 0.0, 0.0}});
    // }


    double lastTime = glfwGetTime();
    int nbFrames = 0;



    while (!glfwWindowShouldClose(ctx->getWindow()))
    {
        double currentTime = glfwGetTime();
        nbFrames++;
        if ( currentTime - lastTime >= 1.0 )
        {
            printf("%f ms/frame\n", 1000.0/double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }
        ctx->renderCurrent();
    }

    return 0;
}
