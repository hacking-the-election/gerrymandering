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

int RECURSION_STATE = 0;
double PADDING = (14.0/16.0);


Graphics::Pixel::Pixel() {
    color = Color(-1,-1,-1);
}


std::array<int, 3> Graphics::interpolate_rgb(std::array<int, 3> rgb1, std::array<int, 3> rgb2, double interpolator) {

    // return hsl_to_rgb(interpolate_hsl(rgb_to_hsl(rgb1), rgb_to_hsl(rgb2), interpolator));
    // return hsl_to_rgb(rgb_to_hsl(rgb1));
    int r = round(rgb1[0] + (double)(rgb2[0] - rgb1[0]) * interpolator);
    int g = round(rgb1[1] + (double)(rgb2[1] - rgb1[1]) * interpolator);
    int b = round(rgb1[2] + (double)(rgb2[2] - rgb1[2]) * interpolator);

    return {r, g, b};
}


double Graphics::hue_to_rgb(double p, double q, double t) {	
    if(t < 0) t += 1;	
    if(t > 1) t -= 1;	
    if(t < 1.0/6.0) return p + (q - p) * 6.0 * t;	
    if(t < 1.0/2.0) return q;	
    if(t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;	
    return p;	
}	


std::array<int, 3> Graphics::hsl_to_rgb(std::array<double, 3> hsl) {	
    double r, g, b;	

    if (hsl[1] == 0.0) {	
        r = g = b = hsl[2]; // achromatic	
    }	
    else {	
        double q = (hsl[2] < 0.5) ? hsl[2] * (1.0 + hsl[1]) : hsl[2] + hsl[1] - hsl[2] * hsl[1];	
        // std::cout << std::endl << "q: " << q << ", " << hsl[2] << std::endl;	
        double p = 2.0 * hsl[2] - q;	

        r = hue_to_rgb(p, q, hsl[0] + 1.0/3.0);	
        g = hue_to_rgb(p, q, hsl[0]);	
        b = hue_to_rgb(p, q, hsl[0] - 1.0/3.0);	
    }	

    return { (int)(r * 255.0), (int)(g * 255.0), (int) (b * 255.0) };	
}	


std::array<double, 3> Graphics::rgb_to_hsl(std::array<int, 3> rgb) {
    double r = (double) rgb[0] / 255.0;
    double g = (double) rgb[1] / 255.0;
    double b = (double) rgb[2] / 255.0;

    double max = (double)*std::max_element(rgb.begin(), rgb.end()) / 255.0,
           min = (double)*std::min_element(rgb.begin(), rgb.end()) / 255.0;

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

    return { h, s, l };	
}


std::array<double, 3> Graphics::interpolate_hsl(std::array<double, 3> hsl1, std::array<double, 3> hsl2, double interpolator) {	

    double h = hsl1[0] + (double)(hsl2[0] - hsl1[0]) * interpolator;
    double s = hsl1[1] + (double)(hsl2[1] - hsl1[1]) * interpolator;
    double l = hsl1[2] + (double)(hsl2[2] - hsl1[2]) * interpolator;

    return {h,s,l};	
}


Geometry::coordinate i_average(Geometry::bounding_box n) {
    /*
        @desc: Finds center of bounding box through averaging coords
        @params: `bounding_box` b: bounding box to average
        @return: `coordinate` center of box
    */

    double y = n[0] + n[1];
    double x = n[2] + n[3];
    
    return {(int)(x / (double) 2), (int)(y / (double) 2)};
}


void Graphics::Pixel::draw(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderDrawPoint(renderer, x, y);
}


void Graphics::Canvas::add_shape(Geometry::LinearRing s, bool f, Color c, int t) {
    /*
        @desc: Add a LinearRing object to the screen
        @params: `LinearRing` s: LinearRing object to add
        @return: void
    */

    Outline outline(s, c, t, f);
    outlines.push_back(outline);
    return;
}


void Graphics::Canvas::add_shape(Geometry::Polygon s, bool f, Color c, int t) {
    /*
        @desc: Add a shape object to the screen
        @params: `Polygon` s: Polygon object to add
        @return: void
    */

    Outline outline(s.hull, c, t, f);
    outlines.push_back(outline);

    for (Geometry::LinearRing l : s.holes) {
        Outline hole(l, c, t, f);
        holes.push_back(hole);
    }

    return;
}


void Graphics::Canvas::add_shape(Geometry::Multi_Polygon s, bool f, Color c, int t) {
    /*
        @desc: Add a shape object to the screen
        @params: `Polygon` s: Polygon object to add
        @return: void
    */

    for (Geometry::Polygon shape : s.border) {
        Outline outline(shape.hull, c, t, f);
        outlines.push_back(outline);

        for (Geometry::LinearRing l : shape.holes) {
            Outline hole(l, c, t, f);
            holes.push_back(hole);
        }
    }

    return;
}


void Graphics::Canvas::add_shape(Geometry::Precinct_Group s, bool f, Color c, int t) {
    /*
        @desc: Add a shape object to the screen
        @params: `Polygon` s: Polygon object to add
        @return: void
    */

    for (Geometry::Precinct shape : s.precincts) {
        double r = shape.get_ratio();

        Outline outline(shape.hull, c, t, f);
        outlines.push_back(outline);
        
        for (Geometry::LinearRing l : shape.holes) {
            Outline hole(l, c, t, f);
            holes.push_back(hole);
        }
    }
}


std::vector<Graphics::Color> generate_n_colors(int n) {
    
    std::vector<Graphics::Color> colors;
    for (int i = 0; i < 360; i += 360 / n) {
        std::array<int, 3> color = Graphics::hsl_to_rgb({(double)i / 360.0, (double)(80 + rand_num(0, 20)) / 100.0, (double)(50 + rand_num(0, 10)) / 100.0});
        Graphics::Color c(color[0], color[1], color[2]);
        colors.push_back(c);
    }

    std::shuffle(colors.begin(), colors.end(), std::random_device());
    return colors;
}


void Graphics::Canvas::add_shape(Geometry::Communities s, Geometry::Graph g, bool f, Graphics::Color c, int t) {
    std::vector<Graphics::Color> colors = generate_n_colors(s.size());

    for (int i = 0; i < s.size(); i++) {
        Geometry::Community community = s[i];
        for (Geometry::Polygon shape : community.shape.border) {
            Outline outline(shape.hull, colors[i], t, true);
            outlines.push_back(outline);


            for (Geometry::LinearRing l : shape.holes) {
                Outline hole(l, colors[i], t, true);
                holes.push_back(hole);
            }
        }

        Outline outline2(generate_exterior_border(community.shape).border[0].hull, Color(0,0,0), t, false);
        outlines.push_back(outline2);
    }

}


void Graphics::Canvas::add_graph(Geometry::Graph g) {
    for (int i = 0; i < g.vertices.size(); i++) {
        for (Geometry::Edge edge : g.vertices[i].edges) {
            Geometry::coordinate c1 = g.vertices[edge[0]].precinct->get_center();
            Geometry::coordinate c2 = g.vertices[edge[1]].precinct->get_center();
            Geometry::LinearRing lr({c1, c2});
            this->add_shape(lr, false, Color(0, 0, 0), 1);
        }
    }

    // for (int i = 0; i < g.vertices.size(); i++) {
    //     Geometry::Node node = g.vertices[i];

    //     std::array<int, 3> rgb = interpolate_rgb({0, 0, 255}, {255, 0, 0}, node.precinct->get_ratio());
    //     Color color(rgb[0], rgb[1], rgb[2]);
    //     if (node.precinct->get_ratio() == -1) color = Color(0,255,0);
    //     // int factor = 30000;
    //     int t = 8;
    //     // if (sqrt(node.precinct->pop * factor) > t) t = sqrt(node.precinct->pop * factor);
    //     Geometry::coordinate c = {(long int)round(node.precinct->get_center()[0] / 1000), (long int)round(node.precinct->get_center()[1] / 1000)};
    //     this->add_shape(generate_gon(c, t, 20), false, color, 1);
    //     // this->add_shape(generate_gon(node.precinct->get_center(), t, 20), false, Color(0, 0, 0), 1);
    // }

    cout << "added graph to canvas" << endl;
}


void Graphics::Canvas::resize_window(int dx, int dy) {
    x = dx;
    y = dy;
}


Geometry::bounding_box get_bounding_box(Graphics::Outline outline) {
    /*
        @desc: returns a bounding box of the outline
        
        @params: none;
        @return: `bounding_box` the superior bounding box of the shape
    */

    // set dummy extremes
    int top = outline.border.border[0][1], 
        bottom = outline.border.border[0][1], 
        left = outline.border.border[0][0], 
        right = outline.border.border[0][0];

    // loop through and find actual corner using ternary assignment
    for (Geometry::coordinate coord : outline.border.border) {
        if (coord[1] > top) top = coord[1];
        if (coord[1] < bottom) bottom = coord[1];
        if (coord[0] < left) left = coord[0];
        if (coord[0] > right) right = coord[0];
    }

    return {top, bottom, left, right}; // return bounding box
}


Geometry::bounding_box Graphics::Canvas::get_bounding_box() {
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
        for (Geometry::coordinate coord : ring.border.border) {
            if (coord[1] > top) top = coord[1];
            if (coord[1] < bottom) bottom = coord[1];
            if (coord[0] < left) left = coord[0];
            if (coord[0] > right) right = coord[0];
        }
    }

    box = {top, bottom, left, right};
    return {top, bottom, left, right}; // return bounding box
}


