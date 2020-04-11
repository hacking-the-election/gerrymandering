/*=======================================
 canvas.cpp:                    k-vernooy
 last modified:               Tue, Feb 18
 
 A file for all the canvas functions for
 various gui apps, tests, functions and
 visualizations.
========================================*/

#include <iostream>

#include "../include/canvas.hpp"
#include "../include/geometry.hpp"

using std::cout;
using std::endl;

int RECURSION_STATE = 0;
double PADDING = (3.0/4.0);



GeoDraw::Pixel::Pixel() {
    color = Color(-1,-1,-1);
}


std::array<int, 3> GeoDraw::interpolate_rgb(std::array<int, 3> rgb1, std::array<int, 3> rgb2, double interpolator) {
    int r = round(rgb1[0] + (double)(rgb2[0] - rgb1[0]) * interpolator);
    int g = round(rgb1[1] + (double)(rgb2[1] - rgb1[1]) * interpolator);
    int b = round(rgb1[2] + (double)(rgb2[2] - rgb1[2]) * interpolator);

    return {r, g, b};
}


GeoGerry::coordinate i_average(GeoGerry::bounding_box n) {
    /*
        @desc: Finds center of bounding box through averaging coords
        @params: `bounding_box` b: bounding box to average
        @return: `coordinate` center of box
    */

    double y = n[0] + n[1];
    double x = n[2] + n[3];
    
    return {(int)(x / (double) 2), (int)(y / (double) 2)};
}


void GeoDraw::Pixel::draw(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderDrawPoint(renderer, x, y);
}


void GeoDraw::Canvas::add_shape(GeoGerry::LinearRing s, bool f, Color c, int t) {
    /*
        @desc: Add a LinearRing object to the screen
        @params: `LinearRing` s: LinearRing object to add
        @return: void
    */

    Outline outline(s, c, t, f);
    outlines.push_back(outline);
    return;
}


void GeoDraw::Canvas::add_shape(GeoGerry::Polygon s, bool f, Color c, int t) {
    /*
        @desc: Add a shape object to the screen
        @params: `Polygon` s: Polygon object to add
        @return: void
    */

    Outline outline(s.hull, c, t, f);
    outlines.push_back(outline);

    for (GeoGerry::LinearRing l : s.holes) {
        Outline hole(l, c, t, f);
        holes.push_back(hole);
    }

    return;
}


void GeoDraw::Canvas::add_shape(GeoGerry::Multi_Polygon s, bool f, Color c, int t) {
    /*
        @desc: Add a shape object to the screen
        @params: `Polygon` s: Polygon object to add
        @return: void
    */

    for (GeoGerry::Polygon shape : s.border) {
        Outline outline(shape.hull, c, t, f);
        outlines.push_back(outline);

        for (GeoGerry::LinearRing l : shape.holes) {
            Outline hole(l, c, t, f);
            holes.push_back(hole);
        }
    }

    return;
}


void GeoDraw::Canvas::add_shape(GeoGerry::Precinct_Group s, bool f, Color c, int t) {
    /*
        @desc: Add a shape object to the screen
        @params: `Polygon` s: Polygon object to add
        @return: void
    */

    for (GeoGerry::Precinct shape : s.precincts) {
            Outline outline(shape.hull, c, t, f);
            outlines.push_back(outline);
        for (GeoGerry::LinearRing l : shape.holes) {
            Outline hole(l, c, t, f);
            holes.push_back(hole);
        }
    }
}


void GeoDraw::Canvas::add_shape(GeoGerry::Communities s, bool f, GeoDraw::Color c, int t) {
    for (GeoGerry::Community community : s) {
        for (GeoGerry::Polygon shape : community.border) {
            Outline outline(shape.hull, c, t, f);
            outlines.push_back(outline);

            for (GeoGerry::LinearRing l : shape.holes) {
                Outline hole(l, c, t, f);
                holes.push_back(hole);
            }
        }
    }
}


