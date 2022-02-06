
#include "KeyFuncs.h"

#include <iomanip>
using namespace std;

template <typename T>
T add_with_limit(T a, T b) {
    if (a > 0 && b > 0 && (a > numeric_limits<T>::max() - b)) {
        return numeric_limits<T>::max();
    } else if (a < 0 && b < 0 && (a < numeric_limits<T>::min() - b)) {
        return numeric_limits<T>::min();
    }
    return a + b;
}

void FunctionTable::merge(const FunctionTable &ft) {
    // bit fields get bitwise OR'd together, mouse positions are added
    down |= ft.down;
    up   |= ft.up;
    held |= ft.held;
    mouseDelta.v = add_with_limit(mouseDelta.v, ft.mouseDelta.v);
    mouseDelta.h = add_with_limit(mouseDelta.h, ft.mouseDelta.h);
    buttonStatus |= ft.buttonStatus;
}

// serialize a FunctionTable instance to an ostream
ostream& operator<< (ostream& os, const FunctionTable& ft) {
    os << hex << setfill('0')
       << setw(8) << ft.down << " "
       << setw(8) << ft.up << " "
       << setw(8) << ft.held << " "
       << setw(4) << ft.mouseDelta.v << " "
       << setw(4) << ft.mouseDelta.h << " "
       << setw(1) << int(ft.buttonStatus) << endl
       << dec;
    return os;
}

// de-serialize an istream into a FunctionTable instance
istream& operator>> (istream& is, FunctionTable& ft) {
    // lines beginning with # are treated as comments
    while (is.peek() == '#') {
        is.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    // clear all values first
    ft = {};
    if (!is.eof()) {
        uint16_t uv, uh;
        int buttonStatus; // load it into an int, otherwise it assumes it's an actual char
        is >> hex >> skipws
           >> ft.down
           >> ft.up
           >> ft.held
           >> uv
           >> uh
           >> buttonStatus;
        // parsing signed values is wonky, so parse as unsigned then cast to signed (there must be a better way)
        ft.mouseDelta.v = short(uv);
        ft.mouseDelta.h = short(uh);
        ft.buttonStatus = buttonStatus;
        is.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return is;
}
