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
            asm volatile("cpsid i");
        }

        inline ~CriticalSection()
        {
            asm volatile("cpsie i");
        }
    };
}

#endif