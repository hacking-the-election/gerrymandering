#include "../include/shape.hpp"
#include "../include/geometry.hpp"
#include "../include/canvas.hpp"

using namespace Geometry;
using namespace std;
using namespace Graphics;

int main() {
    
    Anim animation(80);
    for (int i = 4; i < 45; i++) {
        Polygon s = generate_gon({0,0}, 1000, i);
        Canvas canvas(800, 800);
        canvas.add_shape(s);
        animation.frames.push_back(canvas);
    }
 
    animation.playback();

    return 0;
}
