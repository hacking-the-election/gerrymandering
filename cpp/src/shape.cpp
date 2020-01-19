/*=======================================
 shape.cpp:                     k-vernooy
 last modified:               Sun, Jan 19
 
 Definition of methods for shapes, 
 precincts, districts and states. Basic
 calculation, area, and drawing - no
 algorithmic specific methods.
========================================*/

#include "../include/shape.hpp"   // class definitions
#include "../include/gui.hpp"     // for the draw function
#include <math.h>                 // for rounding functions


vector<double> Shape::center() {
    // returns the average {x,y} of a shape

    vector<double> coords // initialize with first values
        = { border[0][0], border[0][1] }; 
    
    // loop and add x and y to respective sums
    for ( int i = 1; i < border.size(); i++ ) {
        coords[0] += border[i][0];
        coords[1] += border[i][1];
    }

    // divide by number of elements to average
    coords[0] /= border.size();
    coords[1] /= border.size();

    return coords; // return averages
}

double Shape::area() {
    // returns the area of a shape

    double area = 0;
    int points = border.size() - 1; // last index of border

    for ( int i = 0; i < border.size(); i++ ) {
        area += (border[points][0] + border[i][0]) * (border[points][1] - border[i][1]);
        points = i;
    }

    return (area / 2);
}

double Precinct::get_ratio() {
    // retrieve ratio from precinct
    return dem / (dem + rep);
}

vector<int> Precinct::voter_data() {
    // get vector of voting data
    return {dem, rep};
}

vector<float> normalize_coordinates(Shape* shape) {
    /*
        returns a normalized bounding box,
        and modifies shape object's 
        coordinates to move it to quadrant 1
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

vector<vector<float> > resize_coordinates(vector<float> bounding_box, vector<vector<float> > shape, int screenX, int screenY) {
    // scales an array of coordinates to fit 
    // on a screen of dimensions {screenX, screenY}
    
    float ratioTop = ceil((float) bounding_box[0]) / (float) (screenX);   // the rounded ratio of top:top
    float ratioRight = ceil((float) bounding_box[3]) / (float) (screenY); // the rounded ratio of side:side
    
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

Uint32* pix_array(vector<vector<float> > shape, int x, int y) {
    // creates and returns a pixel array from an array of coordinates

    Uint32 * pixels = new Uint32[x * y];           // new blank pixel array
    memset(pixels, 255, x * y * sizeof(Uint32));   // fill pixel array with white
    int total = (x * y) - 1;                       // last index in pixel array;

    for (vector<float> coords : shape) {
        // locate the coordinate, set it to black
        int start = (int)(coords[1] * x - coords[0]);
        pixels[total - start] = 0;
    }

    // return array
    return pixels;
}

vector<vector<float> > connect_dots(vector<vector<float> > shape) {
    vector<vector<float> > newShape;

    for (int i = 0; i < shape.size(); i++) {
        int p1x = (int) shape[i][0];
        int p1y = (int) shape[i][1];
        int p2x, p2y;

        if ( i != shape.size() - 1) {
            p2x = (int) shape[i + 1][0];
            p2y = (int) shape[i + 1][1];
        }
        else {
            p2x = (int) shape[0][0];
            p2y = (int) shape[0][1];
        }

        int over = p2x - p1x;
        int up = p2y - p1y;

        cout << "\n\n";

        cout << "Got here: {" << p1x << ", " << p1y << "}, {" << p2x << ", " << p2y << "}" << endl;
        cout << "over " << over << ", up " << up << endl;
        newShape.push_back({(float)p1x, (float)p1y});
        int p3x = p1x, p3y = p1y;

        if (abs(over) >= abs(up)) {
            // how many times we need to go over
            int mult;
            if (up == 0)
                up = 1;
            if (over == 0 || up == 0)
                mult = 1;
            else
                mult = ceil(abs(over / up));
        
            for ( int x = 0; x < abs(up); x++) {
                for ( int y = 0; y < mult; y++) {
                    
                    if (over > 0)
                        p3x++;
                    else if (over < 0)
                        p3x--;

                    newShape.push_back({(float) p3x, (float) p3y});
                }
                // go up once
                if ( up > 0) 
                    p3y++;
                else if (up < 0)
                    p3y--;

            }
        }
        else {
            // how many times we need to go over
            int mult;

            if (over == 0)
                over = 1;
            if (over == 0 || up == 0)
                mult = 1;
            else
                mult = ceil(abs(up / over));
        
            cout << "Starting point: " << p3x << ", " << p3y;

            for ( int x = 0; x < abs(over); x++) {
                // go up once
                if ( over > 0) 
                    p3x++;
                else if (over < 0)
                    p3x--;

                for ( int y = 0; y < mult; y++) {
                    if (up > 0)
                        p3y++;
                    else if (up < 0)
                        p3y--;
                    cout << "at " << p3x << ", " << p3y << endl;
                    newShape.push_back({(float) p3x, (float) p3y});
                }
                

            }
        }

        newShape.push_back({(float)p2x, (float)p2y});
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