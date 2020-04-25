/*=======================================
 canvas.cpp:                    k-vernooy
 last modified:               Tue, Feb 18
 
 A file for all the canvas functions for
 various gui apps, tests, functions and
 visualizations.
========================================*/

#pragma once

#include <array>
#include <SDL2/SDL.h>

#include "shape.hpp"
#include "community.hpp"


namespace Graphics {

    class Canvas;
    class Color;
    class Outline;
    class Pixel;


    std::array<int, 3> interpolate_rgb(std::array<int, 3> rgb1, std::array<int, 3> rgb2, double interpolator);
    double hue_to_rgb(double p, double q, double t);
    std::array<int, 3> hsl_to_rgb(std::array<double, 3> hsl);
    std::array<double, 3> rgb_to_hsl(std::array<int, 3> rgb);
    std::array<double, 3> interpolate_hsl(std::array<double, 3> hsl1, std::array<double, 3> hsl2, double interpolator);

    class Color {
        public:
        int r, g, b;
        Uint32 get_uint();

        void set_color(int rx, int gx, int bx) {
            r = rx;
            g = gx;
            b = bx;
        }

        friend bool operator!= (Color c1, Color c2) {
            return (c1.r != c2.r || c1.g != c2.g || c1.b != c2.b);
        }

        friend bool operator== (Color c1, Color c2) {
            return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
        }

        Color() {};
        Color(std::string hex);
        Color(int rx, int gx, int bx) : r(rx), g(gx), b(bx) {};
    };


    class Pixel {
        public:

            Color color;
            int x, y;
            Uint32 get_uint();
            Pixel();
            Pixel(int ax, int ay, Color c) : color(c), x(ax), y(ay) {}
            void draw(SDL_Renderer* renderer);
    };


    class Outline {
        public:
            Geometry::LinearRing border;
            Color color;
            int line_thickness;
            bool filled;

            Geometry::coordinate get_representative_point();
            
            std::vector<std::vector<Pixel> > pixels;
            Pixel get_pixel(Geometry::coordinate c);
            void rasterize(Canvas& canvas);

            // modify canvas attributes
            void flood_fill_util(Geometry::coordinate coord, Color c1, Color c2, Canvas& canvas);
            void flood_fill(Geometry::coordinate, Color c, Canvas& canvas);

            Outline(Geometry::LinearRing lr, Color c, int th, bool f) :
                border(lr), color(c), line_thickness(th), filled(f) {}
    };


    class Canvas {
        /*
            A class for storing information about a screen
            of pixels and shapes to be written to an SDL display
        */
       
        public:

        // contents of the canvas
        std::vector<Outline> outlines;               // shapes to be drawn individually
        std::vector<Outline> holes;                  // shapes to be drawn individually


        // meta information
        std::vector<std::vector<Pixel> > pixels;        // the pixel array to write to screen

        Geometry::bounding_box box;       // the outer bounding box
        Geometry::bounding_box get_bounding_box();   // calculate bounding box of coordinates
        Uint32* background;
        int x, y;                         // dimensions of the screen

        // modify canvas attributes
        void translate(long int x, long int y, bool b);      // move the outlines by x and y
        void scale(double scale_factor);             // scale the shapes by scale factor
        Pixel get_pixel(Geometry::coordinate c);

        Canvas(int dx, int dy) : x(dx), y(dy) {
            background = new Uint32[dx * dy];
            memset(background, 255, dx * dy * sizeof(Uint32));
        }

        // add shape to the canvas
        void add_shape(Geometry::Polygon s, bool = false, Color = Color(0,0,0), int = 1);
        void add_shape(Geometry::LinearRing s, bool = false, Color = Color(0,0,0), int = 1);
        void add_shape(Geometry::Multi_Polygon s, bool = false, Color = Color(0,0,0), int = 1);
        void add_shape(Geometry::Precinct_Group s, bool = false, Color = Color(0,0,0), int = 1);
        void add_shape(Geometry::Communities s, Geometry::Graph g, bool = false, Color = Color(0,0,0), int = 1);

        void add_graph(Geometry::Graph s);

        void clear();
        void resize_window(int x, int y);
        void draw();
    };

    class Anim {
        public:
        std::vector<Canvas> frames;
        int delay;

        void playback();
        Anim(int d) : delay(d) {};
    };
}