#ifndef CRITICAL_SECTION_H_
#define CRITICAL_SECTION_H_

namespace OS
{
    class CriticalSection
    {
    public:
        // These instructions set the PRIMASK register,
        // Masking all Interrupts but the hardfault and NMI
        explicit inline CriticalSection()
        {
            #ifndef UNITTEST
            asm volatile("cpsid i");
            #endif
        }

        inline ~CriticalSection()
        {
            #ifndef UNITTEST
            asm volatile("cpsie i");
            #endif
        }
    };
}

#endif