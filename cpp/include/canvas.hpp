#include "shape.hpp"
#include <SDL2/SDL.h>

namespace GeoDraw {
    class Canvas {
        GeoGerry::bounding_box box;
        GeoGerry::coordinate_set shape = connect_dots(shapes);
        int x, y;
    
        Uint32 * pixels = pix_array;

        void add_shape(GeoGerry::Shape s);
        void add_shape(GeoGerry::Multi_Shape s);
        void resize(int x, int y);
        void draw();
    };

    class Anim {
        std::vector<Canvas> frames;
        int delay;

        void playback();
    };
}