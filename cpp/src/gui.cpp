/*=======================================
 gui.cpp:                  k-vernooy
 last modified:                Sun, Feb 9
 
 A file for all the sdl functions for
 various gui apps, functions and tests
========================================*/

#include "../include/gui.hpp"
#include "../include/geometry.hpp"

Uint32* pix_array(coordinate_set shape, int x, int y) {

    /* 
        creates and returns a pixel array 
        from an vector of floating point coordinates
    */

    Uint32 * pixels = new Uint32[x * y];           // new blank pixel array
    memset(pixels, 255, x * y * sizeof(Uint32));   // fill pixel array with white
    int total = (x * y) - 1;                       // last index in pixel array;

    for (coordinate coords : shape) {
        // locate the coordinate, set it to black
        int start = (int)((int)coords[1] * x - (int)coords[0]);
        pixels[total - start] = 0;
    }

    // return array
    return pixels;
}

coordinate_set connect_dots(coordinate_set shape) {
    coordinate_set newShape;

    int dx, dy, p, x, y, x0, x1, y0, y1;

    for (int i = 0; i < shape.size() - 1; i++) {

        x0 = (int) shape[i][0];
        y0 = (int) shape[i][1];

        if ( i != shape.size() - 1) {
            x1 = (int) shape[i + 1][0];
            y1 = (int) shape[i + 1][1];
        }
        else {
            x1 = (int) shape[0][0];
            y1 = (int) shape[0][1];
        }

        dx = x1 - x0;
        dy = y1 - y0;

        int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

        float xinc = dx / (float) steps;
        float yinc = dy / (float) steps;

        float x = x0;
        float y = y0;

        for (int i = 0; i <= steps; i++) {
            newShape.push_back({(float)x, (float)y});
            newShape.push_back({(float)x + 1, (float)y + 1});
            newShape.push_back({(float)x + 1, (float)y});
            newShape.push_back({(float)x, (float)y + 1});
            x += xinc;
            y += yinc;
        }
    }
    
    return newShape;
}

void destroy_window(SDL_Window* window) {
    
    // cleanup for sdl windows

    SDL_DestroyWindow( window );
    SDL_Quit();
}

void Shape::draw() {
    
    /*
        open an SDL window, create a pixel array 
        with the shape's geometry, and print it to the window
    */
    
    int dim[2] = {900, 900}; // the size of the SDL window

    // prepare array of coordinates to be drawn
    bounding_box box = normalize_coordinates(this);
    coordinate_set shape = resize_coordinates(box, this->border, dim[0], dim[1]);
    shape = connect_dots(shape);

    // write coordinates to pixel array
    Uint32 * pixels = pix_array(shape, dim[0], dim[1]);

    // initialize window
    SDL_Event event;
    SDL_Window * window = SDL_CreateWindow("Shape", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dim[0], dim[1], 0);
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture * texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, dim[0], dim[1]);
    SDL_SetWindowResizable(window, SDL_TRUE);
    bool quit = false;

    while (!quit) {

        // write current array to screen
        SDL_UpdateTexture(texture, NULL, pixels, dim[0] * sizeof(Uint32));

        // wait for screen to quit
        SDL_WaitEvent(&event);

        if (event.type == SDL_QUIT)
            quit = true;

        // update the SDL renderer
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    // destroy arrays and SDL objects
    delete[] pixels;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    destroy_window(window);
}

void State::draw() {
    // combine precincts into single array, draw array
    coordinate_set b;

    for (Precinct p : state_precincts) {
        for (coordinate c : p.border) {
            b.push_back(c);
        }
    }

    Shape test(b);
    test.draw();
}
