/*=======================================
 canvas.cpp:                    k-vernooy
 last modified:               Fri, Jun 19
 
 A file for all the canvas functions for
 various gui apps, tests, functions and
 visualizations.
========================================*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "algorithm.h"
#include "data.h"
#include "util.h"

#include <array>
#include <SDL2/SDL.h>


namespace hte {
namespace Graphics {

    class Canvas;
    class RgbColor;
    class HslColor;
    class OutlineGroup;
    class Outline;
    class Style;
    class PixelBuffer;

    /**
     * An enumeration representing various image
     * formats that the canvas can be written to
     */
    enum class ImageFileFormat {
        PNG, SVG, BMP, PNM
    };


    // for color space conversions (currently just hsl/rgb)
    double HueToRgb(double p, double q, double t);
    RgbColor HslToRgb(HslColor hsl);
    HslColor RgbToHsl(RgbColor rgb);

    // interpolate between colors
    HslColor InterpolateHsl(HslColor, HslColor, double);
    RgbColor InterpolateRgb(RgbColor, RgbColor, double);
    double Lerp(double, double, double);

    // color palette generators
    std::vector<RgbColor> GenerateColors(int n);

    // convert geometry shapes into styled outlines
    Outline ToOutline(Geometry::LinearRing r);
    std::vector<Outline> ToOutline(Data::State state);
    std::vector<Outline> ToOutline(Algorithm::Graph& graph);
    std::vector<Outline> ToOutline(Algorithm::Communities& communities);
    std::vector<OutlineGroup> ToOutlineGroup(Algorithm::Communities& communities);
    OutlineGroup ToOutline(Geometry::MultiPolygon&, double, bool abs_quant);
    OutlineGroup ToOutline(Geometry::MultiPolygon&);


    class EdgeBucket {
    public:
        int miny;
        int maxy;
        double minyX;
        double slope;
        
        friend bool operator< (const EdgeBucket& b1, const EdgeBucket& b2) {
            if (b1.miny < b2.miny) return true;
            if (b2.miny < b1.miny) return false;

            if (b1.minyX < b2.minyX) return true;
            if (b2.minyX < b1.minyX) return false;

            return false;
        }

        EdgeBucket() {}
    };


    class RgbColor {
        // a color representing rgb color channels
        public:
            int r, g, b;
            Uint32 toUint();
            static RgbColor fromUint(Uint32 color);

            friend bool operator!= (const RgbColor& c1, const RgbColor& c2) {
                return (c1.r != c2.r || c1.g != c2.g || c1.b != c2.b);
            }

            friend bool operator== (const RgbColor& c1, const RgbColor& c2) {
                return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
            }

            // constructors with default conversions
            RgbColor() {}
            RgbColor(int r, int g, int b) : r(r), g(g), b(b) {}
            RgbColor(std::string hex);
            RgbColor(HslColor);
    };


    class HslColor {
        // a color representing hsl color channels
        public:
            double h, s, l;

            friend bool operator!= (const HslColor& c1, const HslColor& c2) {
                return (c1.h != c2.h || c1.s != c2.s || c1.l != c2.l);
            }

            friend bool operator== (const HslColor& c1, const HslColor& c2) {
                return (c1.h == c2.h && c1.s == c2.s && c1.l == c2.l);
            }

            // constructors with default conversions
            HslColor() {};
            HslColor(double h, double s, double l) : h(h), s(s), l(l) {}
            HslColor(RgbColor);
            HslColor(std::string hex);
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

            void setFromPosition(int, int, Uint32);
            Uint32 getFromPosition(int a, int b);
            int indexFromPosition(int, int);
    };

    /**
     * \brief An encapsulation of visual attributes
     *
     * Contains fill, outline color, and thickness,
     * used for each outline in a canvas.
     */
    class Style {    
        public:

            RgbColor fill_;     //!< The fill color of an outline
            RgbColor outline_;  //!< The outline color
            double thickness_;   //!< The outline thickness (in pixels)
        
            Style& thickness(double);
            Style& fill(RgbColor);
            Style& fill(HslColor);
            Style& outline(RgbColor);
    };


    /**
     * \brief A graphics poly to be rendered
     * 
     * Contains a style object and a border
     * that represents coordinates of a LinearRing
     * to be rendered to a PixelBuffer.
     */
    class Outline {
        private:
            Style style_;  //!< An object storing thickness, fill, and color

        public:
            Outline(Geometry::LinearRing border) : border(border) {}
            Style& style() {return style_;}

            Geometry::LinearRing border;  //!< The coordinates of the outline
            
            /**
             * \brief Get the SVG string of the object
             * 
             * Gets a string with a `d` attribute representing
             * the LinearRing, and with inline styling representing
             * the `style_` object.
             * 
             * \param sf The scale factor to scale the canvas by (without rounding)
             * \return An inline SVG string
             */
            std::string getSvg(double sf);
    };


    /**
     * \brief A collection of outlines.
     * 
     * Used to represent a group of non-contiguous
     * outlines that must be grouped under interactions.
     */
    class OutlineGroup {
        public:
            /**
             * \brief Get an SVG string representing the outline group
             * \return The SVG string of the outlines
             */
            std::string getSvg(double);

            OutlineGroup() {};
            OutlineGroup(Outline o) { outlines.push_back(o); }
            OutlineGroup(std::vector<Outline> o) { outlines.insert(outlines.end(), o.begin(), o.end()); }

            /**
             * Add an outline to the outline group
             */
            void addOutline(Outline o) {outlines.push_back(o);}

            std::vector<Outline> outlines;  //!< A group of outlines
    };


    /**
     * \brief Represents a canvas for graphics and rendering
     * 
     * Used for rendering Outline objects to screens or images.
     * Can render shapes (filled or otherwise) and lines in combination
     * with Style objects.
     */
    class Canvas {
        /*
            A class for storing information about a screen
            of pixels and shapes to be written to an SDL display
        */

        private:
            // update the canvas's pixel buffer
            // to be called by internal methods such as to_gui();
            std::string getSvg();
            bool getBmp(std::string write_path);
            bool getPnm(std::string write_path);
            void resizeCont(bool scale_down);

        public:

            bool toDate = true;  //!< Whether or not the canvas' pixels are up to date
            
            /**
             * \brief Rasterize all outlines to pixel_buffer
             * 
             * Updates the canvas's pixel buffer with rasterized outlines
             * Rasterizes outlines using the Bresenham method (antialiased)
             * and fills polygons with the scanline fill method.
             */
            void rasterize();

            std::vector<OutlineGroup> outlines;   //!< The outline data (coordinates) to be written to pixels
            PixelBuffer pixelBuffer;              //!< The pixels to be written to a screen or image
            int width, height;                    //!< The size of the canvas in pixels
            Geometry::BoundingBox box;            //!< The axis-aligned bounding box of all outlines on the canvas

            /**
             * \brief Updates and returns the bounding box of all outline groups
             * in the canvas.
             */
            Geometry::BoundingBox getBoundingBox();
            Canvas(int width, int height) : width(width), height(height) {}

            /**
             * \brief Translate canvas objects
             * 
             * Translates all linear rings contained in the
             * canvas object by t_x and t_y
             * 
             * \param t_x  The x coordinate to translate by
             * \param t_y  The y coordinate to translate by
             */
            void translate(long, long, bool);

            /**
             * \brief Scale canvas objects
             * 
             * Scales all linear rings contained in the canvas
             * object by scale_factor (including holes)
             * \param scale_factor  The factor by which to scale coordinates
             */
            void scale(double scaleFactor);

            void saveImage(ImageFileFormat, std::string);
            void saveImageToAnim(ImageFileFormat, std::string);


            // add shape to the canvas
            void addOutline(Outline o) {outlines.push_back(OutlineGroup(o));};
            void addOutlines(std::vector<Outline> os) {for (Outline o : os) outlines.push_back(OutlineGroup(o));}

            void addOutlineGroup(OutlineGroup og) {outlines.push_back(og);}
            void addOutlineGroups(std::vector<OutlineGroup> ogs) {outlines.insert(outlines.end(), ogs.begin(), ogs.end());}

            void clear();

            /**
             * Draws the shapes in the canvas to the screen
             * (in the case of no window passed, create a window)
             */
            void drawToWindow();

            /**
             * Draws the shapes in the canvas to the screen
             * (in the case of no window passed, create a window)
             */
            void drawToWindow(SDL_Window* window);
    };


    /**
     * \brief: Rasterizes a line using Bresenham's algorithm
     * 
     * Uses the basic Bresenham rasterization algorithm, but
     * also fills with thickness and antialiasing. Draws with
     * any RGB provided color, defaulting to black.
     * 
     * \param buffer The PixelBuffer to rasterize a line to
     * \param start The start coordinate ot he line
     * \param end The endpoint of the line
     * \param t The thickness of the line
     * \param color The color of the line (default to black)
     */
    void DrawLine(
        PixelBuffer& buffer, Geometry::Point2d start, Geometry::Point2d end,
        RgbColor color = RgbColor(0,0,0), double t = 1
    );

    /**
     * \brief Draw a polygon with scanline fill
     * 
     * Rasterizes a LinearRing object using the scanline
     * fill algorithm. Can change fill and outline styles
     * 
     * \see Canvas::draw_polygon
     * \param buffer The PixelBuffer to draw a ring to
     * \param ring The coordinates to rasterize
     * \param style A style object containing color, thickness, and fill
     */
    void DrawPolygon(PixelBuffer& buffer, Geometry::LinearRing ring, Style style);
}
}

#endif
