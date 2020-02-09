#include <SDL2/SDL.h>
#include <stdio.h>
#include <iostream>
#include "../include/shape.hpp"

using namespace std;

const int SCREEN_PIXEL_WIDTH = 640;
const int SCREEN_PIXEL_HEIGHT = 480;

Uint32* pix_array(coordinate_set shape, int x, int y);
coordinate_set connect_dots(coordinate_set shape);
void destroy_window(SDL_Window* window);