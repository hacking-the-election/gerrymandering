/*=======================================
 canvas.cpp:                    k-vernooy
 last modified:               Tue, Feb 18
 
 A file for all the canvas functions for
 various gui apps, tests, functions and
 visualizations.
========================================*/

#include <iostream>
#include <random>

#include "../include/community.hpp"
#include "../include/util.hpp"
#include "../include/canvas.hpp"
#include "../include/geometry.hpp"
#include <boost/filesystem.hpp>

using std::cout;
using std::endl;
using namespace hte;
using namespace Graphics;

int RECURSION_STATE = 0;
double PADDING = (15.0/16.0);


Style& Style::outline(RGB_Color c) {
    outline_ = c;
    return *this;
}


Style& Style::thickness(int t) {
    thickness_ = t;
    return *this;
}


Style& Style::fill(RGB_Color c) {
    fill_ = c;
    return *this;
}


Style& Style::fill(HSL_Color c) {
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
    for (int i = 0; i < 360; i += 360 / n) {
        // create and add colors
        colors.push_back(
            hsl_to_rgb({
                (double)i / 360.0,
                (double)(80 + rand_num(0, 20)) / 100.0,
                (double)(50 + rand_num(0, 10)) / 100.0
            })
        );

    }

    return colors;
}


int PixelBuffer::index_from_position(int a, int b) {
    return ((x * (b - 1)) + a - 1);
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

    // set dummy extremes
    int top = outlines[0].border.border[0][1], 
        bottom = outlines[0].border.border[0][1], 
        left = outlines[0].border.border[0][0], 
        right = outlines[0].border.border[0][0];

    // loop through and find actual corner using ternary assignment
    for (Outline ring : outlines) {
        Geometry::bounding_box x = ring.border.get_bounding_box();
        if (x[0] > top) top = x[0];
        if (x[1] < bottom) bottom = x[1];
        if (x[2] < left) left = x[2];
        if (x[3] > right) right = x[3];
    }

    box = {top, bottom, left, right};
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
        for (int j = 0; j < outlines[i].border.border.size(); j++) {
            outlines[i].border.border[j][0] += t_x;
            outlines[i].border.border[j][1] += t_y;
        }
    }

    for (int i = 0; i < holes.size(); i++) {
        for (int j = 0; j < holes[i].border.border.size(); j++) {
            holes[i].border.border[j][0] += t_x;
            holes[i].border.border[j][1] += t_y;
        }
    }
    
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
        for (int j = 0; j < outlines[i].border.border.size(); j++) {
            outlines[i].border.border[j][0] *= scale_factor;
            outlines[i].border.border[j][1] *= scale_factor;
        }
    }

    for (int i = 0; i < holes.size(); i++) {
        for (int j = 0; j < holes[i].border.border.size(); j++) {
            holes[i].border.border[j][0] *= scale_factor;
            holes[i].border.border[j][1] *= scale_factor;
        }
    }
}


// void Outline::flood_fill_util(Geometry::coordinate coord, Color c1, Color c2, Canvas& canvas) {
//     RECURSION_STATE++;
//     if (RECURSION_STATE > 10000) return;

//     if (coord[0] < 0 || coord[0] > pixels.size() || coord[1] < 0 || coord[1] > pixels[0].size()) return;
//     if (this->get_pixel({coord[0], coord[1]}).color != c1) return;
    
//     Pixel p(coord[0], coord[1], c2);
//     this->pixels[coord[0]][coord[1]] = p;
//     canvas.pixels[coord[0]][coord[1]] = p;

//     flood_fill_util({coord[0] + 1, coord[1]}, c1, c2, canvas);
//     flood_fill_util({coord[0] - 1, coord[1]}, c1, c2, canvas);
//     flood_fill_util({coord[0], coord[1] + 1}, c1, c2, canvas);
//     flood_fill_util({coord[0], coord[1] - 1}, c1, c2, canvas);

//     return;
// }


// void Outline::flood_fill(Geometry::coordinate coord, Color c, Canvas& canvas) {
//     RECURSION_STATE = 0;
//     this->flood_fill_util(coord, Color(-1,-1,-1), c, canvas);
//     return;
// }


Geometry::coordinate Outline::get_representative_point() {
    return this->border.get_center();
}


