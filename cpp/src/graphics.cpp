/*=======================================
 canvas.cpp:                    k-vernooy
 last modified:               Mon, Jun 22
 
 A file for all the canvas functions for
 various gui apps, tests, functions and
 visualizations.
========================================*/

#include "../include/hte.h"

using namespace hte;
using namespace std;

namespace fs = boost::filesystem;


int RECURSION_STATE = 0;
double PADDING = (15.0/16.0);


Outline hte::ToOutline(LinearRing r) {
    Outline o(r);
    o.style().fill(RgbColor(-1, -1, -1)).outline(RgbColor(0,0,0)).thickness(1);
    return o;
}


vector<Outline> hte::ToOutline(State state) {
    vector<Outline> outlines;
    for (Precinct p : state.precincts) {
        Outline o(p.hull);
        double ratio = 0.5;
        if (!(p.voterData[PoliticalParty::Democrat] == 0 && p.voterData[PoliticalParty::Republican] == 0)) {
            ratio = static_cast<double>(p.voterData[PoliticalParty::Democrat]) / static_cast<double>(p.voterData[PoliticalParty::Democrat] + p.voterData[PoliticalParty::Republican]);
        }

        o.style().outline(InterpolateRgb(RgbColor(255,0,0), RgbColor(0,0,255), ratio)).thickness(1.0).fill(InterpolateRgb(RgbColor(255,0,0), RgbColor(0,0,255), ratio));
        outlines.push_back(o);
    }

    return outlines;
}


OutlineGroup hte::ToOutline(MultiPolygon& mp, double v, bool abs_quant) {
    OutlineGroup os;
    RgbColor fill;

    if (!abs_quant) {
        if (v < 0) fill = InterpolateRgb(RgbColor(255,255,255), RgbColor(255, 0, 0), abs(v) * 2);
        else fill = InterpolateRgb(RgbColor(255,255,255), RgbColor(0, 0, 255), v * 2);
    }
    else {
        fill = InterpolateRgb(RgbColor(0,0,0), RgbColor(255,255,255), v);
    }

    for (Polygon p : mp.border) {
        Outline o = ToOutline(p.hull);
        o.style().fill(fill).thickness(0);
        os.addOutline(o);
    }
    return os;
}


vector<Outline> hte::ToOutline(Graph& graph) {
    vector<Outline> outlines;
    for (int i = 0; i < graph.vertices.size(); i++) {
        Node node = (graph.vertices.begin() + i).value();
        Outline node_b(GenerateGon(node.precinct->getCentroid(), node.precinct->pop * 2, 50).hull);

        float ratio = 0.5;
        int sum = node.precinct->voterData[PoliticalParty::Democrat] + node.precinct->voterData[PoliticalParty::Republican];
        if (sum <= 0) ratio = 0.5;
        else ratio = (float)node.precinct->voterData[PoliticalParty::Republican] / (float)sum;

        node_b.style().fill(InterpolateRgb(RgbColor(0, 0, 255), RgbColor(255,0,0), (double)ratio))
            .thickness(1).outline(InterpolateRgb(RgbColor(0, 0, 255), RgbColor(255,0,0), (double)ratio));
        
        for (Edge edge : node.edges) {
            if (graph.vertices.find(edge[1]) != graph.vertices.end()) {
                Point2d start = graph.vertices[edge[0]].precinct->getCentroid();
                Point2d end = graph.vertices[edge[1]].precinct->getCentroid();
                Outline o(LinearRing({start, end}));
                o.style().outline(RgbColor(0,0,0)).thickness(0.2).fill(RgbColor(-1,-1,-1));
                // outlines.push_back(o);
            }
        }
        outlines.push_back(node_b);
    }

    return outlines;
}


vector<Outline> hte::ToOutline(Communities& communities) {
    vector<Outline> outlines;
    vector<RgbColor> colors = GenerateColors(communities.size());

    for (int i = 0; i < communities.size(); i++) {
        for (auto& j : communities[i].vertices) {
            Outline o(j.second.precinct->hull);
            o.style().fill(colors[i]).outline(colors[i]).thickness(1);
            outlines.push_back(o);
        }
    }

    return outlines;
} 


