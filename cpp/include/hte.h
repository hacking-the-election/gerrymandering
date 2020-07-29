/*========================================
 hte.hpp:                        k-vernooy
 last modified:                Sun, Jun 21

 Main include header for the HtE library.
 Has definitions for all data structures,
 methods, and functions used in the C++
 library.
=========================================*/

#ifndef HTE_H
#define HTE_H
#define HTE_USE_GRAPHICS
#ifdef HTE_USE_GRAPHICS
#include <SDL2/SDL.h>
#endif


#include <map>
#include <array>
#include <vector>
#include <string>
#include <random>
#include <sstream>
#include <fstream>
#include <numeric>
#include <cmath>
#include <iostream>
#include <unordered_map>

// external library includes for json, maps, and serialization
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/filesystem.hpp>

#include "../lib/ordered-map/include/tsl/ordered_map.h"
#include "../lib/Clipper/cpp/clipper.hpp"
#include "../lib/Miniball.hpp"

#define PI 3.14159265358979323



/**
 * \brief The Hacking the Election library namespace.
 * 
 * All code for parsing data, optimizing communities,
 * and drawing maps is declared here.
 * 
 * Make sure to write big description here
 */
namespace hte {
    // algorithm structures
    class Node;
    class Graph;
    class Community;

    // geometry structures
    struct Point2d;
    class LinearRing;
    class Polygon;
    class MultiPolygon;
    
    // data/political structures
    class Precinct;
    class PrecinctGroup;
    class State;
    enum class PoliticalParty;
    enum class IdType;
    class DataParser;

    // graphics structures
    class Outline;
    class OutlineGroup;
    class PixelBuffer;
    class Canvas;
    class RgbColor;
    class HslColor;
    class Style;
    enum class ImageFileFormat;

    // any misc typedefs
    typedef double                  UnitInterval;  //!< Represents values on the interval [0, 1]
    typedef std::array<int, 2>      Edge;          //!< An edge on a graph. Contains two node IDs.
    typedef std::vector<Point2d>    Point2dVec;    //!< A list of Point2d coordinates
    typedef std::array<long, 4>     BoundingBox;   //!< An array of 4 max/mins: {top, bottom, left, right};
    typedef std::array<long, 4>     Segment;       //!< A set of two coordinates:
    typedef std::vector<Segment>    SegmentVec;    //!< A list of multiple segments
    typedef std::vector<Community>  Communities;   //!< A list of multiple Community objects. Used mostly to represent a state.

    // external geometry library typedefs (Boost.Geometry and Miniball)
    typedef boost::geometry::model::d2::point_xy<long long int>                          BoostPoint2d;
    typedef boost::geometry::model::polygon<BoostPoint2d>                                BoostPolygon;
    typedef std::vector<double>::const_iterator                                          CoordIterator;
    typedef std::vector<std::vector<double> >::const_iterator                            PointIterator; 
    typedef Miniball::Miniball <Miniball::CoordAccessor<PointIterator, CoordIterator> >  MB;


    /**
     * \brief Types of geometric boolean operations 
     */
    enum class ClipType {
        UNION,
        INTERSECTION,
        DIFFERENCE,
        XOR
    };

    /**
     * A 2-dimensional cartesian point, consisting
     * of public x and y integer coordinates.
     */
    struct Point2d {
        long x;
        long y;
        Point2d() {};
        Point2d(long int x, long int y)
            : x(x), y(y) {}

        Point2d(std::array<long, 2> list) {
            x = list[0];
            y = list[1];
        }

        // add operator overloading for object equality
        friend bool operator== (const Point2d& l1, const Point2d& l2);
        friend bool operator!= (const Point2d& l1, const Point2d& l2);
    };


    /**
     * \brief A closed set of points
     * 
     * A class containing a closed coordinate_set, analagous to
     * the `LinearRing` property in most GeoJson. 
     * A wrapper for the coordinate_set typedef
     * with extended method functionality for geometric properties
     * such as area, perimeter, and centroid calculation
     * 
     * \throw Must be passed a closed coordinate set in the 
     * constructor or will throw Exceptions::LinearRingOpen
     */
    class LinearRing {
        public:

