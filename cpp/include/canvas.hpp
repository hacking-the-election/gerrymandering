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

namespace hte {
namespace Graphics {

    // to render shapes
    class Canvas;

    // not inherited from base
    // color for naming attrs
    class RGB_Color;
    class HSL_Color;
    
    class Outline_Group;
    class Outline;
    class Style;
    class PixelBuffer;

    enum class ImageFmt { PNG, SVG, BMP, PNM };


    // for color space conversions (currently just hsl/rgb)
    double hue_to_rgb(double p, double q, double t);
    RGB_Color hsl_to_rgb(HSL_Color hsl);
    HSL_Color rgb_to_hsl(RGB_Color rgb);

    // interpolate between colors
    HSL_Color interpolate_hsl(HSL_Color, HSL_Color, double);
    RGB_Color interpolate_rgb(RGB_Color, RGB_Color, double);
    double lerp(double, double, double);

    // color palette generators
    std::vector<RGB_Color> generate_n_colors(int n);

    // convert geometry shapes into styled outlines
    Outline to_outline(Geometry::LinearRing r);
    std::vector<Outline> to_outline(Geometry::State state);
    std::vector<Outline> to_outline(Geometry::Graph& graph);
    std::vector<Outline> to_outline(Geometry::Communities& communities);
    Outline_Group to_outline(Geometry::Multi_Polygon&, double, bool abs_quant);
    Outline_Group to_outline(Geometry::Multi_Polygon&);


    class EdgeBucket {
    public:
        int miny;
        int maxy;
        double miny_x;
        double slope;
        
        friend bool operator< (const EdgeBucket& b1, const EdgeBucket& b2) {
            if (b1.miny < b2.miny) return true;
            if (b2.miny < b1.miny) return false;

            if (b1.miny_x < b2.miny_x) return true;
            if (b2.miny_x < b1.miny_x) return false;

            return false;
        }

        EdgeBucket() {}
    };


    class RGB_Color {
        // a color representing rgb color channels
        public:
            int r, g, b;
            Uint32 to_uint();
            static RGB_Color from_uint(Uint32 color);

            friend bool operator!= (const RGB_Color& c1, const RGB_Color& c2) {
                return (c1.r != c2.r || c1.g != c2.g || c1.b != c2.b);
            }

            friend bool operator== (const RGB_Color& c1, const RGB_Color& c2) {
                return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
            }

            // constructors with default conversions
            RGB_Color() {}
            RGB_Color(int r, int g, int b) : r(r), g(g), b(b) {}
            RGB_Color(std::string hex);
            RGB_Color(HSL_Color);
    };


    class HSL_Color {
        // a color representing hsl color channels
        public:
            double h, s, l;

            friend bool operator!= (const HSL_Color& c1, const HSL_Color& c2) {
                return (c1.h != c2.h || c1.s != c2.s || c1.l != c2.l);
            }

            friend bool operator== (const HSL_Color& c1, const HSL_Color& c2) {
                return (c1.h == c2.h && c1.s == c2.s && c1.l == c2.l);
            }

            // constructors with default conversions
            HSL_Color() {};
            HSL_Color(double h, double s, double l) : h(h), s(s), l(l) {}
            HSL_Color(RGB_Color);
            HSL_Color(std::string hex);
    };
 

    class PixelBuffer {
        // contains pixel data in the form of
        // uint array, see `Uint_to_rgb`

        public:
            int x, y;
            Uint32* ar;
        
            PixelBuffer() {};
            PixelBuffer(int x, int y) : x(x), y(y) { ar = new Uint32[x * y]; memset(ar, 255, x * y * sizeof(Uint32));}
            void resize(int t_x, int t_y) { x = t_x; y = t_y; ar = new Uint32[x * y]; memset(ar, 255, x * y * sizeof(Uint32));}

            void set_from_position(int, int, Uint32);
            Uint32 get_from_position(int a, int b);
            int index_from_position(int, int);
    };


    class Style {    
        public:

            RGB_Color fill_;
            RGB_Color outline_;
            double thickness_;
        
            Style& thickness(double);
            Style& fill(RGB_Color);
            Style& fill(HSL_Color);
            Style& outline(RGB_Color);
    };


    class Outline {
        private:
            Style style_;

        public:
            PixelBuffer pix;
            Geometry::LinearRing border;
            Style& style() {return style_;}
            std::string get_svg();

            Outline(Geometry::LinearRing border) : border(border) {}
    };


    class Outline_Group {
        public:
        
            std::string get_svg();
            Outline_Group() {};
            Outline_Group(Outline o) {
                outlines.push_back(o);
            }

            Outline_Group(std::vector<Outline> o) {
                outlines.insert(outlines.end(), o.begin(), o.end());
            }

            void add_outline(Outline o) {
                outlines.push_back(o);
            }

            std::vector<Outline> outlines;
    };


    class Canvas {
        /*
            A class for storing information about a screen
            of pixels and shapes to be written to an SDL display
        */

        private:
            // update the canvas's pixel buffer
            // to be called by internal methods such as to_gui();
            std::string get_svg();
            bool get_bmp(std::string write_path);
            bool get_pnm(std::string write_path);

        public:

            bool to_date = true;
            void rasterize();

            // contents of the canvas
            std::vector<Outline_Group> outlines;     // shapes to be drawn individually

            // meta information
            PixelBuffer pixel_buffer;
            
            // dimensions of the screen
            int width, height;

            Geometry::bounding_box box;
            Geometry::bounding_box get_bounding_box();

            // transformations for getting the coordinates of
            // the outlines in the right size

            void translate(long, long, bool);                       // move the outlines by x and y
            void scale(double scale_factor);                        // scale the shapes by scale factor
            void rotate(Geometry::coordinate center, int degrees);  // rotate the shapes by n degrees

            void save_image(ImageFmt, std::string);
            void save_img_to_anim(ImageFmt, std::string);

            Canvas(int width, int height) : width(width), height(height) {}

            // add shape to the canvas
            void add_outline(Outline o) {outlines.push_back(Outline_Group(o));};
            void add_outlines(std::vector<Outline> os) {for (Outline o : os) outlines.push_back(Outline_Group(o));}

            void add_outline_group(Outline_Group og) {outlines.push_back(og);}
            void add_outline_groups(std::vector<Outline_Group> ogs) {outlines.insert(outlines.end(), ogs.begin(), ogs.end());}

            void clear();
            void draw_to_window();
            void draw_to_window(SDL_Window* window);
    };


    // basic rasterizers
    void draw_line(
        PixelBuffer&, Geometry::coordinate, Geometry::coordinate,
        RGB_Color color = RGB_Color(0,0,0), double t = 1
    );

    void draw_polygon(PixelBuffer& buffer, Geometry::LinearRing ring, Style style);

}
}