vector<OutlineGroup> hte::ToOutlineGroup(Communities& communities) {
    vector<OutlineGroup> districts;
    vector<RgbColor> colors = GenerateColors(communities.size());

    for (int i = 0; i < communities.size(); i++) {
        OutlineGroup og;
        vector<Precinct> precincts = communities[i].shape.precincts;
        vector<Polygon> polys;
        polys.insert(polys.end(), precincts.begin(), precincts.end());

        for (Polygon p : GenerateExteriorBorder(polys).border) {
            Outline o(p.hull);
            o.style().fill(colors[i]).outline(RgbColor(0,0,0)).thickness(1);
            og.addOutline(o);
        }
        districts.push_back(og);
    }

    return districts;
}


OutlineGroup hte::ToOutline(MultiPolygon& mp) {
    OutlineGroup o;
    for (Polygon p : mp.border) {
        o.addOutline(ToOutline(p.hull));
    }
    return o;
}


Style& Style::outline(RgbColor c) {
    // set the outline color
    outline_ = c;
    return *this;
}


Style& Style::thickness(double t) {
    // set the outline thickness
    thickness_ = t;
    return *this;
}


Style& Style::fill(RgbColor c) {
    // set the fill color (RGB)
    fill_ = c;
    return *this;
}


Style& Style::fill(HslColor c) {
    // set the fill color (HSL)
    fill_ = HslToRgb(c);
    return *this;
}


