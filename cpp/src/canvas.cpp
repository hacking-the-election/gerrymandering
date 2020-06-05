/*=======================================
 canvas.cpp:                    k-vernooy
 last modified:               Tue, Feb 18
 
 A file for all the canvas functions for
 various gui apps, tests, functions and
 visualizations.
========================================*/

#include <iostream>
#include <random>
#include <cmath>

#include "../include/community.hpp"
#include "../include/util.hpp"
#include "../include/canvas.hpp"
#include "../include/geometry.hpp"  
#include <boost/filesystem.hpp>

using std::cout;
using std::endl;
using std::vector;
using std::to_string;
using namespace hte;
using namespace Geometry;
using namespace Graphics;

namespace fs = boost::filesystem;

int RECURSION_STATE = 0;
double PADDING = (15.0/16.0);

vector<RGB_Color> COLORS = {RGB_Color(79,161,154),RGB_Color(220,65,182),RGB_Color(83,206,83),RGB_Color(92,79,210),RGB_Color(159,212,68),RGB_Color(185,87,218),RGB_Color(210,198,52),RGB_Color(138,49,146),RGB_Color(106,164,49),RGB_Color(164,122,223),RGB_Color(103,213,137),RGB_Color(227,54,102),RGB_Color(86,213,187),RGB_Color(224,71,48),RGB_Color(110,207,226),RGB_Color(163,53,37),RGB_Color(91,126,219),RGB_Color(230,157,47),RGB_Color(91,73,156),RGB_Color(215,185,92),RGB_Color(73,111,171),RGB_Color(199,101,39),RGB_Color(111,170,232),RGB_Color(144,143,44),RGB_Color(211,78,147),RGB_Color(72,145,66),RGB_Color(184,106,175),RGB_Color(66,102,30),RGB_Color(226,157,227),RGB_Color(68,152,108),RGB_Color(162,48,72),RGB_Color(178,200,121),RGB_Color(139,69,112),RGB_Color(158,201,160),RGB_Color(222,108,115),RGB_Color(34,106,107),RGB_Color(225,131,94),RGB_Color(76,157,190),RGB_Color(174,118,44),RGB_Color(193,182,235),RGB_Color(108,99,36),RGB_Color(80,80,126),RGB_Color(226,183,135),RGB_Color(56,108,139),RGB_Color(123,80,39),RGB_Color(146,128,175),RGB_Color(64,105,70),RGB_Color(221,155,187),RGB_Color(126,142,92),RGB_Color(134,71,59),RGB_Color(226,158,149),RGB_Color(174,136,89),RGB_Color(176,110,107)};


Outline Graphics::to_outline(Geometry::LinearRing r) {
    Outline o(r);
    o.style().fill(RGB_Color(-1, -1, -1)).outline(RGB_Color(0,0,0)).thickness(1);
    return o;
}


vector<Outline> Graphics::to_outline(Geometry::State state) {
    /*
        Return list of precinct outlines
    */

    vector<Outline> outlines;
    for (Geometry::Precinct p : state.precincts) {
        Outline o(p.hull);
        double ratio = 0.5;
        if (!(p.voter_data[POLITICAL_PARTY::DEMOCRAT] == 0 && p.voter_data[POLITICAL_PARTY::REPUBLICAN] == 0)) {
            ratio = (double)(p.voter_data[POLITICAL_PARTY::DEMOCRAT]) / (double)(p.voter_data[POLITICAL_PARTY::DEMOCRAT] + p.voter_data[POLITICAL_PARTY::REPUBLICAN]);
        }

        o.style().outline(interpolate_rgb(RGB_Color(255,0,0), RGB_Color(0,0,255), ratio)).thickness(1.0).fill(interpolate_rgb(RGB_Color(255,0,0), RGB_Color(0,0,255), ratio));
        outlines.push_back(o);
    }

    return outlines;
}