            LinearRing() : centroid(NULL, NULL) {}
            LinearRing(Point2dVec b) : centroid(NULL, NULL) {
                border = b;
            }

            Point2d     centroid;    //!< The centroid (default NULL,NULL) of the ring
            Point2dVec  border;   //!< A closed set of coordinates

            virtual double       getSignedArea();
            virtual std::string  toJson();
            virtual double       getPerimeter();
            virtual Point2d      getCentroid();
            virtual SegmentVec   getSegments();
            virtual BoundingBox  getBoundingBox();

            // add operator overloading for object equality
            friend bool operator== (const LinearRing& l1, const LinearRing& l2);
            friend bool operator!= (const LinearRing& l1, const LinearRing& l2);
    };


    /**
        * \brief A shape with a hull and holes
        * 
        * Contains a public border of a LinearRing object
        * and a public vector of LinearRings for holes.
        * Polygon objects may contain holes, but do not have
        * multiple exterior borders
        */
    class Polygon {
        public:

            Polygon(){}
            Polygon(LinearRing hull)
                : hull(hull) {}
            Polygon(LinearRing hull, std::string shape_id)
                : hull(hull), shapeId(shape_id) {}

            Polygon(LinearRing hull, std::vector<LinearRing> holes) 
                : hull(hull), holes(holes) {}

            Polygon(LinearRing hull, std::vector<LinearRing> holes, std::string shape_id) 
                : hull(hull), holes(holes), shapeId(shape_id) {}


            LinearRing               hull;      //!< Ring of exterior hull coordinates
            std::vector<LinearRing>  holes;     //!< List of exterior holes in shape
            std::string              shapeId;  //!< The shape's GEOID, if applicable

            virtual std::string  toJson();          // get the coordinate data of the polygon as GeoJson
            virtual double       getSignedArea();   // return (area of shape - area of holes)
            virtual double       getPerimeter();    // total perimeter of holes + hull
            virtual Point2d      getCentroid();     // average centers of holes + hull
            virtual SegmentVec   getSegments();     // return a segment list with shape's segments
            virtual BoundingBox  getBoundingBox();  // get the bounding box of the hull

            // add operator overloading for object equality
            friend bool operator== (const Polygon& p1, const Polygon& p2);
            friend bool operator!= (const Polygon& p1, const Polygon& p2);
            
            int pop = 0;                     //!< Total population of the polygon
            int isPartOfMultiPolygon = -1;   //!< Internal data for parsing rules
    };

    
    
    /**
        * \brief A polygon object with multiple polygons.
        * 
        * Used for geographical objects similar to the entire discontiguous
        * US and Alaska within a single object, useful for graphics.
        * Each polygon has a mandatory `hull` property, and may contain holes.
        */
    class MultiPolygon : public Polygon {
        public:

            MultiPolygon(){} // default constructor
            MultiPolygon(std::vector<Polygon> s) {
                // constructor with assignment
                border = s;
            }
            
            MultiPolygon(std::vector<Polygon> s, std::string t_id) {
                // constructor with assignment
                border = s;
                shapeId = t_id;
            }

            MultiPolygon(std::vector<Precinct> s);

            double        getPerimeter();         // total perimeter of border array
            double        getSignedArea();        // total area of the border shape array
            virtual       std::string toJson();   // get a json string of the borders and holes
            Point2d       getCentroid();          // average centroid of inner polys
            SegmentVec    getSegments();          // return a segment list with shape's segments
            BoundingBox   getBoundingBox();

            std::vector<Polygon> border;

            // add operator overloading for object equality
            friend bool operator== (const MultiPolygon& s1, const MultiPolygon& s2);
            friend bool operator!= (const MultiPolygon& s1, const MultiPolygon& s2);
    };

    std::vector<long> GetEquation(Segment s);
    Segment PointsToSegment(Point2d c1, Point2d c2);

    bool GetBordering(Polygon, Polygon);
    bool GetBoundOverlap(BoundingBox, BoundingBox);
    bool GetBoundInside(BoundingBox, BoundingBox);
    bool GetPointInRing(Point2d, LinearRing);
    bool GetInside(LinearRing, LinearRing);
    bool GetInsideFirst(LinearRing s0, LinearRing s1);
    bool GetPointInCircle(Point2d center, double radius, Point2d point);