double hte::HueToRgb(double p, double q, double t) {	
    /*
        Convert a hue to a single rgb value
        I honestly forget how this works sorry future me
    */

    if(t < 0) t += 1;
    if(t > 1) t -= 1;
    if(t < 1.0/6.0) return p + (q - p) * 6.0 * t;
    if(t < 1.0/2.0) return q;
    if(t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;
    return p;
}	


RgbColor hte::HslToRgb(HslColor hsl) {
    /*
        @desc: Convert a HslColor object into RgbColor
        @params: `HslColor` hsl: color to convert
        @return: `RgbColor` converted color
    */

    double r, g, b;

    if (hsl.s == 0.0) {
        r = hsl.l;
        g = hsl.l;
        b = hsl.l;
    }
    else {
        double q = (hsl.l < 0.5) ? hsl.l * (1.0 + hsl.s)
            : hsl.l + hsl.s - hsl.l * hsl.s;

        double p = 2.0 * hsl.l - q;

        r = HueToRgb(p, q, hsl.h + 1.0/3.0);
        g = HueToRgb(p, q, hsl.h);
        b = HueToRgb(p, q, hsl.h - 1.0/3.0);
    }

    return RgbColor((r * 255.0), (g * 255.0), (b * 255.0));
}


HslColor hte::RgbToHsl(RgbColor rgb) {
    /*
        @desc: Converts RgbColor object into HslColor
    */

    double r = (double) rgb.r / 255.0;
    double g = (double) rgb.g / 255.0;
    double b = (double) rgb.b / 255.0;
    double max = r, min = r;

    for (double x : {r,g,b}) {
        if (x > max) max = x;
        else if (x < min) min = x;
    }


    double h, s, l = (max + min) / 2.0;

    if (max == min) {
        h = 0.0;
        s = 0.0; // achromatic
    }
    else {
        double d = max - min;
        s = (l > 0.5) ? (d / (2.0 - max - min)) : (d / (max + min));

        if (max == r) h = (g - b) / d + ((g < b) ? 6.0 : 0.0);
        else if (max == g) h = (b - r) / d + 2;
        else h = (r - g) / d + 4;

        h /= 6.0;
    }

    return HslColor(h, s, l);
}


double hte::Lerp(double a, double b, double t) {
    return(a + (b - a) * t);
}


HslColor hte::InterpolateHsl(HslColor hsl1, HslColor hsl2, double interpolator) {	
    return HslColor(
        Lerp(hsl1.h, hsl2.h, interpolator),
        Lerp(hsl1.s, hsl2.s, interpolator),
        Lerp(hsl1.l, hsl2.l, interpolator)
    );
}


RgbColor hte::InterpolateRgb(RgbColor rgb1, RgbColor rgb2, double interpolator) {
    return RgbColor(
        round(Lerp(rgb1.r, rgb2.r, interpolator)),
        round(Lerp(rgb1.g, rgb2.g, interpolator)),
        round(Lerp(rgb1.b, rgb2.b, interpolator))
    );
}


vector<RgbColor> hte::GenerateColors(int n) {
    /*
        @desc: Generates a number of colors blindly (and semi-randomly)
        @params: `int` n: number of colors to generate
        @return: `vector<hte::RgbColor>` list of color objects
    */

    vector<RgbColor> colors;

    int shift = 310;
    for (int i = shift; i < 360 + shift; i += 360 / n) {
        // create and add colors
        colors.push_back(
            HslToRgb(
                HslColor(
                    ((double)(i % 360) / 360.0),
                    (86.0 / 100.0),
                    (72.0 / 100.0)
                )
            )
        );
    }

    return colors;
}


int PixelBuffer::indexFromPosition(int a, int b) {
    if (a >= 0 && b >= 0)
        return ((x * (b - 1)) + a - 1);
    else return (1);
}


void PixelBuffer::setFromPosition(int a, int b, Uint32 value) {
    ar[indexFromPosition(a, b)] = value;
}


Uint32 PixelBuffer::getFromPosition(int a, int b) {
    return ar[indexFromPosition(a, b)];
}


BoundingBox Canvas::getBoundingBox() {
    /*
        @desc:
            returns a bounding box of the internal list of hulls
            (because holes cannot be outside shapes)
        
        @params: none;
        @return: `bounding_box` the superior bounding box of the shape
    */

    if (outlines.size() > 0) {
        // set dummy extremes
        int top = outlines[0].outlines[0].border.border[0].y, 
            bottom = outlines[0].outlines[0].border.border[0].y, 
            left = outlines[0].outlines[0].border.border[0].x, 
            right = outlines[0].outlines[0].border.border[0].x;

        // loop through and find actual corner using ternary assignment
        for (OutlineGroup og : outlines) {
            for (Outline ring : og.outlines) {
                BoundingBox x = ring.border.getBoundingBox();
                if (x[0] > top) top = x[0];
                if (x[1] < bottom) bottom = x[1];
                if (x[2] < left) left = x[2];
                if (x[3] > right) right = x[3];
            }
        }

        box = {top, bottom, left, right};
    }
    else {
        box = {height, 0, 0, width};
    }

    return box; // return bounding box
}


void Canvas::translate(long int tX, long int tY, bool b) {
    // for each outline group
    for (int i = 0; i < outlines.size(); i++) {
        // for each outline
        for (int o = 0; o < outlines[i].outlines.size(); o++) {
            // for each coordinate
            for (int j = 0; j < outlines[i].outlines[o].border.border.size(); j++) {
                // translate coordinate
                outlines[i].outlines[o].border.border[j].x += tX;
                outlines[i].outlines[o].border.border[j].y += tY;
            }
        }
    }

    toDate = false;
    if (b) box = {box[0] + tY, box[1] + tY, box[2] + tX, box[3] + tX};
}


void Canvas::scale(double scaleFactor) {
    // for each outline group
    for (int i = 0; i < outlines.size(); i++) {
        // for each outine
        for (int o = 0; o < outlines[i].outlines.size(); o++) {
            // for each coordinate
            for (int j = 0; j < outlines[i].outlines[o].border.border.size(); j++) {
                // scale by the `scaleFactor`
                outlines[i].outlines[o].border.border[j].x *= scaleFactor;
                outlines[i].outlines[o].border.border[j].y *= scaleFactor;
            }
        }
    }

    // must be re-rasterized
    toDate = false;
}


void hte::DrawLine(PixelBuffer& buffer, Point2d start, Point2d end, RgbColor color, double t) {
    int dx = abs(end.x - start.x), sx = start.x < end.x ? 1 : -1;
    int dy = abs(end.y - start.y), sy = start.y < end.y ? 1 : -1;
    int err = dx - dy, e2, x2, y2;
    float ed = dx + dy == 0 ? 1 : sqrt((float)dx * dx + (float)dy * dy);

    for (t = (t + 1) / 2; ;) {
        // if cval is 0, we want to draw pure color
        double cval = max(0.0, 255 * (abs(err - dx + dy) / ed - t + 1)) / 255.0;
        buffer.setFromPosition(start.x, start.y, color.toUint());
        e2 = err; x2 = start.x;

        if (2 * e2 >= -dx) {
            for (e2 += dy, y2 = start.y; e2 < ed*t && (end.y != y2 || dx > dy); e2 += dx) {
                double cval = max(0.0, 255 * (abs(e2) / ed - t + 1)) / 255.0;
                buffer.setFromPosition(start.x, y2 += sy, color.toUint());
            }
            if (start.x == end.x) break;
            e2 = err; err -= dy; start.x += sx; 
        } 
        if (2 * e2 <= dy) {
            for (e2 = dx - e2; e2 < ed * t && (end.x != x2 || dx < dy); e2 += dy) {
                int cval = max(0.0, 255 * (abs(e2) / ed - t + 1)) / 255.0;
                buffer.setFromPosition(x2 += sx, start.y, color.toUint());
            }
            if (start.y == end.y) break;
            err += dx; start.y += sy; 
        }
    }

    return;
}


void hte::DrawPolygon(PixelBuffer& buffer, LinearRing ring, Style style) {
    // fill polygon
    if (style.fill_.r != -1) {
        vector<EdgeBucket> allEdges;
        for (Segment seg : ring.getSegments()) {
            EdgeBucket bucket;
            bucket.miny = min(seg[1], seg[3]);
            bucket.maxy = max(seg[1], seg[3]);
            bucket.minyX = (bucket.miny == seg[1]) ? seg[0] : seg[2];
            int dy = seg[3] - seg[1];
            int dx = seg[2] - seg[0];

            if (dx == 0) bucket.slope = INFINITY;
            else bucket.slope = (double)dy / (double)dx;

            allEdges.push_back(bucket);
        }

        // this following algorithm can probably get faster
        vector<EdgeBucket> globalEdges = {};

        for (int i = 0; i < allEdges.size(); i++) {
            if (allEdges[i].slope != 0) {
                globalEdges.push_back(allEdges[i]);
            }
        }

        sort(globalEdges.begin(), globalEdges.end());
        if (globalEdges.size() != 0) {
            int scan_line = globalEdges[0].miny;
            vector<EdgeBucket> activeEdges = {};

            for (int i = 0; i < globalEdges.size(); i++) {
                if (globalEdges[i].miny > scan_line) break;
                if (globalEdges[i].miny == scan_line) activeEdges.push_back(globalEdges[i]);
            }

            while (activeEdges.size() > 0) {
                for (int i = 0; i < activeEdges.size(); i += 2) {
                    // draw all points between edges with even parity
                    for (int j = activeEdges[i].minyX; j <= activeEdges[i + 1].minyX; j++) {
                        buffer.setFromPosition(j, buffer.y - scan_line, style.fill_.toUint());
                    }
                }

                scan_line++;

                // Remove any edges from the active edge table for which the maximum y value is equal to the scan_line.
                for (int i = 0; i < activeEdges.size(); i++) {
                    if (activeEdges[i].maxy == scan_line) {
                        activeEdges.erase(activeEdges.begin() + i);
                        i--;
                    }
                    else {
                        activeEdges[i].minyX = (double)activeEdges[i].minyX + (double)(1.0 / activeEdges[i].slope);
                    }
                }


                for (int i = 0; i < globalEdges.size(); i++) {
                    if (globalEdges[i].miny == scan_line) {
                        activeEdges.push_back(globalEdges[i]);
                        globalEdges.erase(globalEdges.begin() + i);
                        i--;
                    }
                }

                sort(activeEdges.begin(), activeEdges.end(), [](EdgeBucket& one, EdgeBucket& two){return one.minyX < two.minyX;});
            }

        }
    }

    // draw polygon outline 
    for (Segment s : ring.getSegments()) {
        DrawLine(buffer, {s[0], buffer.y - s[1]}, {s[2], buffer.y - s[3]}, style.outline_, style.thickness_);
    }
}


Uint32 RgbColor::toUint() {
    // bitshift RGB colors into an ARGB Uint32
    Uint32 argb =
        (255 << 24) +
        (r << 16) +
        (g << 8)  +
        (b);

    return argb;
}


RgbColor RgbColor::fromUint(Uint32 color) {
    // bitshift an ARGB Uint32 into rgb colors
    int t_r = (color >> 16) & 255;
    int t_g = (color >> 8) & 255;
    int t_b = color & 255;

    return RgbColor(t_r, t_g, t_b);
}


void Canvas::clear() {
    /// \warning This method will clear all canvas data.
    this->outlines = {};
    this->pixelBuffer = PixelBuffer(width, height);
    toDate = true;
}


bool Canvas::getBmp(string write_path) {
    // initialize SDL objects for writing to image
    SDL_Event event;
    SDL_Window* window = SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);

    // create SDL texture and renderer from PixelBuffer
    SDL_UpdateTexture(texture, NULL, pixelBuffer.ar, width * sizeof(Uint32));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // create empty RGB surface to hold window data
    SDL_Surface* pScreenShot = SDL_CreateRGBSurface(
        0, width, height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000
    );
    
    // check file path and output
    if (boost::filesystem::exists(write_path + ".bmp")) {
        cout << "File already exists, returning" << endl;
        return false; // failed with error
    }
    
    if (pScreenShot) {
        // read pixels from render target, save to surface
        SDL_RenderReadPixels(
            renderer, NULL, SDL_GetWindowPixelFormat(window),
            pScreenShot->pixels, pScreenShot->pitch
        );

        // Create the bmp screenshot file
        SDL_SaveBMP(pScreenShot, string(write_path + ".bmp").c_str());
    }
    else {
        cout << "Surface not created correctly, returning" << endl;
        return false; // failed with error
    }

    // delete SDL constructs
    SDL_FreeSurface(pScreenShot);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    // written succesfully
    return true;
}


