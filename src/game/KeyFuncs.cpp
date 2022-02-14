
#include "KeyFuncs.h"

#include <iomanip>
#include <limits>

template <typename T>
T add_with_limit(T a, T b) {
    if (a > 0 && b > 0 && (a > std::numeric_limits<T>::max() - b)) {
        return std::numeric_limits<T>::max();
    } else if (a < 0 && b < 0 && (a < std::numeric_limits<T>::min() - b)) {
        return std::numeric_limits<T>::min();
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
std::ostream& operator<< (std::ostream& os, const FunctionTable& ft) {
    os << std::hex << std::setfill('0')
       << std::setw(8) << ft.down << " "
       << std::setw(8) << ft.up << " "
       << std::setw(8) << ft.held << " "
       << std::setw(4) << ft.mouseDelta.v << " "
       << std::setw(4) << ft.mouseDelta.h << " "
       << std::setw(1) << int(ft.buttonStatus) << std::endl
       << std::dec;
    return os;
}

// de-serialize an istream into a FunctionTable instance
std::istream& operator>> (std::istream& is, FunctionTable& ft) {
    // lines beginning with # are treated as comments
    while (is.peek() == '#') {
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    // clear all values first
    ft = {};
    if (!is.eof()) {
        uint16_t uv, uh;
        int buttonStatus; // load it into an int, otherwise it assumes it's an actual char
        is >> std::hex >> std::skipws
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
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return is;
}
