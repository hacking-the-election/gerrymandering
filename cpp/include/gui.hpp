#include <SDL2/SDL.h>
#include <stdio.h>
#include <iostream>

using namespace std;

const int SCREEN_PIXEL_WIDTH = 640;
const int SCREEN_PIXEL_HEIGHT = 480;

SDL_Window* create_window();
void destroy_window(SDL_Window* window);