    double             GetDistance(Point2d c1, Point2d c2);
    double             GetDistance(Segment s);
    
    Polygon            GenerateGon(Point2d center, double radius, int nSides);
    MultiPolygon       GenerateExteriorBorder(PrecinctGroup pg);

    Segment            CoordsToSegment(Point2d c1, Point2d c2);
    LinearRing         PathToRing(ClipperLib::Path path);
    BoostPolygon       RingToBoostPoly(LinearRing);
    MultiPolygon       PathsToMultiPolygon(ClipperLib::Paths paths);
    ClipperLib::Path   RingToPath(LinearRing ring);
    ClipperLib::Paths  PolygonToPaths(Polygon shape);


    enum class PoliticalParty {
        Democrat, Republican, Green, Independent,
        Libertarian, Reform, Other, Total, AbsoluteQuantification
    };


    enum class IdType {
        GEOID,
        ELECTIONID,
        POPUID
    };


    /**
     * \brief Derived shape class for defining a precinct.
     * 
     * Contains border data, holes, and constituency data
     * Has total population, election data in the format of
     * the POLITICAL_PARTY enum, likely from the 2008 presidential election
     */
    class Precinct : public Polygon {
        public:
            Precinct(){} // default constructor
            Precinct(LinearRing ext)
                : Polygon(ext) {}
            Precinct(LinearRing ext, std::string id)
                : Polygon(ext, id) {}
            Precinct(LinearRing ext, std::vector<LinearRing> interior)
                : Polygon(ext, interior) {}
            Precinct(LinearRing ext, std::vector<LinearRing> interior, std::string id)
                : Polygon(ext, interior, id) {}
            Precinct(LinearRing ext, int popu, std::string id)
                :  Polygon(ext, id) {
                this->pop = popu;
            }

            // add operator overloading for object equality
            friend bool operator== (const Precinct& p1, const Precinct& p2);
            friend bool operator!= (const Precinct& p1, const Precinct& p2);

            std::map<PoliticalParty, int> voterData;  //!< Voter data in the form `{POLITICAL_PARTY, count}`
    };


    /**
     * A class for defining a group of precincts.
     * Used for all geometric calculations in the
     * community generation algorithm
     */
    class PrecinctGroup : public MultiPolygon {
        public:

            PrecinctGroup(){}
            PrecinctGroup(std::vector<Polygon> shapes)
                : MultiPolygon(shapes) {}
            
            PrecinctGroup(std::vector<Precinct> precincts)
                : MultiPolygon(precincts), precincts(precincts) {}


            // array of precinct objects
            std::vector<Precinct> precincts;

            std::string  toJson();
            int            getPopulation();
            void           removePrecinct(Precinct);
            void           addPrecinct(Precinct);
            Precinct       getPrecinctFromId(std::string);
            double         getArea();

            BoundingBox   getBoundingBox();
            Point2d       getCentroid();
            
            static PrecinctGroup from_graph(Graph& g);

    };


    /**
     * A vertex on the `Graph` class, containing
     * precinct information and edge information
    */
    class Node {
        public:

            int id;                        //!< A unique integer identifier for the node
            int community;                 //!< Which community the node belongs to
            Precinct* precinct;            //!< A precinct pointer for geometry functions
            std::vector<Edge> edges;       //!< A list of unique edges for the node, in the form `{this->id, connected_node.id}`
            std::vector<int> collapsed;    //!< A list of collapsed nodes for the karger-stein algorithm

            Node(){}
            Node(Precinct* precinct) : precinct(precinct) {}

            friend bool operator< (const Node& l1, const Node& l2);
            friend bool operator== (const Node& l1, const Node& l2);
    };

    
    /**
     * \brief Remove all edges to specific node
     * 
     * For every node connected to `id`, remove the edge
     * connecting to `id`.
     * 
     * \param g Graph to remove edges from
     * \param id Node to removes edges to
     * \return Updated graph with removed edges
     */
    Graph RemoveEdgesTo(Graph g, int id);