string Outline::getSvg(double scaleFactor) {
    // create svg string with open tag
    string svg = "<path d=\"M";
    // add initial point to start drawing
    svg += to_string(static_cast<double>(border.border[0].x) * scaleFactor) + "," + to_string(static_cast<double>(border.border[0].y) * scaleFactor);

    for (Segment s : border.getSegments()) {
        // add coordinate to `d` tag
        svg += "L";
        svg += to_string(static_cast<double>(s[0]) * scaleFactor) + "," + to_string(static_cast<double>(s[1]) * scaleFactor) + "," 
             + to_string(static_cast<double>(s[2]) * scaleFactor) + "," + to_string(static_cast<double>(s[3]) * scaleFactor);
    }

    // close tag and add inline styling from the `style()` method
    svg += "z\" stroke=\"rgb(" + to_string(style().outline_.r) + "," + to_string(style().outline_.g) + ","
        + to_string(style().outline_.b) + ")\" fill=\"rgb(" + to_string(style().fill_.r) + "," + to_string(style().fill_.g) + ","
        + to_string(style().fill_.b) + ")\" stroke-width=\"" + to_string(style().thickness_) + "\"></path>";

    return svg;
}


string OutlineGroup::getSvg(double scaleFactor) {
    if (outlines.size() == 0) cout << "NO OUTLINES, EXPECT SEGFAULT" << endl;
    string svg = "<path d=\"";

    for (Outline o : outlines) {
        svg += "M " + to_string(static_cast<double>(o.border.border[0].x) * scaleFactor)
              + "," + to_string(static_cast<double>(o.border.border[0].y) * scaleFactor);

        for (Segment s : o.border.getSegments()) {
            svg += "L";
            svg += to_string(static_cast<double>(s[0]) * scaleFactor) + "," + to_string(static_cast<double>(s[1]) * scaleFactor) + "," 
                 + to_string(static_cast<double>(s[2]) * scaleFactor) + "," + to_string(static_cast<double>(s[3]) * scaleFactor);
        }
        svg += "Z ";
    }
    
    svg += "\" stroke=\"rgb(" + to_string(outlines[0].style().outline_.r) + "," + to_string(outlines[0].style().outline_.g) + ","
        + to_string(outlines[0].style().outline_.b) + ")\" fill=\"rgb(" + to_string(outlines[0].style().fill_.r) + "," + to_string(outlines[0].style().fill_.g) + ","
        + to_string(outlines[0].style().fill_.b) + ")\" stroke-width=\"" + to_string(outlines[0].style().thickness_) + "\"></path>";

    return svg;
}