void Outline::rasterize(Canvas& canvas) {
    /*
        @desc:
            scales an array of coordinates to fit on a screen
            of dimensions {x, y}, and determines pixel values
            and placements

        @params: none
        @return: void
    */


    if (filled) this->pixels = std::vector<std::vector<Pixel> > (canvas.x, std::vector<Pixel>(canvas.y, Pixel()));
    int dx, dy, x0, x1, y0, y1;

    for (Geometry::segment s : this->border.get_segments()) {
        x0 = s[0];
        y0 = s[1];
        x1 = s[2];
        y1 = s[3];

        dx = x1 - x0;
        dy = y1 - y0;

        int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

        double xinc = (double) dx / (double) steps;
        double yinc = (double) dy / (double) steps;

        double xv = (double) x0;
        double yv = (double) y0;

        for (int i = 0; i <= steps; i++) {
         
            if (filled) this->pixels[xv][yv] = Pixel((int)xv, (int)yv, this->color);
            canvas.pixels[xv][yv] = Pixel((int)xv , (int)yv, this->color);
            
            if (line_thickness > 1) {
                if (abs(dx) > abs(dy)) {
                    // up/down
                    int add = 0;
                    for (int i = 1; i <= ceil((double)(line_thickness - 1) / 2.0); i++) {
                        for (int f = 1; f >= -1; f -= 2) {
                            if (filled) this->pixels[xv][yv + (i * f)] = Pixel((int)xv, (int)yv + (i * f), this->color);
                            canvas.pixels[xv][yv + (i * f)] = Pixel((int)xv, (int)yv + (i * f), this->color);
                            add++;

                            if (add == line_thickness - 1) break;
                        }
                    }
                }
                else {
                    // left/right
                    int add = 0;
                    for (int i = 1; i <= ceil((double)(line_thickness - 1) / 2.0); i++) {
                        for (int f = 1; f >= -1; f -= 2) {
                            if (filled) this->pixels[xv + (i * f)][yv] = Pixel((int)xv + (i * f), (int)yv, this->color);
                            canvas.pixels[xv + (i * f)][yv] = Pixel((int)xv + (i * f), (int)yv, this->color);
                            add++;

                            if (add == line_thickness - 1) break;
                        }
                    }
                }
            }

            xv += xinc;
            yv += yinc;
        }
    }

    if (this->filled) {
        this->flood_fill(this->get_representative_point(), this->color, canvas);
    }

    this->pixels.clear();

    return;
}


Uint32 Pixel::get_uint() {
    Uint32 rgba = (0 << 24) +
         (color.r << 16) +
         (color.g << 8)  +
         color.b;
    return rgba;
}


Uint32 Color::get_uint() {
    Uint32 rgba = (0 << 24) +
         (r << 16) +
         (g << 8)  +
         b;
    return rgba;
}


