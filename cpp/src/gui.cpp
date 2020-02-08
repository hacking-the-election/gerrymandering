// A file for all the sdl functions for various
//  gui apps, functions and tests

#include "../include/gui.hpp"
#include "../include/shape.hpp"

Uint32* pix_array(vector<vector<float> > shape, int x, int y) {
    // creates and returns a pixel array from an array of coordinates

    Uint32 * pixels = new Uint32[x * y];           // new blank pixel array
    memset(pixels, 255, x * y * sizeof(Uint32));   // fill pixel array with white
    int total = (x * y) - 1;                       // last index in pixel array;

    for (vector<float> coords : shape) {
        // locate the coordinate, set it to black
        int start = (int)((int)coords[1] * x - (int)coords[0]);
        pixels[total - start] = 0;
    }

    // return array
    return pixels;
}

vector<vector<float> > connect_dots(vector<vector<float> > shape) {
    vector<vector<float> > newShape;

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
            // newShape.push_back({(float)x + 1, (float)y + 1});
            // newShape.push_back({(float)x + 1, (float)y});
            // newShape.push_back({(float)x, (float)y + 1});
            x += xinc;
            y += yinc;
        }
    }
    
    return newShape;
}

void Shape::draw() {

    int dim[2] = {900, 900}; // the size of the SDL window

    // prepare array of coordinates to be drawn
    vector<float> bounding_box = normalize_coordinates(this);
    vector<vector<float> > shape = resize_coordinates(bounding_box, this->border, dim[0], dim[1]);
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
    vector<vector<float> > b;

    for (Precinct p : state_precincts) {
        for (vector<float> c : p.border) {
            b.push_back(c);
        }
    }

    Shape test(b);
    test.draw();
}

SDL_Window* create_window(int x, int y) {

    SDL_Window* window = NULL;
    SDL_Surface* screenSurface = NULL;

    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
    }
    else {
        //Create window
        window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, x, y, SDL_WINDOW_SHOWN );
    
        if (window == NULL) {
            cout << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        }
        else {
            screenSurface = SDL_GetWindowSurface( window );
            SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );
            SDL_UpdateWindowSurface( window );
        }
    }
    return window;
}

void destroy_window(SDL_Window* window) {
    SDL_DestroyWindow( window );
    SDL_Quit();
}

// void setPixel(int x, int y, int color, SDL_Surface* screen) {
//     Uint8 *row8;
//     Uint16 *row16;
//     Uint32 *row32;

//     if (x < 0 || x >= SCREEN_PIXEL_WIDTH || y < 0 || y >= SCREEN_PIXEL_HEIGHT) {
//         cout << "Error: out of bounds at " << x << ", " << y << endl;
//         return; // Don’t allow overwriting boundaries of the screen

//     }
    
//     switch (bytes_per_pixel) {
// 	    case 1:
// 		    row8 = (Uint8 *) (screen->pixels + y * pitch + x * bytes_per_pixel);
//             *row8 = (Uint8) color;
//             break;
// 	    case 2:
// 		    row16 = (Uint16 *) (screen->pixels + y * pitch + x * bytes_per_pixel);
//             *row16 = (Uint16) color;
//         break;

// 	    case 4:
// 		    row32 = (Uint32 *) (screen->pixels + y * pitch + x * bytes_per_pixel);
//             *row32 = (Uint32) color;
//         break;
//     }
// }