string Canvas::getSvg() {
    /*
        @desc: gets a string representing an svg graphic from a canvas
        @params: none
        @return: string
    */

    string svg = "<svg xmlns=\"http://www.w3.org/2000/svg\" height=\"100%\" width=\"100%\" viewBox=\"0 0 " + to_string(width) + " " + to_string(height) + "\">";
    BoundingBox b = getBoundingBox();
    double ratioTop = ceil((double) this->box[0]) / (double) (height);
    double ratioRight = ceil((double) this->box[3]) / (double) (width);
    double scaleFactor = 1 / ((ratioTop > ratioRight) ? ratioTop : ratioRight); 
    
    for (OutlineGroup o : outlines) {
        svg += o.getSvg(scaleFactor);
    }

    return (svg + "</svg>");
}


bool Canvas::getPnm(string writePath) {
    string file = "P3 " + to_string(width) + " " + to_string(height) + " 255\n";
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            RgbColor c = RgbColor::fromUint(pixelBuffer.getFromPosition(j, i));
            file += to_string(c.r) + " " + to_string(c.g) + " " + to_string(c.b) + " ";
        }
        file.pop_back();
        file += "\n";
    }

    WriteFile(file, writePath);
    return true;
}


void Canvas::saveImage(ImageFileFormat fmt, string path) {
    /*if (!toDate && fmt != ImageFileFormat::SVG)*/

    if (fmt == ImageFileFormat::BMP) {
        rasterize();
        getBmp(path);
    }
    else if (fmt == ImageFileFormat::SVG) {
        resizeCont(false);
        WriteFile(getSvg(), path + ".svg");
    }
    else if (fmt == ImageFileFormat::PNM) {
        rasterize();
        getPnm(path);
    }
}


