#include "timer.h"
#include <iostream>
using namespace std;

int main()
{
    timer t;
    int i = 1000000;
    while(i--);
    cout << t.get_usec() << endl;
    timer t2;
    i = 1000000;
    while(i--);
    cout << t2.get_sec() << endl;
}
