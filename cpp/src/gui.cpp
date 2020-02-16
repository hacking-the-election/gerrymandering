/*=======================================
 gui.cpp:                  k-vernooy
 last modified:                Sun, Feb 9
 
 A file for all the sdl functions for
 various gui apps, functions and tests
========================================*/

#include "../include/gui.hpp"
#include "../include/geometry.hpp"

bounding_box normalize_coordinates(Shape* shape) {

    /*
        returns a normalized bounding box, and modifies 
        shape object's coordinates to move it to Quadrant I
    */

    // set dummy extremes
    float top = shape->border[0][1], 
        bottom = shape->border[0][1], 
        left = shape->border[0][0], 
        right = shape->border[0][0];

    // loop through and find actual corner using ternary assignment
    for (vector<float> coord : shape->border) {
        top = (coord[1] > top)? coord[1] : top;
        bottom = (coord[1] < bottom)? coord[1] : bottom;
        left = (coord[0] < left)? coord[0] : left;
        right = (coord[0] > right)? coord[0] : right;
    }

    // add to each coordinate to move it to quadrant 1
    for (float i = 0; i < shape->border.size(); i++) {
        shape->border[i][0] += (0 - left);
        shape->border[i][1] += (0 - bottom);
    }

    // normalize the bounding box too
    top += (0 - bottom);
    right += (0 - left);
    bottom = 0;
    left = 0;

    return {top, bottom, left, right}; // return bounding box
}

coordinate_set resize_coordinates(bounding_box box, coordinate_set shape, int screenX, int screenY) {
    // scales an array of coordinates to fit 
    // on a screen of dimensions {screenX, screenY}
    
    float ratioTop = ceil((float) box[0]) / (float) (screenX);   // the rounded ratio of top:top
    float ratioRight = ceil((float) box[3]) / (float) (screenY); // the rounded ratio of side:side
    
    // find out which is larger and assign it's reciporical to the scale factor
    float scaleFactor = floor(1 / ((ratioTop > ratioRight) ? ratioTop : ratioRight)); 

    // dilate each coordinate in the shape
    for ( int i = 0; i < shape.size(); i++ ) {
        shape[i][0] *= scaleFactor;
        shape[i][1] *= scaleFactor;        
    }

    // return scaled coordinates
    return shape;
}

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
    cout << "a" << endl;
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


bounding_box normalize_coordinates(Multi_Shape* multi_shape) {
    /*
        returns a normalized bounding box, and modifies 
        shape object's coordinates to move it to Quadrant I
    */

    // set dummy extremes
    float top = multi_shape->border[0].border[0][1], 
        bottom = multi_shape->border[0].border[0][1], 
        left = multi_shape->border[0].border[0][0], 
        right = multi_shape->border[0].border[0][0];
    cout << "bottom: " << bottom << ", left:" << left << endl;

    // loop through and find actual corner using ternary assignment
    for (int i = 0; i < multi_shape->border.size(); i++) {
        for (int x = 0; x < multi_shape->border[i].border.size(); x++) {
            top = (multi_shape->border[i].border[x][1] > top) ? multi_shape->border[i].border[x][1] : top;
            bottom = (multi_shape->border[i].border[x][1] < bottom)? multi_shape->border[i].border[x][1] : bottom;
            left = (multi_shape->border[i].border[x][0] < left)? multi_shape->border[i].border[x][0] : left;
            right = (multi_shape->border[i].border[x][0] > right)? multi_shape->border[i].border[x][0] : right;
        }
    }

    cout << "bottom: " << bottom << ", left:" << left << endl;
    for (int i = 0; i < multi_shape->border.size(); i++) {
        // add to each coordinate to move it to quadrant 1
        for (int x = 0; x < multi_shape->border[i].border.size(); x++) {
            multi_shape->border[i].border[x][0] += (0 - left);
            multi_shape->border[i].border[x][1] += (0 - bottom);
        }
    }

    // normalize the bounding box too
    top += (0 - bottom);
    right += (0 - left);
    bottom = 0;
    left = 0;

    return {top, bottom, left, right}; // return bounding box
}



vector<Shape> resize_coordinates(bounding_box box, vector<Shape> shapes, int screenX, int screenY) {
    // scales an array of coordinates to fit 
    // on a screen of dimensions {screenX, screenY}
    
    float ratioTop = ceil((float) box[0]) / (float) (screenX);   // the rounded ratio of top:top
    float ratioRight = ceil((float) box[3]) / (float) (screenY); // the rounded ratio of side:side
    
    // find out which is larger and assign it's reciporical to the scale factor
    float scaleFactor = floor(1 / ((ratioTop > ratioRight) ? ratioTop : ratioRight)); 

    // dilate each coordinate in the shape
    for (int i = 0; i < shapes.size(); i++) {
        for ( int x = 0; x < shapes[i].border.size(); x++ ) {
            shapes[i].border[x][0] *= scaleFactor;
            shapes[i].border[x][1] *= scaleFactor;        
        }
    }

    // return scaled coordinates
    return shapes;
}

coordinate_set connect_dots(vector<Shape> shapes) {
    /*
        Given an array of shapes, calculates the
        pixels in a sized matrix that connect the 
        vertices of the shape
    */

    coordinate_set newShape;

    // loop through shapes
    for (int j = 0; j < shapes.size(); j++) {
        int dx, dy, p, x, y, x0, x1, y0, y1;

        for (int i = 0; i < shapes[j].border.size() - 1; i++) {

            x0 = (int) shapes[j].border[i][0];
            y0 = (int) shapes[j].border[i][1];

            if ( i != shapes[j].border.size() - 1) {
                x1 = (int) shapes[j].border[i + 1][0];
                y1 = (int) shapes[j].border[i + 1][1];
            }
            else {
                x1 = (int) shapes[j].border[0][0];
                y1 = (int) shapes[j].border[0][1];
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
    }

    return newShape;
}

void Multi_Shape::draw() {
    // combine precincts into single array, draw array

    int dim[2] = {900, 900}; // the size of the SDL window

    // prepare array of coordinates to be drawn
    bounding_box box = normalize_coordinates(this);
    cout << this->border[0].border[0][0] << ", " << this->border[0].border[0][1] << endl;
    vector<Shape> shapes = resize_coordinates(box, this->border, dim[0], dim[1]);
    for (Shape s : shapes) {
        for (coordinate c : s.border) {
            cout << c[0] << ", " << c[1] << endl;
        }
    }
    coordinate_set shape = connect_dots(shapes);
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