void Canvas::saveImageToAnim(ImageFileFormat fmt, string anim_dir) {
    if (!fs::is_directory(fs::path(anim_dir))) {
        fs::create_directory(fs::path(anim_dir));
    }

    string filename = anim_dir + "/";
    int x = 0;
    string app = filename + to_string(x);

    do {
        x++;
        app = filename;
        if (x < 10)
            app += "0";
        if (x < 100)
            app += "0";
        app += to_string(x);
    } while (boost::filesystem::exists(app + ".bmp"));

    saveImage(fmt, app);
}


void Canvas::rasterize() {
    // reset to a blank pixel buffer
    pixelBuffer = PixelBuffer(width, height);
    // resize all data to width/height and first quad
    resizeCont(true);

    for (OutlineGroup o : outlines) {
        for (Outline outline : o.outlines) {
            // rasterize all polygons
            DrawPolygon(pixelBuffer, outline.border, outline.style());
        }
    }

    toDate = true;
}


void Canvas::resizeCont(bool scaleDown) {
    /*
        Resize the canvas content so it fits on the height x width grid.
    */

    getBoundingBox();
    // translate into first quadrant
    translate(-box[2], -box[1], true);

    if (scaleDown) {
        // determine smaller side/side ratio for scaling
        double ratioTop = ceil(static_cast<double>(this->box[0])) / static_cast<double>(height);
        double ratioRight = ceil(static_cast<double>(this->box[3])) / static_cast<double>(width);
        double scaleFactor = 1 / ((ratioTop > ratioRight) ? ratioTop : ratioRight); 
        scale(scaleFactor * PADDING);
        
        // add padding and translate for corner sizes
        int px = static_cast<int>(static_cast<double>(width) * (1.0 - PADDING) / 2.0),
            py = static_cast<int>(static_cast<double>(height) * (1.0 - PADDING) / 2.0);

        translate(px, py, false);

        if (ratioTop < ratioRight) {
            // center vertically
            int t = static_cast<int>(((static_cast<double>(height) - (py * 2)) - static_cast<double>(this->box[0]) * scaleFactor) / 2.0);
            translate(0, t, false);
        }
        else {
            int t = static_cast<int>(((static_cast<double>(width) - (px * 2)) - static_cast<double>(this->box[3]) * scaleFactor) / 2.0);
            translate(t, 0, false);
        }
    }
}


void Canvas::drawToWindow() {
    this->rasterize();
    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_Event event;
    SDL_Window* window = SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);

    SDL_Cursor* cursor;
    cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    SDL_SetCursor(cursor);
    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_UpdateTexture(texture, NULL, pixelBuffer.ar, width * sizeof(Uint32));
    SDL_WaitEvent(&event);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    bool quit = false;

    while (!quit) {
        SDL_UpdateTexture(texture, NULL, pixelBuffer.ar, width * sizeof(Uint32));
        SDL_WaitEvent(&event);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        if (event.type == SDL_QUIT) quit = true;
    }

    // destroy arrays and SDL objects
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
