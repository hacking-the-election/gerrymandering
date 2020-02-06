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
    vector<vector<float> > border = {{-119.091796875,21.207458730482642},{-101.77734374999999,33.211116472416855},{-122.958984375,32.91648534731439},{-105.1171875,21.53484700204879},{-111.09374999999999,38.61687046392973},{-119.091796875,21.207458730482642}};
    Shape shape(border);
    shape.draw();

    // assert_equal("checking area function", area(shape), 0.5);
    // assert_equal("testing center coord 1", center(shape)[0], 0.333333);
    // assert_equal("testing center coord 2", center(shape)[1], 0.333333);
}