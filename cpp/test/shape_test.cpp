#include "../include/shape.hpp"
#include "../include/term_disp.hpp"

using namespace std;

void assert_equal(string context, float n1, float n2) {
    if ( n1 != n2 )
        cout << RED << "error " << RESET << "in " << context << ", " << n1 << " != " << n2;
    else
        cout << GREEN << "passed " << RESET << context << ", " << n1 << " = " << n2;
    cout << endl;
}

int main(int argc, char* argv[]) {
    vector<vector<float> > border = {{-18.017578125,50.12057809796008},{-10.1953125,50.3454604086048},{-2.724609375,50.62507306341435},{0.9667968749999999,50.84757295365389},{8.7890625,51.069016659603896},{16.69921875,51.45400691005982},{22.587890625,51.67255514839674},{22.939453125,50.17689812200107},{17.05078125,49.95121990866204},{8.525390625,49.724479188712984},{0.791015625,49.61070993807422},{-2.8125,49.55372551347579},{-9.580078125,49.03786794532644},{-12.919921874999998,48.86471476180277},{-17.666015625,48.63290858589535}};

    
    Shape shape(border);
    shape.draw();

    // assert_equal("checking area function", area(shape), 0.5);
    // assert_equal("testing center coord 1", center(shape)[0], 0.333333);
    // assert_equal("testing center coord 2", center(shape)[1], 0.333333);
}