Outline_Group Graphics::to_outline(Geometry::Multi_Polygon& mp, double v, bool abs_quant) {
    Outline_Group os;
    RGB_Color fill;

    if (!abs_quant) {
        if (v < 0) {
            fill = interpolate_rgb(RGB_Color(255,255,255), RGB_Color(255, 0, 0), abs(v));
        }
        else {
            fill = interpolate_rgb(RGB_Color(255,255,255), RGB_Color(0, 0, 255), v);
        }
    }
    else {
        fill = interpolate_rgb(RGB_Color(0,0,0), RGB_Color(255,255,255), v);
    }

    for (Polygon p : mp.border) {
        Outline o = to_outline(p.hull);
        o.style().fill(fill);
        os.add_outline(o);
    }
    return os;
}


vector<Outline> Graphics::to_outline(Geometry::Graph& graph) {
    vector<Outline> outlines;
    for (int i = 0; i < graph.vertices.size(); i++) {
        Node node = (graph.vertices.begin() + i).value();
        Outline node_b(generate_gon(node.precinct->get_centroid(), 500, 50).hull);

        float ratio = 0.5;
        int sum = node.precinct->voter_data[POLITICAL_PARTY::DEMOCRAT] + node.precinct->voter_data[POLITICAL_PARTY::REPUBLICAN];
        if (sum <= 0) ratio = 0.5;
        else ratio = (float)node.precinct->voter_data[POLITICAL_PARTY::REPUBLICAN] / (float)sum;

        node_b.style().fill(interpolate_rgb(RGB_Color(0, 0, 255), RGB_Color(255,0,0), (double)ratio)).thickness(1).outline(interpolate_rgb(RGB_Color(0, 0, 255), RGB_Color(255,0,0), (double)ratio));
        outlines.push_back(node_b);
        
        for (Edge edge : node.edges) {
            if (graph.vertices.find(edge[1]) != graph.vertices.end()) {
                coordinate start = graph.vertices[edge[0]].precinct->get_centroid();
                coordinate end = graph.vertices[edge[1]].precinct->get_centroid();
                Outline o(LinearRing({start, end}));
                o.style().outline(RGB_Color(0,0,0)).thickness(1.0).fill(RGB_Color(-1,-1,-1));
                outlines.push_back(o);
            }
        }
    }

    return outlines;
}


vector<Outline> Graphics::to_outline(Communities& communities) {
    vector<Outline> outlines;
    vector<RGB_Color> colors = generate_n_colors(communities.size());

    for (int i = 0; i < communities.size(); i++) {

        for (auto& j : communities[i].vertices) {
            Outline o(j.second.precinct->hull);
            o.style().fill(colors[i]).outline(colors[i]).thickness(1);
            outlines.push_back(o);
        }

        // for (Polygon x : generate_exterior_border(communities[i].shape).border) {
        //     Outline border(x.hull);
        //     border.style().outline(RGB_Color(0,0,0)).thickness(2).fill(RGB_Color(-1, -1, -1));
        //     outlines.push_back(border);
        // }
    }

    return outlines;
} 


vector<Outline_Group> Graphics::to_outline_group(Geometry::Communities& communities) {
    vector<Outline_Group> districts;
    vector<RGB_Color> colors = generate_n_colors(communities.size());

    for (int i = 0; i < communities.size(); i++) {
        Outline_Group og;
        for (Polygon p : generate_exterior_border(communities[i].shape).border) {
            Outline o(p.hull);
            o.style().fill(colors[i]).outline(RGB_Color(0,0,0)).thickness(1);
            og.add_outline(o);
        }
        districts.push_back(og);
    }

    return districts;
}


Outline_Group Graphics::to_outline(Geometry::Multi_Polygon& mp) {
    Outline_Group o;
    for (Polygon p : mp.border) {
        o.add_outline(to_outline(p.hull));
    }
    return o;
}


Style& Style::outline(RGB_Color c) {
    // set the outline color
    outline_ = c;
    return *this;
}


Style& Style::thickness(double t) {
    // set the outline thickness
    thickness_ = t;
    return *this;
}


Style& Style::fill(RGB_Color c) {
    // set the fill color (RGB)
    fill_ = c;
    return *this;
}


Style& Style::fill(HSL_Color c) {
    // set the fill color (HSL)
    fill_ = hsl_to_rgb(c);
    return *this;
}


