#include "Saleae.h"
#include <cstdio>

int main()
{
    Saleae saleae("127.0.0.1", 10429);
    std::printf("Get samples\n");
    printf("nsamples: %d\n", saleae.GetNumSamples());

    return 0;
}