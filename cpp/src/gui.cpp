// A file for all the sdl functions for various
//  gui apps, functions and tests
#include <SDL2/SDL.h>
#include <stdio.h>
#include <iostream>

using namespace std;

//Screen dimension constants
const int SCREEN_PIXEL_WIDTH = 640;
const int SCREEN_PIXEL_HEIGHT = 480;

int main( int argc, char* args[] )
{
    //The window we'll be rendering to
    SDL_Window* window = NULL;
    
    //The surface contained by the window
    SDL_Surface* screenSurface = NULL;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }

    else
    {
        //Create window
        window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_PIXEL_WIDTH, SCREEN_PIXEL_HEIGHT, SDL_WINDOW_SHOWN );
        if( window == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get window surface
            screenSurface = SDL_GetWindowSurface( window );

            //Fill the surface white
            SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );
            
            //Update the surface
            SDL_UpdateWindowSurface( window );

            //Wait two seconds
            SDL_Delay( 2000 );
        }
    }

    //Destroy window
    SDL_DestroyWindow( window );

    //Quit SDL subsystems
    SDL_Quit();

    return 0;
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