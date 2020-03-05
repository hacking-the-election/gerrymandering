#include "shape.hpp"
#include <SDL2/SDL.h>

namespace GeoDraw {
    
    class Color {
        private:
        int r, g, b;

        public:
        Color(std::string hex);
        Color(int rx, int gx, int bx) : r(rx), g(gx), b(bx) {};
    };

    class Canvas {
        /*
            A class for storing information about a screen
            of pixels and shapes to be written to an SDL display
        */

        private:
        // contents of the canvas
        std::vector<GeoGerry::LinearRing> outlines;  // shapes to be drawn individually
        std::vector<GeoGerry::LinearRing> holes;     // holes to be drawn individually
        GeoGerry::bounding_box get_bounding_box();   // calculate bounding box of coordinates

        // modify canvas attributes
        void translate(long int x, long int y);      // move the outlines by x and y
        void scale(double scale_factor);             // scale the shapes by scale factor
        void rasterize_shapes();                     // determine pixel positions and values for coordiantes

        // meta information
        int x, y;                                    // dimensions of the screen
        Uint32* pixels;                              // the pixel array to write to screen
        GeoGerry::bounding_box box;                  // the outer bounding box

        public:
        Canvas(int dx, int dy) : x(dx), y(dy) {};

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