double Graphics::hue_to_rgb(double p, double q, double t) {	
    /*
        Convert a hue to a single rgb value
        I honestly forget how this works sorry future me
    */

    if(t < 0) t += 1;
    if(t > 1) t -= 1;
    if(t < 1.0/6.0) return p + (q - p) * 6.0 * t;
    if(t < 1.0/2.0) return q;
    if(t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;
    return p;
}	


RGB_Color Graphics::hsl_to_rgb(HSL_Color hsl) {
    /*
        @desc: Convert a HSL_Color object into RGB_Color
        @params: `HSL_Color` hsl: color to convert
        @return: `RGB_Color` converted color
    */

    double r, g, b;

    if (hsl.s == 0.0) {
        r = hsl.l;
        g = hsl.l;
        b = hsl.l;
    }
    else {
        double q = (hsl.l < 0.5) ? hsl.l * (1.0 + hsl.s)
            : hsl.l + hsl.s - hsl.l * hsl.s;

        double p = 2.0 * hsl.l - q;

        r = hue_to_rgb(p, q, hsl.h + 1.0/3.0);
        g = hue_to_rgb(p, q, hsl.h);
        b = hue_to_rgb(p, q, hsl.h - 1.0/3.0);
    }

    return RGB_Color((r * 255.0), (g * 255.0), (b * 255.0));
}


HSL_Color Graphics::rgb_to_hsl(RGB_Color rgb) {
    /*
        @desc: Converts RGB_Color object into HSL_Color
    */

    double r = (double) rgb.r / 255.0;
    double g = (double) rgb.g / 255.0;
    double b = (double) rgb.b / 255.0;
    double max = r, min = r;

    for (double x : {r,g,b}) {
        if (x > max) max = x;
        else if (x < min) min = x;
    }


    double h, s, l = (max + min) / 2.0;

    if (max == min) {
        h = 0.0;
        s = 0.0; // achromatic
    }
    else {
        double d = max - min;
        s = (l > 0.5) ? (d / (2.0 - max - min)) : (d / (max + min));

        if (max == r) h = (g - b) / d + ((g < b) ? 6.0 : 0.0);
        else if (max == g) h = (b - r) / d + 2;
        else h = (r - g) / d + 4;

        h /= 6.0;
    }

    return HSL_Color(h, s, l);
}


double Graphics::lerp(double a, double b, double t) {
    return(a + (b - a) * t);
}


HSL_Color Graphics::interpolate_hsl(HSL_Color hsl1, HSL_Color hsl2, double interpolator) {	
    return HSL_Color(
        lerp(hsl1.h, hsl2.h, interpolator),
        lerp(hsl1.s, hsl2.s, interpolator),
        lerp(hsl1.l, hsl2.l, interpolator)
    );
}


RGB_Color Graphics::interpolate_rgb(RGB_Color rgb1, RGB_Color rgb2, double interpolator) {
    return RGB_Color(
        round(lerp(rgb1.r, rgb2.r, interpolator)),
        round(lerp(rgb1.g, rgb2.g, interpolator)),
        round(lerp(rgb1.b, rgb2.b, interpolator))
    );
}


std::vector<RGB_Color> Graphics::generate_n_colors(int n) {
    /*
        @desc: Generates a number of colors blindly (and semi-randomly)
        @params: `int` n: number of colors to generate
        @return: `vector<Graphics::RGB_Color>` list of color objects
    */

    std::vector<RGB_Color> colors;

    int shift = 310;
    for (int i = shift; i < 360 + shift; i += 360 / n) {
        // create and add colors
        colors.push_back(
            hsl_to_rgb(
                HSL_Color(
                    ((double)(i % 360) / 360.0),
                    (86.0 / 100.0),
                    (72.0 / 100.0)
                )
            )
        );
    }

    return colors;
}


int PixelBuffer::index_from_position(int a, int b) {
    if (a >= 0 && b >= 0)
        return ((x * (b - 1)) + a - 1);
    else return (1);
}


void PixelBuffer::set_from_position(int a, int b, Uint32 value) {
    ar[index_from_position(a, b)] = value;
}


Uint32 PixelBuffer::get_from_position(int a, int b) {
    return ar[index_from_position(a, b)];
}


Geometry::bounding_box get_bounding_box(Outline outline) {
    /*
        @desc: returns a bounding box of the outline
        
        @params: none;
        @return: `bounding_box` the superior bounding box of the shape
    */

    // set dummy extremes
    return outline.border.get_bounding_box();
}


Geometry::bounding_box Canvas::get_bounding_box() {
    /*
        @desc:
            returns a bounding box of the internal list of hulls
            (because holes cannot be outside shapes)
        
        @params: none;
        @return: `bounding_box` the superior bounding box of the shape
    */

    if (outlines.size() > 0) {
        // set dummy extremes
        int top = outlines[0].outlines[0].border.border[0][1], 
            bottom = outlines[0].outlines[0].border.border[0][1], 
            left = outlines[0].outlines[0].border.border[0][0], 
            right = outlines[0].outlines[0].border.border[0][0];

        // loop through and find actual corner using ternary assignment
        for (Outline_Group og : outlines) {
            for (Outline ring : og.outlines) {
                Geometry::bounding_box x = ring.border.get_bounding_box();
                if (x[0] > top) top = x[0];
                if (x[1] < bottom) bottom = x[1];
                if (x[2] < left) left = x[2];
                if (x[3] > right) right = x[3];
            }
        }

        box = {top, bottom, left, right};
    }
    else {
        box = {height, 0, 0, width};
    }

    return box; // return bounding box
}


void Canvas::translate(long int t_x, long int t_y, bool b) {
    /*
        @desc:
            Translates all linear rings contained in the
            canvas object by t_x and t_y
        
        @params: 
            `long int` t_x: x coordinate to translate
            `long int` t_y: y coordinate to translate
        
        @return: void
    */

    for (int i = 0; i < outlines.size(); i++) {
        for (int o = 0; o < outlines[i].outlines.size(); o++) {
            for (int j = 0; j < outlines[i].outlines[o].border.border.size(); j++) {
                outlines[i].outlines[o].border.border[j][0] += t_x;
                outlines[i].outlines[o].border.border[j][1] += t_y;
            }
        }
    }

    to_date = false;
    if (b) box = {box[0] + t_y, box[1] + t_y, box[2] + t_x, box[3] + t_x};
}


void Canvas::scale(double scale_factor) {
    /*
        @desc:
            Scales all linear rings contained in the canvas
            object by scale_factor (including holes)
        
        @params: `double` scale_factor: factor to scale coordinates by
        
        @return: void
    */

    for (int i = 0; i < outlines.size(); i++) {
        for (int o = 0; o < outlines[i].outlines.size(); o++) {
            for (int j = 0; j < outlines[i].outlines[o].border.border.size(); j++) {
                outlines[i].outlines[o].border.border[j][0] *= scale_factor;
                outlines[i].outlines[o].border.border[j][1] *= scale_factor;
            }
        }
    }

    to_date = false;
}


bool edge_bucket_null(EdgeBucket b) {
    return (b.slope == 0);
}


void Graphics::draw_line(PixelBuffer& buffer, Geometry::coordinate start, Geometry::coordinate end, RGB_Color color, double t) {
    /*
        @desc: draws a line from a to b using Bresenham's algorithm

        @params: 
            `PixelBuffer&` buffer: the buffer to draw line to
            `Geometry::coordinate` start, end: endpoints of the line
            `double` t: thickness of the line

        @return: void
    */

    int dx = abs(end[0] - start[0]), sx = start[0] < end[0] ? 1 : -1;
    int dy = abs(end[1] - start[1]), sy = start[1] < end[1] ? 1 : -1;
    int err = dx - dy, e2, x2, y2;
    float ed = dx + dy == 0 ? 1 : sqrt((float)dx * dx + (float)dy * dy);

    for (t = (t + 1) / 2; ;) {
        // if cval is 0, we want to draw pure color
        double cval = std::max(0.0, 255 * (abs(err - dx + dy) / ed - t + 1)) / 255.0;
        buffer.set_from_position(start[0], start[1], interpolate_rgb(color, RGB_Color::from_uint(buffer.get_from_position(start[0], start[1])), cval).to_uint());
        e2 = err; x2 = start[0];

        if (2 * e2 >= -dx) {
            for (e2 += dy, y2 = start[1]; e2 < ed*t && (end[1] != y2 || dx > dy); e2 += dx) {
                double cval = std::max(0.0, 255 * (abs(e2) / ed - t + 1)) / 255.0;
                buffer.set_from_position(start[0], y2 += sy, interpolate_rgb(color, RGB_Color::from_uint(buffer.get_from_position(start[0], start[1])), cval).to_uint());
            }
            if (start[0] == end[0]) break;
            e2 = err; err -= dy; start[0] += sx; 
        } 
        if (2 * e2 <= dy) {
            for (e2 = dx - e2; e2 < ed * t && (end[0] != x2 || dx < dy); e2 += dy) {
                int cval = std::max(0.0, 255 * (abs(e2) / ed - t + 1)) / 255.0;
                buffer.set_from_position(x2 += sx, start[1], interpolate_rgb(color, RGB_Color::from_uint(buffer.get_from_position(start[0], start[1])), cval).to_uint());
            }
            if (start[1] == end[1]) break;
            err += dx; start[1] += sy; 
        }
    }

    return;
}


void Graphics::draw_polygon(PixelBuffer& buffer, Geometry::LinearRing ring, Style style) {
    /*
        @desc: draws a filled and/or stroked LinearRing to a framebuffer

        @params: 
            `PixelBuffer&` buffer: the buffer to draw polygon
            `Geometry::LinearRing`: coordinates defining polygon (*not* in pixel space)
            `Style` style: style object (fill, outline) to apply to rasterized poly

        @return: void
    */

    // fill polygon
    if (style.fill_.r != -1) {
        vector<EdgeBucket> all_edges;
        for (Geometry::segment seg : ring.get_segments()) {
            EdgeBucket bucket;
            bucket.miny = std::min(seg[1], seg[3]);
            bucket.maxy = std::max(seg[1], seg[3]);
            bucket.miny_x = (bucket.miny == seg[1]) ? seg[0] : seg[2];
            // bucket.slope = get_equation(seg)[0]; // @warn
            int dy = seg[3] - seg[1];
            int dx = seg[2] - seg[0];

            if (dx == 0) bucket.slope = INFINITY;
            else bucket.slope = (double)dy / (double)dx;

            all_edges.push_back(bucket);
        }

        // this following algorithm can probably get faster
        vector<EdgeBucket> global_edges = {};

        for (int i = 0; i < all_edges.size(); i++) {
            if (all_edges[i].slope != 0) {
                global_edges.push_back(all_edges[i]);
            }
        }

        std::sort(global_edges.begin(), global_edges.end());
        if (global_edges.size() != 0) {
            int scan_line = global_edges[0].miny;
            vector<EdgeBucket> active_edges = {};

            for (int i = 0; i < global_edges.size(); i++) {
                if (global_edges[i].miny > scan_line) break;
                if (global_edges[i].miny == scan_line) active_edges.push_back(global_edges[i]);
            }

            while (active_edges.size() > 0) {
                for (int i = 0; i < active_edges.size(); i += 2) {
                    // draw all points between edges with even parity
                    for (int j = active_edges[i].miny_x; j <= active_edges[i + 1].miny_x; j++) {
                        buffer.set_from_position(j, buffer.y - scan_line, style.fill_.to_uint());
                    }
                }

                scan_line++;

                // Remove any edges from the active edge table for which the maximum y value is equal to the scan_line.
                for (int i = 0; i < active_edges.size(); i++) {
                    if (active_edges[i].maxy == scan_line) {
                        active_edges.erase(active_edges.begin() + i);
                        i--;
                    }
                    else {
                        active_edges[i].miny_x = (double)active_edges[i].miny_x + (double)(1.0 / active_edges[i].slope);
                    }
                }


                for (int i = 0; i < global_edges.size(); i++) {
                    if (global_edges[i].miny == scan_line) {
                        active_edges.push_back(global_edges[i]);
                        global_edges.erase(global_edges.begin() + i);
                        i--;
                    }
                }

                std::sort(active_edges.begin(), active_edges.end(), [](EdgeBucket& one, EdgeBucket& two){return one.miny_x < two.miny_x;});
            }

        }
    }

    // draw polygon outline 
    for (Geometry::segment s : ring.get_segments()) {
        draw_line(buffer, {s[0], buffer.y - s[1]}, {s[2], buffer.y - s[3]}, style.outline_, style.thickness_);
    }
}


Uint32 RGB_Color::to_uint() {
    Uint32 argb =
        (255 << 24) +
        (r << 16) +
        (g << 8)  +
        (b);

    return argb;
}


RGB_Color RGB_Color::from_uint(Uint32 color) {
    int t_r = (color >> 16) & 255;
    int t_g = (color >> 8) & 255;
    int t_b = color & 255;

    return RGB_Color(t_r,t_g,t_b);
}


void Canvas::clear() {
    // @warn: reset background here
    this->outlines = {};
    this->pixel_buffer = PixelBuffer(width, height);
    to_date = true;
}


bool Canvas::get_bmp(std::string write_path) {
    /*
        @desc: Takes a screenshot as a BMP image of an SDL surface
        @params:
            `string` write_path: output image file
    */

    // SDL_Init(SDL_INIT_VIDEO);
    
    SDL_Event event;
    SDL_Window* window = SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);

    SDL_UpdateTexture(texture, NULL, pixel_buffer.ar, width * sizeof(Uint32));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // create empty RGB surface to hold window data
    SDL_Surface* pScreenShot = SDL_CreateRGBSurface(
        0, width, height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000
    );
    
    // check file path and output
    if (boost::filesystem::exists(write_path + ".bmp")) {
        cout << "File already exists, returning" << endl;
        return false;
    }
    
    if (pScreenShot) {
        // read pixels from render target, save to surface
        SDL_RenderReadPixels(
            renderer, NULL, SDL_GetWindowPixelFormat(window),
            pScreenShot->pixels, pScreenShot->pitch
        );

        // Create the bmp screenshot file
        SDL_SaveBMP(pScreenShot, std::string(write_path + ".bmp").c_str());
    }
    else {
        cout << "Uncaught error here, returning" << endl;
        return false;
    }

    SDL_FreeSurface(pScreenShot);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return true;
}


