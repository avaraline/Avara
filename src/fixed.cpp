#include "FastMat.h"
#include <cstdio>
#include <cmath>

#define UNITPOINTS (double)14.4 // 72 / 5

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: fixed [value]\n");
        return 1;
    }

    double value = std::stod(argv[1]);
    long scaled = std::lround(value * UNITPOINTS);

    printf("value = %f\n", value);
    printf("scaled = %ld\n", scaled);
    printf("fixed = %d\n", ToFixed(value));

    return 0;
}
