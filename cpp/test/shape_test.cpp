#include "../include/shape.hpp"
#include "../include/colors.hpp"

using namespace std;

void assert_equal(string context, float n1, float n2) {
    if ( n1 != n2 )
        cout << RED << "error " << RESET << "in " << context << ", " << n1 << " != " << n2;
    else
        cout << GREEN << "passed " << RESET << context << ", " << n1 << " = " << n2;
    cout << endl;
}

int main(int argc, char* argv[]) {

    Shape shape({ {0, 0}, {0,1}, {1,0} });

    assert_equal("checking area function", area(shape), 0.5);
    assert_equal("testing center coord 1", center(shape)[0], 0.333333);
    assert_equal("testing center coord 2", center(shape)[1], 0.333333);
}