std::string Outline::get_svg() {
    std::string svg = "<path d=\"M";
    svg += std::to_string(border.border[0][0]) + "," + std::to_string(border.border[0][1]);

    for (segment s : border.get_segments()) {
        svg += "L";
        svg += std::to_string(s[0]) + "," + std::to_string(s[1]) + "," 
            + std::to_string(s[2]) + "," + std::to_string(s[3]);
    }

    svg += "z\" stroke=\"rgb(" + std::to_string(style().outline_.r) + "," + std::to_string(style().outline_.g) + ","
        + std::to_string(style().outline_.b) + ")\" fill=\"rgb(" + std::to_string(style().fill_.r) + "," + std::to_string(style().fill_.g) + ","
        + std::to_string(style().fill_.b) + ")\" stroke-width=\"" + std::to_string(style().thickness_) + "\"></path>";

    return svg;
}


std::string Outline_Group::get_svg() {
    if (outlines.size() == 0) cout << "NO OUTLINES, EXPECT SEGFAULT" << endl;

    std::string svg = "<path d=\"";

    for (Outline o : outlines) {
        svg += "M " + std::to_string(o.border.border[0][0]) + "," + std::to_string(o.border.border[0][1]);
        for (segment s : o.border.get_segments()) {
            svg += "L";
            svg += std::to_string(s[0]) + "," + std::to_string(s[1]) + "," 
                + std::to_string(s[2]) + "," + std::to_string(s[3]);
        }
        svg += "Z ";
    }
    
    svg += "\" stroke=\"rgb(" + std::to_string(outlines[0].style().outline_.r) + "," + std::to_string(outlines[0].style().outline_.g) + ","
        + std::to_string(outlines[0].style().outline_.b) + ")\" fill=\"rgb(" + std::to_string(outlines[0].style().fill_.r) + "," + std::to_string(outlines[0].style().fill_.g) + ","
        + std::to_string(outlines[0].style().fill_.b) + ")\" stroke-width=\"" + std::to_string(outlines[0].style().thickness_) + "\"></path>";

    return svg;
}


