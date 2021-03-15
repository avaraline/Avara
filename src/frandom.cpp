#include "FastMat.h"
#include <cstdio>

extern Fixed FRandSeed;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: frandom [seed] [num]\n");
        return 1;
    }

    FRandSeed = std::stoi(argv[1]);
    int num = std::stoi(argv[2]);
    printf("seed = %d\n", FRandSeed);

    for (int i = 0; i < num; i++) {
        Fixed randomFixed = FRandom();
        double randomDouble = randomFixed / 65536.0;
        printf("%d -> %f\n", randomFixed, randomDouble);
    }

    return 0;
}
