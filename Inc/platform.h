#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <cstdint>

#define CLINKAGE                    extern "C"
#define __STRINGIZE(_x)             #_x
#define __NAKED                     __attribute__((naked))
#define __WEAK                      __attribute__((weak))

#ifdef UNITTEST
#define TEST_VIRTUAL virtual
#else
#define TEST_VIRTUAL
#endif

#endif