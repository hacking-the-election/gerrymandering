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
    vector<vector<float> > border = {{1,1},{2,3},{3,3},{4,4},{5,5},{6,6},{7,7},{8,8},{9,9},{10,10},{11,11},{12,12},{13,13},{14,13},{15,12},{16,11},{17,10},{18,9},{19,8},{20,7}};//{{-41.8359375,43.068887774169625},{-34.1015625,36.59788913307022},{-27.0703125,30.751277776257812},{-20.7421875,25.16517336866393},{-15.1171875,30.44867367928756},{-9.4921875,37.43997405227057},{-3.8671874999999996,44.59046718130883},{-17.9296875,45.089035564831036},{-28.828124999999996,44.33956524809713},{-35.5078125,43.83452678223682}};//{{1,1},{2,2},{3,3},{4,4},{5,5},{6,6},{7,7},{8,8},{9,9},{10,10},{11,11},{12,12},{13,13},{14,13},{15,12},{16,11},{17,10},{18,9},{19,8},{20,7}};
    
    Shape shape(border);
    shape.draw();

    // assert_equal("checking area function", area(shape), 0.5);
    // assert_equal("testing center coord 1", center(shape)[0], 0.333333);
    // assert_equal("testing center coord 2", center(shape)[1], 0.333333);
}