    /**
     * \brief A graph as defined by discrete mathematics
     * 
     * Contains a list of Node objects, with Edges connecting
     * a certain subset of Nodes. This represents an undirected
     * and unweighted graph. It is used for Community
     * generation algorithms
     */
    class Graph {
        public:
            tsl::ordered_map<int, Node> vertices;  //!< All nodes on the graph
            std::vector<Edge> edges;               //!< List of unique edges on the graph

            /**
             * \brief Get the induced subgraph from a list of node ids
             * \param nodes the integer list of nodes to get the subgraph of
             * \return The subgraph object
             */
            Graph getInducedSubgraph(std::vector<int> nodes);

            /**
             * Determine the number of [components](https://en.wikipedia.org/wiki/Component_(graph_theory)) in the graph
             * \return number of components
             */
            int getNumComponents();

            /**
             * get a list of subgraph components
             * \return subgraph components
             */
            std::vector<Graph> getComponents();
            bool isConnected();
            void addNode(Node node);
            void removeNode(int id);
            void addEdge(Edge);
            void removeEdge(Edge);
            void removeEdgesTo(int id);

            protected:
            // recursors for getting different data
            void dfsRecursor(int v, std::unordered_map<int, bool>& visited);
            void dfsRecursor(int v, std::unordered_map<int, bool>& visited, std::vector<int>* nodes);
            void dfsRecursor(int v, int& visited, std::unordered_map<int, bool>& visitedB);
    };



    /**
     * \brief Shape class for defining a state.
     *        Includes arrays of precincts, and districts.
     * 
     * Contains methods for generating from binary files
     * or from raw data with proper district + precinct data
     * and serialization methods for binary and json.
    */
    class State : public PrecinctGroup {
        public:

            State(){}
            State(std::vector<MultiPolygon> districts, std::vector<Precinct> t_precincts, std::vector<Polygon> shapes)
                : PrecinctGroup(shapes), districts(districts) {
                // simple assignment constructor for
                // nonstatic superclass member
                precincts = t_precincts;
            }

            // generate a file from proper raw input with and without additional voter data files
            static State GenerateFromFile(DataParser&);
            static State GenerateFromFile(std::string, std::string, std::map<PoliticalParty, std::string>, std::map<IdType, std::string>);
            static State GenerateFromFile(std::string, std::string, std::string, std::map<PoliticalParty, std::string>, std::map<IdType, std::string>);

            Graph network; // represents the precinct network of the state
            std::vector<MultiPolygon> districts; // the actual districts of the state

            // serialize and read to and from binary, json
            void            toFile(std::string path);
            static State    fromFile(std::string path);
    };


    /**
     * Convert a string that represents a geojson polygon into an MultiPolygon object.
     * \param str The string to be converted into a vector of Polygons
     * \param texas_coordinates Whether or not to use texas-scaled coordinates 
     * \return The converted polygon object
     * 
     * \see MultiPolygon
    */
    MultiPolygon StringToMultiPoly(std::string str, bool texas_coordinates);
    
    /**
     * Convert a string that represents a geojson polygon into an object.
     * \param str The string to be converted into a Polygon
     * \param texas_coordinates Whether or not to use texas-scaled coordinates 
     * \return The converted polygon object
     * 
     * \see Polygon
    */
    Polygon StringToPoly(std::string str, bool texas_coordinates);

    // parsing functions for tsv files
    std::vector<std::vector<std::string > > parseSV(std::string, std::string);
    
    /**
     * Contains a list of precincts, as well as information about
     * linking and where is it on an island list.
    */
    class Community : public Graph {

        public:
            double quantification;
            double partisanQuantification;
            
            // shape object for geometry methods,
            // must be kept up to date in every operation
            PrecinctGroup shape;

            int  getPopulation();
            void addNode(Node&);
            void removeNode(int);
            void resetShape(Graph&);

            Community(std::vector<int>& nodeIds, Graph& graph);
            Community() {}
    };


    double Average(Communities& cs, double (*measure)(Community&));
    double GetPartisanshipStdev(Community& c);
    double GetCompactness(Community& c);
    double GetPreciseCompactness(Community& c);
    double GetDistanceFromPop(Communities& cs, double);
    double GetScalarizedMetric(Communities& cs);
    int GetNumPrecinctsChanged(Graph& g1, Graph& g2);