std::string Canvas::get_svg() {
    /*
        @desc: gets a string representing an svg graphic from a canvas
        @params: none
        @return: string
    */

    std::string svg = "<svg xmlns=\"http://www.w3.org/2000/svg\" height=\"100%\" width=\"100%\" viewBox=\"0 0 " + std::to_string(width) + " " + std::to_string(height) + "\">";
    
    for (Outline_Group o : outlines) {
        svg += o.get_svg();
    }

    return (svg + "</svg>");
}


bool Canvas::get_pnm(std::string write_path) {
    std::string file = "P3 " + to_string(width) + " " + to_string(height) + " 255\n";
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            RGB_Color c = RGB_Color::from_uint(pixel_buffer.get_from_position(j, i));
            file += to_string(c.r) + " " + to_string(c.g) + " " + to_string(c.b) + " ";
        }
        file.pop_back();
        file += "\n";
    }

    writef(file, write_path);
    return true;
}


void Canvas::save_image(ImageFmt fmt, std::string path) {
    /*if (!to_date && fmt != ImageFmt::SVG)*/
    rasterize();

    if (fmt == ImageFmt::BMP) {
        get_bmp(path);
    }
    else if (fmt == ImageFmt::SVG) {
        writef(get_svg(), path + ".svg");
    }
    else if (fmt == ImageFmt::PNM) {
        get_pnm(path);
    }
}