void Canvas::draw() {
    /*
        @desc: Prints the shapes in the canvas to the screen
        @params: none
        @return: void
    */

    // size and position coordinates in the right quad
    this->pixels = std::vector<std::vector<Pixel> > (x, std::vector<Pixel>(y));
    memset(background, 255, x * y * sizeof(Uint32));
    get_bounding_box();
    translate(-box[2], -box[1], true);

    double ratio_top = ceil((double) this->box[0]) / (double) (x);   // the rounded ratio of top:top
    double ratio_right = ceil((double) this->box[3]) / (double) (y); // the rounded ratio of side:side
    double scale_factor = 1 / ((ratio_top > ratio_right) ? ratio_top : ratio_right); 
    scale(scale_factor * PADDING);

    int px = (int)((double)x * (1.0-PADDING) / 2.0), py = (int)((double)y * (1.0-PADDING) / 2.0);
    translate(px, py, false);

    // if (ratio_top < ratio_right) {
    //     // center vertically
    //     std::cout << "x" << std::endl;
    //     int t = (int)((((double)y - ((double)py * 2.0)) - (double)this->box[0]) / 2.0);
    //     std::cout << t << std::endl;
    //     translate(0, t, false);
    // }

    for (int i = 0; i < outlines.size(); i++) {
        outlines[i].rasterize(*this);
    }

    for (std::vector<Pixel> pr : this->pixels) {
        for (Pixel p : pr) {
            if (p.color.r != -1) {
                int total = (x * y) - 1;
                int start = p.y * x - p.x;

                if (total - start < x * y && total - start >= 0) 
                    this->background[total - start] = p.get_uint();
            }
        }
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Event event;
    SDL_Window* window = SDL_CreateWindow("Drawing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, x, y, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, x, y);

    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_UpdateTexture(texture, NULL, background, x * sizeof(Uint32));

    SDL_WaitEvent(&event);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    // // Create an empty RGB surface that will be used to create the screenshot bmp file
    // SDL_Surface* pScreenShot = SDL_CreateRGBSurface(0, x, y, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

    // if (pScreenShot) {
    //     // Read the pixels from the current render target and save them onto the surface
    //     SDL_RenderReadPixels(renderer, NULL, SDL_GetWindowPixelFormat(window), pScreenShot->pixels, pScreenShot->pitch); 

    //     std::string filename = "anim-out/test";
    //     int x = 0;
    //     std::string app = filename + std::to_string(x);

    //     do {
    //         x++;

    //         app = filename;
    //         if (x < 10)
    //             app += "0";
    //         if (x < 100)
    //             app += "0";

    //         app += std::to_string(x);
            
    //     } while (boost::filesystem::exists(app + ".bmp"));
    
    //     // Create the bmp screenshot file
    //     SDL_SaveBMP(pScreenShot, (app + ".bmp").c_str());

    //     // Destroy the screenshot surface
    //     SDL_FreeSurface(pScreenShot);
    // }


    bool quit = false;

    while (!quit) {
        SDL_UpdateTexture(texture, NULL, background, x * sizeof(Uint32));

        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT) quit = true;

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    // // destroy arrays and SDL objects1
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow( window );
    SDL_Quit();
}


void Canvas::clear() {
    // @warn: reset background here
    this->outlines = {};
    this->holes = {};
    this->background = new Uint32[x * y];
    memset(background, 255, x * y * sizeof(Uint32));
}


void Anim::playback() {
    /*
        Play back an animation object
    */
   
    // return;
    std::vector<Uint32*> backgrounds;
    for (Canvas c : frames) {
        Uint32 background[c.x * c.y];

        c.pixels = std::vector<std::vector<Pixel> > (c.x, std::vector<Pixel>(c.y));
        memset(background, 255, c.x * c.y * sizeof(Uint32));
        c.get_bounding_box();
        c.translate(-c.box[2], -c.box[1], true);

        double ratio_top = ceil((double) c.box[0]) / (double) (c.x);   // the rounded ratio of top:top
        double ratio_right = ceil((double) c.box[3]) / (double) (c.y); // the rounded ratio of side:side
        double scale_factor = 1 / ((ratio_top > ratio_right) ? ratio_top : ratio_right); 
        c.scale(scale_factor * PADDING);

        int px = (int)((double)c.x * (1.0-PADDING) / 2.0), py = (int)((double)c.y * (1.0-PADDING) / 2.0);
        c.translate(px, py, false);


        for (int i = 0; i < c.outlines.size(); i++) {
            c.outlines[i].rasterize(c);
        }

        for (std::vector<Pixel> pr : c.pixels) {
            for (Pixel p : pr) {
                if (p.color.r != -1) {
                    int total = (c.x * c.y) - 1;
                    int start = p.y * c.x - p.x;

                    if (total - start < c.x * c.y && total - start >= 0) 
                        background[total - start] = p.get_uint();
                }
            }
        }

        backgrounds.push_back(background);
    }

    cout << "playing back" << endl;

    Uint32* background = new Uint32[frames[0].x * frames[0].y];
    memset(background, 255, frames[0].x * frames[0].y * sizeof(Uint32));

    SDL_Init(SDL_INIT_VIDEO);

    // initialize window
    SDL_Event event;
    SDL_Window* window = SDL_CreateWindow("Drawing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, frames[0].x, frames[0].y, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, frames[0].x, frames[0].y);

    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_UpdateTexture(texture, NULL, background, frames[0].x * sizeof(Uint32));
    
    SDL_RenderPresent(renderer);
    // SDL_PollEvent(&event);
    // SDL_Delay(200);

    for (int i = 0; i < backgrounds.size() - 1; i++) {
        SDL_UpdateTexture(texture, NULL, backgrounds[i], frames[0].x * sizeof(Uint32));
        // SDL_PollEvent(&event);
        SDL_Delay(delay);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    bool quit = false;

    while (!quit) {
        SDL_UpdateTexture(texture, NULL, backgrounds[backgrounds.size() - 1], frames[0].x * sizeof(Uint32));
        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT) quit = true;

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    // destroy arrays and SDL objects
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow( window );
    SDL_Quit();
}