    void SaveCommunitiesToFile(Communities, std::string);
    Communities LoadCommunitiesFromFile(std::string, Graph&);
    Communities LoadCommunitiesWithQuantification(std::string, Graph&, std::string);
    std::vector<std::vector<double> > LoadQuantification(std::string tsv);

    bool ExchangePrecinct(Graph& g, Communities& cs, int nodeToTake, int communityToTake);
    std::vector<std::array<int, 2> > GetAllExchanges(Graph& g, Communities& cs);

    /**
     * Partitions a graph according to the Karger-Stein algorithm
     * \param graph The graph to partition
     * \param n_communities The number of partitions to make
     * \return: `Communities` list of id's corresponding to the pa
     * rtition
     */
    Communities KargerStein(Graph& graph, int nCommunities);
    void GradientDescentOptimization(Graph& g, Communities& cs, double (*measure)(Communities&));
    void SimulatedAnnealingOptimization(Graph& g, Communities& cs, double (*measure)(Community&));

    double CollapseVals(double a, double b);
    double GetPopulationFromMask(PrecinctGroup pg, MultiPolygon mp);
    std::map<PoliticalParty, double> GetPartisanshipFromMask(PrecinctGroup pg, MultiPolygon mp);
    std::map<PoliticalParty, double> GetQuantification(Graph& graph, Communities& communities, MultiPolygon district);


    /**
     * \brief Exceptions for geometric or algorithmic
     * errors and recursion breakouts for graphics.
    */
    class Exceptions {
        public:
            struct PrecinctNotInGroup : public std::exception {
                const char* what() const throw() {
                    return "No precinct in this precinct group matches the provided argument";
                }
            };

            struct LinearRingOpen : public std::exception {
                const char* what() const throw() {
                    return "Points LinearRing do not form closed ring";
                }
            };
    };
    
    /**
     * \brief Gets whether or not a string represents a numeric value
     * \param token The string to check
     * \return Whether or not the string is a float, double, or int
     */
    bool IsNumber(std::string token);
    
    /**
     * \brief Writes (if possible) text to a specified file
     * \param contents The string to write to a file
     * \param path The file path to write to
     */
    void WriteFile(std::string contents, std::string path);
   
    /**
     * \brief Reads (as an std::string) the contents of a file
     * \param path The file path to read from.
     * \return The contents of the file
     */
    std::string ReadFile(std::string path);
       
    /**
     * \brief Joins a std::vector<> by a delimeter
     * \param str The list of strings to be joined
     * \param del The delimeter to join by
     * \return The vector as a joined string
     */
    std::string Join(std::vector<std::string> str, std::string del);
    
    /**
     * \brief Splits an std::string by a delimeter
     * \param str The string to be split to a vector
     * \param del The delimeter to split by
     * \return The string as a split vector
     */
    std::vector<std::string> Split(std::string str, std::string del);
    std::string GetProgressBar(double progress);

    int RandInt(int start, int end);
    double RandUnitInterval();

    double GetStdev(std::vector<int>& data);
    double GetStdev(std::vector<double>& data);


    /**
     * An enumeration representing various image
     * formats that the canvas can be written to
     */
    enum class ImageFileFormat {
        PNG, SVG, BMP, PNM
    };


    // for color space conversions (currently just hsl/rgb)
    double HueToRgb(double p, double q, double t);
    RgbColor HslToRgb(HslColor hsl);
    HslColor RgbToHsl(RgbColor rgb);

    // interpolate between colors
    HslColor InterpolateHsl(HslColor, HslColor, double);
    RgbColor InterpolateRgb(RgbColor, RgbColor, double);
    double Lerp(double, double, double);

    // color palette generators
    std::vector<RgbColor> GenerateColors(int n);

    // convert geometry shapes into styled outlines
    Outline ToOutline(LinearRing r);
    std::vector<Outline> ToOutline(State state);
    std::vector<Outline> ToOutline(Graph& graph);
    std::vector<Outline> ToOutline(Communities& communities);
    std::vector<OutlineGroup> ToOutlineGroup(Communities& communities);
    OutlineGroup ToOutline(MultiPolygon&, double, bool absQuant);
    OutlineGroup ToOutline(MultiPolygon&);


