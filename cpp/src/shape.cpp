#include "../include/shape.hpp"
#include "../include/gui.hpp"
#include <math.h>

// functions for modifying shapes

Shape expand_border(Shape shape) {
    vector<vector<float> > expanded;
    Shape expanded_shape = Shape(expanded);
    return expanded_shape;
}

vector<double> center(Shape shape) {
    vector<double> coords = { shape.border[0][0], shape.border[0][1] };
    
    for ( int i = 1; i < shape.border.size(); i++ ) {
        coords[0] += shape.border[i][0];
        coords[1] += shape.border[i][1];
    }

    coords[0] /= shape.border.size();
    coords[1] /= shape.border.size();

    return coords;
}

double area(Shape shape) {
    double area = 0;
    int points = shape.border.size() - 1;

    for ( int i = 0; i < shape.border.size(); i++ ) {
        area += (shape.border[points][0] + shape.border[i][0]) * (shape.border[points][1] - shape.border[i][1]);
        points = i;
    }

    return (area / 2);
}

double Precinct::get_ratio() {
    // retrieve ratio from precinct
    return dem / (dem + rep);
    // return rep / (dem + rep);
}

vector<int> Precinct::voter_data() {
    return {dem, rep};
}

Shape::Shape(vector<vector<float> > shape) {
    border = shape;
}

// overload constructor for adding id
Shape::Shape(vector<vector<float> > shape, string id) {
    border = shape;
    shape_id = id;
}

vector<float> normalize_coordinates(Shape* shape) {
    // loop through
    float top = shape->border[0][1], 
        bottom = shape->border[0][1], 
        left = shape->border[0][0], 
        right = shape->border[0][0];

    for (vector<float> coord : shape->border) {
        top = (coord[1] > top)? coord[1] : top;
        bottom = (coord[1] < bottom)? coord[1] : bottom;
        left = (coord[0] < left)? coord[0] : left;
        right = (coord[0] > right)? coord[0] : right;
    }

    for (float i = 0; i < shape->border.size(); i++) {
        shape->border[i][0] += (0 - left);
        shape->border[i][1] += (0 - bottom);
    }

    // normalize the bounding box, too
    top += (0 - bottom);
    right += (0 - left);
    bottom = 0;
    left = 0;

    // cout << "top " << top << ", bottom " << bottom << ", left " << left << ", right " << right << endl;
    vector<float> m = {top, bottom, left, right};
    return m;
}

vector<vector<float> > resize_coordinates(vector<float> bounding_box, vector<vector<float> > shape, int screenX, int screenY) {
    float ratioTop = ceil((float) bounding_box[0]) / (float) (screenX);
    float ratioRight = ceil((float) bounding_box[3]) / (float) (screenY);
    float scaleFactor = floor(1 / ((ratioTop > ratioRight) ? ratioTop : ratioRight)); 

    // find out which axis need to be made smaller,
    // find out which one needs to be smallest
    
    for (int i = 0; i < bounding_box.size(); i++) {
        bounding_box[i] *= scaleFactor;
    }

    for ( int i = 0; i < shape.size(); i++ ) {
        shape[i][0] *= scaleFactor;
        shape[i][1] *= scaleFactor;        
    }

    return shape;
}

Uint32* pix_array(vector<vector<float> > shape, int x, int y) {
    Uint32 * pixels = new Uint32[x * y];
    memset(pixels, 255, x * y * sizeof(Uint32));
    
    int total = (x * y) - 1;

    for (vector<float> coords : shape) {
        // cout << "drawing " << coords[0] << ", " << coords[1]  << " at " << coords[0] * x + coords[1] << endl;
        int start = (int)(coords[1] * x - coords[0]);
        pixels[total - start] = 0;
    }

    return pixels;
}

void Shape::draw() {
    int dim[2] = {900, 900};

    // prepare coordinates for pixel array
    vector<float> bounding_box = normalize_coordinates(this);
    vector<vector<float> > shape = resize_coordinates(bounding_box, this->border, dim[0], dim[1]);
    for (vector<float> coord : shape) {
        cout << "[" << coord[0] << ", " << coord[1] << "], ";
    }
    // write coordinates to pixel array
    Uint32 * pixels = pix_array(shape, dim[0], dim[1]);

    // initialize window
    SDL_Event event;
    SDL_Window * window = SDL_CreateWindow("Shape",
    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dim[0], dim[1], 0);
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture * texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, dim[0], dim[1]);

    bool quit = false;

    while (!quit) {
        SDL_UpdateTexture(texture, NULL, pixels, dim[0] * sizeof(Uint32));
        SDL_WaitEvent(&event);
        // SDL_Delay(500);
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
        }
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    // cleanup
    delete[] pixels;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    destroy_window(window);


    // bool leftMouseButtonDown = false;
    // bool quit = false;
    // SDL_Event event;

    // SDL_Init(SDL_INIT_VIDEO);

    // SDL_Window * window = SDL_CreateWindow("Shape",
    //     SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dim[0], dim[1], 0);

    // SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
    // SDL_Texture * texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, dim[0], dim[1]);
    // Uint32 * pixels = new Uint32[640 * 480];

    // memset(pixels, 255, 640 * 480 * sizeof(Uint32));

    // while (!quit)
    // {
    //     SDL_UpdateTexture(texture, NULL, pixels, 640 * sizeof(Uint32));
    //     SDL_WaitEvent(&event);

    //     switch (event.type)
    //     {
    //     case SDL_MOUSEBUTTONUP:
    //         if (event.button.button == SDL_BUTTON_LEFT)
    //             leftMouseButtonDown = false;
    //         break;
    //     case SDL_MOUSEBUTTONDOWN:
    //         if (event.button.button == SDL_BUTTON_LEFT)
    //             leftMouseButtonDown = true;
    //     case SDL_MOUSEMOTION:
    //         if (leftMouseButtonDown)
    //         {
    //             int mouseX = event.motion.x;
    //             int mouseY = event.motion.y;
    //             pixels[mouseY * 640 + mouseX] = 0;
    //         }
    //         break;
    //     case SDL_QUIT:
    //         quit = true;
    //         break;
    //     }

    //     SDL_RenderClear(renderer);
    //     SDL_RenderCopy(renderer, texture, NULL, NULL);
    //     SDL_RenderPresent(renderer);
    // }

    // delete[] pixels;
    // SDL_DestroyTexture(texture);
    // SDL_DestroyRenderer(renderer);
    // SDL_DestroyWindow(window);
    // SDL_Quit();

    // // return 0;
}