void Canvas::save_img_to_anim(ImageFmt fmt, std::string anim_dir) {

    if (!fs::is_directory(fs::path(anim_dir))) {
        cout << "creating dir " << anim_dir << endl;
        fs::create_directory(fs::path(anim_dir));
    }

    std::string filename = anim_dir + "/";
    int x = 0;
    std::string app = filename + std::to_string(x);

    do {
        x++;
        app = filename;
        if (x < 10)
            app += "0";
        if (x < 100)
            app += "0";

        app += std::to_string(x);
    } while (boost::filesystem::exists(app + ".bmp"));

    save_image(fmt, app);
}


void Canvas::rasterize() {
    /*
        @desc: Updates the canvas's pixel buffer with rasterized outlines
        @params: none
        @return `void`
    */

    // if (!to_date) {
    pixel_buffer = PixelBuffer(width, height);
    // @warn may be doing extra computation here
    get_bounding_box();

    // translate into first quadrant
    translate(-box[2], -box[1], true);
    
    // determine smaller side/side ratio for scaling
    double ratio_top = ceil((double) this->box[0]) / (double) (width);
    double ratio_right = ceil((double) this->box[3]) / (double) (height);
    double scale_factor = 1 / ((ratio_top > ratio_right) ? ratio_top : ratio_right); 
    scale(scale_factor * PADDING);

    // add padding and translate for corner sizes
    int px = (int)((double)width * (1.0-PADDING) / 2.0), py = (int)((double)height * (1.0-PADDING) / 2.0);
    translate(px, py, false);

    if (ratio_top < ratio_right) {
        // center vertically
        int t = (int)((((double)height - ((double)py * 2.0)) - (double)this->box[0] * scale_factor) / 2.0);
        translate(0, t, false);
    }
    else {
        int t = (int)((((double)width - ((double)px * 2.0)) - (double)this->box[3] * scale_factor) / 2.0);
        translate(t, 0, false);
    }
    

    for (Outline_Group o : outlines) {
        for (Outline outline : o.outlines) {
            draw_polygon(pixel_buffer, outline.border, outline.style());
        }
    }
    
    to_date = true;
    // draw_to_window();
}


void Canvas::draw_to_window() {
    /*
        @desc:
            Prints the shapes in the canvas to the screen
            (in the case of no window passed, create a window)

        @params: none
        @return: void
    */

    this->rasterize();
    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_Event event;
    SDL_Window* window = SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);

    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_UpdateTexture(texture, NULL, pixel_buffer.ar, width * sizeof(Uint32));
    SDL_WaitEvent(&event);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    bool quit = false;

    while (!quit) {
        SDL_UpdateTexture(texture, NULL, pixel_buffer.ar, width * sizeof(Uint32));
        SDL_WaitEvent(&event);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        if (event.type == SDL_QUIT) quit = true;
    }

    // destroy arrays and SDL objects
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
