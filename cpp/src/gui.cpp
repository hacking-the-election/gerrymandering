// A file for all the sdl functions for various
//  gui apps, functions and tests

#include "../include/gui.hpp"

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