    class EdgeBucket {
    public:
        int miny;
        int maxy;
        double minyX;
        double slope;
        
        friend bool operator< (const EdgeBucket& b1, const EdgeBucket& b2) {
            if (b1.miny < b2.miny) return true;
            if (b2.miny < b1.miny) return false;

            if (b1.minyX < b2.minyX) return true;
            if (b2.minyX < b1.minyX) return false;

            return false;
        }

        EdgeBucket() {}
    };


    class RgbColor {
        // a color representing rgb color channels
        public:
            int r, g, b;
            Uint32 toUint();
            static RgbColor fromUint(Uint32 color);

            friend bool operator!= (const RgbColor& c1, const RgbColor& c2) {
                return (c1.r != c2.r || c1.g != c2.g || c1.b != c2.b);
            }

            friend bool operator== (const RgbColor& c1, const RgbColor& c2) {
                return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
            }

            // constructors with default conversions
            RgbColor() {}
            RgbColor(int r, int g, int b) : r(r), g(g), b(b) {}
            RgbColor(std::string hex);
            RgbColor(HslColor);
    };


    class HslColor {
        // a color representing hsl color channels
        public:
            double h, s, l;

            friend bool operator!= (const HslColor& c1, const HslColor& c2) {
                return (c1.h != c2.h || c1.s != c2.s || c1.l != c2.l);
            }

            friend bool operator== (const HslColor& c1, const HslColor& c2) {
                return (c1.h == c2.h && c1.s == c2.s && c1.l == c2.l);
            }

            // constructors with default conversions
            HslColor() {};
            HslColor(double h, double s, double l) : h(h), s(s), l(l) {}
            HslColor(RgbColor);
            HslColor(std::string hex);
    };
 

    class PixelBuffer {
        // contains pixel data in the form of
        // uint array, see `Uint_to_rgb`

        public:
            int x, y;
            Uint32* ar;
        
            PixelBuffer() {};
            PixelBuffer(int x, int y) : x(x), y(y) { ar = new Uint32[x * y]; memset(ar, 255, x * y * sizeof(Uint32));}
            void resize(int t_x, int t_y) { x = t_x; y = t_y; ar = new Uint32[x * y]; memset(ar, 255, x * y * sizeof(Uint32));}

            void setFromPosition(int, int, Uint32);
            Uint32 getFromPosition(int a, int b);
            int indexFromPosition(int, int);
    };

    /**
     * \brief An encapsulation of visual attributes
     *
     * Contains fill, outline color, and thickness,
     * used for each outline in a canvas.
     */
    class Style {    
        public:

            RgbColor fill_;     //!< The fill color of an outline
            RgbColor outline_;  //!< The outline color
            double thickness_;   //!< The outline thickness (in pixels)
        
            Style& thickness(double);
            Style& fill(RgbColor);
            Style& fill(HslColor);
            Style& outline(RgbColor);
    };


    /**
     * \brief A graphics poly to be rendered
     * 
     * Contains a style object and a border
     * that represents coordinates of a LinearRing
     * to be rendered to a PixelBuffer.
     */
    class Outline {
        private:
            Style style_;  //!< An object storing thickness, fill, and color

        public:
            Outline(LinearRing border) : border(border) {}
            Style& style() {return style_;}

            LinearRing border;  //!< The coordinates of the outline
            
            /**
             * \brief Get the SVG string of the object
             * 
             * Gets a string with a `d` attribute representing
             * the LinearRing, and with inline styling representing
             * the `style_` object.
             * 
             * \param sf The scale factor to scale the canvas by (without rounding)
             * \return An inline SVG string
             */
            std::string getSvg(double sf);
    };


    /**
     * \brief A collection of outlines.
     * 
     * Used to represent a group of non-contiguous
     * outlines that must be grouped under interactions.
     */
    class OutlineGroup {
        public:
            /**
             * \brief Get an SVG string representing the outline group
             * \return The SVG string of the outlines
             */
            std::string getSvg(double);

            OutlineGroup() {};
            OutlineGroup(Outline o) { outlines.push_back(o); }
            OutlineGroup(std::vector<Outline> o) { outlines.insert(outlines.end(), o.begin(), o.end()); }