void GeoDraw::Canvas::add_graph(GeoGerry::Graph g) {

    for (std::array<int, 2> edge : g.edges) {
        GeoGerry::coordinate c1 = g.vertices[g.get_node(edge[0])].precinct->get_center();
        GeoGerry::coordinate c2 = g.vertices[g.get_node(edge[1])].precinct->get_center();
        GeoGerry::LinearRing lr({c1, c2});
        this->add_shape(lr, false, Color(200, 200, 200), 1);
    }

    
    for (GeoGerry::Node node : g.vertices) {
        std::array<int, 3> rgb = interpolate_rgb({3, 61, 252}, {252, 3, 3}, node.precinct->get_ratio());
        Color color(rgb[0], rgb[1], rgb[2]);
        if (node.precinct->get_ratio() == -1) color = Color(0,0,0);

        int factor = 8;
        int t = 3500;
        if (node.precinct->pop * factor > 3500) t = node.precinct->pop * factor;

        this->add_shape(generate_gon(node.precinct->get_center(), t, 40), false, color, 2);
    }
}


void GeoDraw::Canvas::resize_window(int dx, int dy) {
    x = dx;
    y = dy;
}


GeoGerry::bounding_box get_bounding_box(GeoDraw::Outline outline) {
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
    for (GeoGerry::coordinate coord : outline.border.border) {
        if (coord[1] > top) top = coord[1];
        if (coord[1] < bottom) bottom = coord[1];
        if (coord[0] < left) left = coord[0];
        if (coord[0] > right) right = coord[0];
    }

    return {top, bottom, left, right}; // return bounding box
}


GeoGerry::bounding_box GeoDraw::Canvas::get_bounding_box() {
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
        for (GeoGerry::coordinate coord : ring.border.border) {
            if (coord[1] > top) top = coord[1];
            if (coord[1] < bottom) bottom = coord[1];
            if (coord[0] < left) left = coord[0];
            if (coord[0] > right) right = coord[0];
        }
    }

    box = {top, bottom, left, right};
    return {top, bottom, left, right}; // return bounding box
}


