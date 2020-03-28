/*=======================================
 canvas.cpp:                    k-vernooy
 last modified:               Tue, Feb 18
 
 A file for all the canvas functions for
 various gui apps, tests, functions and
 visualizations.
========================================*/

#include "../include/canvas.hpp"
#include "../include/geometry.hpp"

int RECURSION_STATE = 0;
double PADDING = (3.0/4.0);

GeoGerry::coordinate i_average(GeoGerry::bounding_box n) {
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


void GeoDraw::Canvas::flood_fill_util(GeoGerry::coordinate coord, Color c1, Color c2) {
    RECURSION_STATE++;
    // if (RECURSION_STATE > 50000) return;
 
    if (coord[0] < 0 || coord[0] > x || coord[1] < 0 || coord[1] > y) return;
    if (this->get_pixel({coord[0], coord[1]}).color != c1) return;
    this->pixels.push_back(Pixel(coord[0], coord[1], c2));

    flood_fill_util({coord[0] + 1, coord[1]}, c1, c2);
    flood_fill_util({coord[0] - 1, coord[1]}, c1, c2);
    flood_fill_util({coord[0], coord[1] + 1}, c1, c2);
    flood_fill_util({coord[0], coord[1] - 1}, c1, c2);

    return;
}


GeoDraw::Pixel GeoDraw::Canvas::get_pixel(GeoGerry::coordinate c) {
    for (Pixel p : pixels) {
        if (p.x == c[0] && p.y == c[1]) return p;
    }

    return (Pixel(-1, -1, Color(255,255,255)));
}

void GeoDraw::Canvas::flood_fill(GeoGerry::coordinate coord, Color c) {
    Color co = this->get_pixel(coord).color;
    this->flood_fill_util(coord, co, c);
    return;
}


void GeoDraw::Canvas::fill_shapes() {
    for (Outline outline : outlines) {
        if (outline.filled) {
            this->flood_fill(outline.get_representative_point(), outline.color);
        }
    }

    return;
}


GeoGerry::coordinate GeoDraw::Outline::get_representative_point() {
    // GeoGerry::bounding_box box = get_bounding_box(*this);
    // GeoGerry::coordinate c = i_average(box);

    // int m = 4;

    // while (!point_in_ring(c, this->border)) {
    //     //
    //     for (int i = 0; i < m; i++) {

    //     }

    //     m *= 4;
    // }

    return {150, 150};
    // return {0,0};
}


void GeoDraw::Canvas::rasterize_edges() {
    /*
        @desc:
            Converts coordinates into pixel array, writes
            to the pixels array contained in the object
            
        @params: none
        @return: void
    */

    int dx, dy, x0, x1, y0, y1;

    for (Outline o : outlines) {
        for (GeoGerry::segment s : o.border.get_segments()) {
            x0 = s[0];
            y0 = s[1];
            x1 = s[2];
            y1 = s[3];

            // Pixel p0(x0, y0, o.color);
            // this->pixels.push_back(p0);
            dx = x1 - x0;
            dy = y1 - y0;

            int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

            double xinc = (double) dx / (double) steps;
            double yinc = (double) dy / (double) steps;

            double xv = (double) x0;
            double yv = (double) y0;

            for (int i = 0; i <= steps; i++) {
                for (int i = -(o.line_thickness - 1); i <= (o.line_thickness - 1); i++) {
                    for (int j = -(o.line_thickness - 1); j <= (o.line_thickness - 1); j++) {
                        Pixel p((int)xv + i, (int)yv + j, o.color);
                        this->pixels.push_back(p);
                    }
                }

                xv += xinc;
                yv += yinc;
            }
        }
    }
}


void GeoDraw::Canvas::rasterize_shapes() {
    /*
        @desc:
            scales an array of coordinates to fit on a screen
            of dimensions {x, y}, and determines pixel values
            and placements

        @params: none
        @return: void
    */

    this->rasterize_edges();
    this->fill_shapes();

    for (Pixel p : pixels) {
        int total = (x * y) - 1;
        int start = p.y * x - p.x;

        if (total - start < x * y && total - start >= 0) 
            this->background[total - start] = p.get_uint();
    }

    // fill_shapes();

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

    // size and position coordinates in the right wuat
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

    rasterize_shapes();

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
    
    std::vector<Uint32*> backgrounds;
    for (Canvas c : frames) {
        memset(c.background, 255, c.x * c.y * sizeof(Uint32));
        GeoGerry::bounding_box b = c.get_bounding_box();
        c.translate(-b[2], -b[1], true);

        double ratio_top = ceil((double) c.box[0]) / (double) (c.x);   // the rounded ratio of top:top
        double ratio_right = ceil((double) c.box[3]) / (double) (c.y); // the rounded ratio of side:side
        double scale_factor = 1 / ((ratio_top > ratio_right) ? ratio_top : ratio_right); 
        c.scale(scale_factor * PADDING);

        int px = (int)((double)c.x * (1.0-PADDING) / 2.0), py = (int)((double)c.y * (1.0-PADDING) / 2.0);
        c.translate(px, py, false);
        c.rasterize_shapes();
        backgrounds.push_back(c.background);

    }

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
    SDL_PollEvent(&event);
    SDL_Delay(200);

    for (int i = 0; i < backgrounds.size() - 1; i++) {
        Uint32* bg = backgrounds[i];
        SDL_UpdateTexture(texture, NULL, bg, frames[0].x * sizeof(Uint32));
        SDL_PollEvent(&event);
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