            /**
             * Add an outline to the outline group
             */
            void addOutline(Outline o) {outlines.push_back(o);}

            std::vector<Outline> outlines;  //!< A group of outlines
    };


    /**
     * \brief Represents a canvas for graphics and rendering
     * 
     * Used for rendering Outline objects to screens or images.
     * Can render shapes (filled or otherwise) and lines in combination
     * with Style objects.
     */
    class Canvas {
        /*
            A class for storing information about a screen
            of pixels and shapes to be written to an SDL display
        */

        private:
            // update the canvas's pixel buffer
            // to be called by internal methods such as to_gui();
            std::string getSvg();
            bool getBmp(std::string write_path);
            bool getPnm(std::string write_path);
            void resizeCont(bool scale_down);

        public:

            bool toDate = true;  //!< Whether or not the canvas' pixels are up to date
            
            /**
             * \brief Rasterize all outlines to pixel_buffer
             * 
             * Updates the canvas's pixel buffer with rasterized outlines
             * Rasterizes outlines using the Bresenham method (antialiased)
             * and fills polygons with the scanline fill method.
             */
            void rasterize();

            std::vector<OutlineGroup> outlines;   //!< The outline data (coordinates) to be written to pixels
            PixelBuffer pixelBuffer;              //!< The pixels to be written to a screen or image
            int width, height;                    //!< The size of the canvas in pixels
            BoundingBox box;            //!< The axis-aligned bounding box of all outlines on the canvas

            /**
             * \brief Updates and returns the bounding box of all outline groups
             * in the canvas.
             */
            BoundingBox getBoundingBox();
            Canvas(int width, int height) : width(width), height(height) {}

            /**
             * \brief Translate canvas objects
             * 
             * Translates all linear rings contained in the
             * canvas object by t_x and t_y
             * 
             * \param t_x  The x coordinate to translate by
             * \param t_y  The y coordinate to translate by
             */
            void translate(long, long, bool);

            /**
             * \brief Scale canvas objects
             * 
             * Scales all linear rings contained in the canvas
             * object by scale_factor (including holes)
             * \param scale_factor  The factor by which to scale coordinates
             */
            void scale(double scaleFactor);

            void saveImage(ImageFileFormat, std::string);
            void saveImageToAnim(ImageFileFormat, std::string);


            // add shape to the canvas
            void addOutline(Outline o) {outlines.push_back(OutlineGroup(o));};
            void addOutlines(std::vector<Outline> os) {for (Outline o : os) outlines.push_back(OutlineGroup(o));}

            void addOutlineGroup(OutlineGroup og) {outlines.push_back(og);}
            void addOutlineGroups(std::vector<OutlineGroup> ogs) {outlines.insert(outlines.end(), ogs.begin(), ogs.end());}

            void clear();

            /**
             * Draws the shapes in the canvas to the screen
             * (in the case of no window passed, create a window)
             */
            void drawToWindow();

            /**
             * Draws the shapes in the canvas to the screen
             * (in the case of no window passed, create a window)
             */
            void drawToWindow(SDL_Window* window);
    };


    /**
     * \brief: Rasterizes a line using Bresenham's algorithm
     * 
     * Uses the basic Bresenham rasterization algorithm, but
     * also fills with thickness and antialiasing. Draws with
     * any RGB provided color, defaulting to black.
     * 
     * \param buffer The PixelBuffer to rasterize a line to
     * \param start The start coordinate ot he line
     * \param end The endpoint of the line
     * \param t The thickness of the line
     * \param color The color of the line (default to black)
     */
    void DrawLine(
        PixelBuffer& buffer, Point2d start, Point2d end,
        RgbColor color = RgbColor(0,0,0), double t = 1
    );

    /**
     * \brief Draw a polygon with scanline fill
     * 
     * Rasterizes a LinearRing object using the scanline
     * fill algorithm. Can change fill and outline styles
     * 
     * \see Canvas::draw_polygon
     * \param buffer The PixelBuffer to draw a ring to
     * \param ring The coordinates to rasterize
     * \param style A style object containing color, thickness, and fill
     */
    void DrawPolygon(PixelBuffer& buffer, LinearRing ring, Style style);
}

#endif