void Graphics::Canvas::translate(long int t_x, long int t_y, bool b) {
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


void Graphics::Canvas::scale(double scale_factor) {
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


void Graphics::Outline::flood_fill_util(Geometry::coordinate coord, Color c1, Color c2, Canvas& canvas) {
    RECURSION_STATE++;
    if (RECURSION_STATE > 10000) return;

    if (coord[0] < 0 || coord[0] > pixels.size() || coord[1] < 0 || coord[1] > pixels[0].size()) return;
    if (this->get_pixel({coord[0], coord[1]}).color != c1) return;
    
    Pixel p(coord[0], coord[1], c2);
    this->pixels[coord[0]][coord[1]] = p;
    canvas.pixels[coord[0]][coord[1]] = p;

    flood_fill_util({coord[0] + 1, coord[1]}, c1, c2, canvas);
    flood_fill_util({coord[0] - 1, coord[1]}, c1, c2, canvas);
    flood_fill_util({coord[0], coord[1] + 1}, c1, c2, canvas);
    flood_fill_util({coord[0], coord[1] - 1}, c1, c2, canvas);

    return;
}


Graphics::Pixel Graphics::Canvas::get_pixel(Geometry::coordinate c) {
    return this->pixels[c[0]][c[1]];
}


Graphics::Pixel Graphics::Outline::get_pixel(Geometry::coordinate c) {
    if (c[0] >= pixels.size() || c[0] < 0 || c[1] >= pixels[0].size() || c[1] < 0)
        return Pixel(-1,-1, Color(-1,-1,-1));
    return this->pixels[c[0]][c[1]];
}


void Graphics::Outline::flood_fill(Geometry::coordinate coord, Color c, Canvas& canvas) {
    RECURSION_STATE = 0;
    this->flood_fill_util(coord, Color(-1,-1,-1), c, canvas);
    return;
}


Geometry::coordinate Graphics::Outline::get_representative_point() {
    return this->border.get_center();
}


void Graphics::Outline::rasterize(Canvas& canvas) {
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


Uint32 Graphics::Pixel::get_uint() {
    Uint32 rgba = (0 << 24) +
         (color.r << 16) +
         (color.g << 8)  +
         color.b;
    return rgba;
}


Uint32 Graphics::Color::get_uint() {
    Uint32 rgba = (0 << 24) +
         (r << 16) +
         (g << 8)  +
         b;
    return rgba;
}


void Graphics::Canvas::draw() {
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

    // cout << "a" << endl;
    for (int i = 0; i < outlines.size(); i++) {
        outlines[i].rasterize(*this);
    }

    // cout << "a" << endl;

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


void Graphics::Canvas::clear() {
    // @warn: reset background here
    this->outlines = {};
    this->holes = {};
    this->background = new Uint32[x * y];
    memset(background, 255, x * y * sizeof(Uint32));
}


void Graphics::Anim::playback() {
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