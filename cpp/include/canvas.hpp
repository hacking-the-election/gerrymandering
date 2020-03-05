#include "shape.hpp"
#include <SDL2/SDL.h>

namespace GeoDraw {
    class Canvas {
        /*
            A class for storing information about a screen
            of pixels and shapes to be written to an SDL display
        */

        private:
        // contents of the canvas
        std::vector<GeoGerry::LinearRing> outlines;  // shapes to be drawn individually
        std::vector<GeoGerry::LinearRing> holes;     // holes to be drawn individually

        // meta information
        int x, y;                                    // dimensions of the screen
        Uint32 * pixels;                             // the pixel array to write to screen
        GeoGerry::bounding_box box;                  // the outer bounding box

        public:
        // add shape to the canvas
        void add_shape(GeoGerry::Shape s);
        void add_shape(GeoGerry::LinearRing s);
        void add_shape(GeoGerry::Multi_Shape s);
        void add_shape(GeoGerry::Precinct_Group s);
        
        void resize_window(int x, int y);
        void draw();
    };

    class Anim {
        std::vector<Canvas> frames;
        int delay;

        void playback();
    };
}