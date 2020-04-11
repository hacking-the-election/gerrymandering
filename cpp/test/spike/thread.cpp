#include <thread>
#include <iostream>
#include <unistd.h>

using namespace std;

void print(int& val, bool& done) {
    while (!done) {
        cout << val << endl;
        usleep(1000000);
    }
}


void dowork(int& val, bool& done) {
    while (val < 20) {
        val++;
        usleep(1000000);
    }

    done = true;
}


int main() {
    int test = 0;
    bool is_finished = false;


    thread th1(dowork, std::ref(test), std::ref(is_finished));
    thread th2(print, std::ref(test), std::ref(is_finished));

    th1.join();
    th2.join();
}