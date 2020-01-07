#include "../include/shape.hpp"
#include "../include/gui.hpp"

// functions for modifying shapes

Shape expand_border(Shape shape) {
    vector<vector<int> > expanded;
    Shape expanded_shape = Shape(expanded);
    return expanded_shape;
}

vector<double> center(Shape shape) {
    vector<double> coords = { (float) shape.border[0][0], (float) shape.border[0][1] };
    
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

Shape::Shape(vector<vector<int> > shape) {
    border = shape;
}

// overload constructor for adding id
Shape::Shape(vector<vector<int> > shape, string id) {
    border = shape;
    shape_id = id;
}

void Shape::draw() {
    
    SDL_Event event;
    SDL_Window * window = create_window();
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture * texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 640, 480);
    
    Uint32 * pixels = new Uint32[640 * 480];
    memset(pixels, 255, 640 * 480 * sizeof(Uint32));
    
    bool quit = false;

    while (!quit) {
        SDL_UpdateTexture(texture, NULL, pixels, 640 * sizeof(Uint32));
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
}