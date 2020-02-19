#include <SDL2/SDL.h>
#include <stdio.h>
#include <iostream>
#include "../include/shape.hpp"

const int SCREEN_PIXEL_WIDTH = 640;
const int SCREEN_PIXEL_HEIGHT = 480;

Uint32* pix_array(GeoGerry::coordinate_set shape, int x, int y);
GeoGerry::coordinate_set connect_dots(GeoGerry::coordinate_set shape);
void destroy_window(SDL_Window* window);