void GeoDraw::Canvas::translate(long int t_x, long int t_y, bool b) {
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


void GeoDraw::Canvas::scale(double scale_factor) {
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


void GeoDraw::Outline::flood_fill_util(GeoGerry::coordinate coord, Color c1, Color c2, Canvas& canvas) {
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


GeoDraw::Pixel GeoDraw::Canvas::get_pixel(GeoGerry::coordinate c) {
    return this->pixels[c[0]][c[1]];
}


GeoDraw::Pixel GeoDraw::Outline::get_pixel(GeoGerry::coordinate c) {
    // if (c[0] >= pixels.size() || c[0] < 0 || c[1] >= pixels[0].size() || c[1] < 0)
    //     return Pixel(-1,-1, Color(-1,-1,-1));
    return this->pixels[c[0]][c[1]];
}


void GeoDraw::Outline::flood_fill(GeoGerry::coordinate coord, Color c, Canvas& canvas) {
    RECURSION_STATE = 0;
    this->flood_fill_util(coord, Color(-1,-1,-1), c, canvas);
    return;
}


GeoGerry::coordinate GeoDraw::Outline::get_representative_point() {
    return this->border.get_center();
}


void GeoDraw::Outline::rasterize(Canvas& canvas) {
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
    
    for (GeoGerry::segment s : this->border.get_segments()) {
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
         
            // if (filled) this->pixels[xv][yv] = Pixel((int)xv, (int)yv, this->color);
            canvas.pixels[xv][yv] = Pixel((int)xv , (int)yv, this->color);
            
            if (line_thickness > 1) {
                if (abs(dx) > abs(dy)) {
                    // up/down
                    int add = 0;
                    for (int i = 1; i <= ceil((double)(line_thickness - 1) / 2.0); i++) {
                        for (int f = 1; f >= -1; f -= 2) {
                            // if (filled) this->pixels[xv][yv + (i * f)] = Pixel((int)xv, (int)yv + (i * f), this->color);
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
                            // if (filled) this->pixels[xv + (i * f)][yv] = Pixel((int)xv + (i * f), (int)yv, this->color);
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


    std::vector<std::vector<Pixel> >().swap(this->pixels);
    // if (this->filled) {
    //     this->flood_fill(this->get_representative_point(), this->color, canvas);
    // }

    return;
}


Uint32 GeoDraw::Pixel::get_uint() {
    Uint32 rgba = (0 << 24) +
         (color.r << 16) +
         (color.g << 8)  +
         color.b;
    return rgba;
}


Uint32 GeoDraw::Color::get_uint() {
    Uint32 rgba = (0 << 24) +
         (r << 16) +
         (g << 8)  +
         b;
    return rgba;
}


void GeoDraw::Canvas::draw() {
    /*
        @desc: Prints the shapes in the canvas to the screen
        @params: none
        @return: void
    */

    // size and position coordinates in the right quad
    cout << "A" << endl;
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

    cout << "A" << endl;

    for (int i = 0; i < outlines.size(); i++) {
        // cout << "Rasterizing " << i << endl;
        outlines[i].rasterize(*this);
    }

    cout << "A" << endl;

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

    // initialize window
    SDL_Event event;
    SDL_Window* window = SDL_CreateWindow("Drawing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, x, y, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, x, y);

    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_UpdateTexture(texture, NULL, background, x * sizeof(Uint32));


    bool quit = false;

    while (!quit) {
        SDL_UpdateTexture(texture, NULL, background, x * sizeof(Uint32));
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


void GeoDraw::Canvas::clear() {
    // @warn: reset background here
    this->outlines = {};
    this->holes = {};
    this->background = new Uint32[x * y];
    memset(background, 255, x * y * sizeof(Uint32));
}


void GeoDraw::Anim::playback() {
    /*
        Play back an animation object
    */
   
    return;
    // std::vector<Uint32*> backgrounds;
    // for (Canvas c : frames) {
    //     memset(c.background, 255, c.x * c.y * sizeof(Uint32));
    //     GeoGerry::bounding_box b = c.get_bounding_box();
    //     c.translate(-b[2], -b[1], true);

    //     double ratio_top = ceil((double) c.box[0]) / (double) (c.x);   // the rounded ratio of top:top
    //     double ratio_right = ceil((double) c.box[3]) / (double) (c.y); // the rounded ratio of side:side
    //     double scale_factor = 1 / ((ratio_top > ratio_right) ? ratio_top : ratio_right); 
    //     c.scale(scale_factor * PADDING);

    //     int px = (int)((double)c.x * (1.0-PADDING) / 2.0), py = (int)((double)c.y * (1.0-PADDING) / 2.0);
    //     c.translate(px, py, false);
    //     c.rasterize_shapes();
    //     backgrounds.push_back(c.background);

    // }

    // Uint32* background = new Uint32[frames[0].x * frames[0].y];
    // memset(background, 255, frames[0].x * frames[0].y * sizeof(Uint32));

    // SDL_Init(SDL_INIT_VIDEO);

    // // initialize window
    // SDL_Event event;
    // SDL_Window* window = SDL_CreateWindow("Drawing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, frames[0].x, frames[0].y, 0);
    // SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    // SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, frames[0].x, frames[0].y);

    // SDL_SetWindowResizable(window, SDL_TRUE);
    // SDL_UpdateTexture(texture, NULL, background, frames[0].x * sizeof(Uint32));
    // SDL_RenderPresent(renderer);
    // SDL_PollEvent(&event);
    // SDL_Delay(200);

    // for (int i = 0; i < backgrounds.size() - 1; i++) {
    //     Uint32* bg = backgrounds[i];
    //     SDL_UpdateTexture(texture, NULL, bg, frames[0].x * sizeof(Uint32));
    //     SDL_PollEvent(&event);
    //     SDL_Delay(delay);
    //     SDL_RenderClear(renderer);
    //     SDL_RenderCopy(renderer, texture, NULL, NULL);
    //     SDL_RenderPresent(renderer);
    // }

    // bool quit = false;

    // while (!quit) {
    //     SDL_UpdateTexture(texture, NULL, backgrounds[backgrounds.size() - 1], frames[0].x * sizeof(Uint32));
    //     SDL_WaitEvent(&event);
    //     if (event.type == SDL_QUIT) quit = true;

    //     SDL_RenderClear(renderer);
    //     SDL_RenderCopy(renderer, texture, NULL, NULL);
    //     SDL_RenderPresent(renderer);
    // }

    // // destroy arrays and SDL objects
    // SDL_DestroyTexture(texture);
    // SDL_DestroyRenderer(renderer);
    // SDL_DestroyWindow( window );
    // SDL_Quit();
}