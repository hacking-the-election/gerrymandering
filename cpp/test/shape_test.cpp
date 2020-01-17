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
    vector<vector<float> > border = {{-37.6611328125,54.27805485967281},{-34.453125,59.701013531997326},{-32.05810546875,54.27805485967281},{-39.79248046875,58.04300405858762},{-29.553222656249996,58.112714441253125}};
    Shape shape(border);
    shape.draw();

    // assert_equal("checking area function", area(shape), 0.5);
    // assert_equal("testing center coord 1", center(shape)[0], 0.333333);
    // assert_equal("testing center coord 2", center(shape)[1], 0.333333);
}