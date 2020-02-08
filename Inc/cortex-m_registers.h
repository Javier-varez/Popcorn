#ifndef CORTEX_M_REGISTERS_H_
#define CORTEX_M_REGISTERS_H_

#include <cstdint>

namespace Hw
{
    constexpr std::uint32_t SCB_CCR_STKALIGN = 1U << 9;
    constexpr std::uint32_t SCB_ICSR_PENDSVSET = 1U << 28;

    constexpr std::uint32_t SYSTICK_SHP_IDX = 11;
    constexpr std::uint32_t PEND_SV_SHP_IDX = 10;
    constexpr std::uint32_t SVC_CALL_SHP_IDX = 7;

    constexpr std::uint32_t SCB_ADDR = 0xE000ED00UL;
    struct SCB_t
    {
        std::uint32_t CPUID;
        std::uint32_t ICSR;
        std::uint32_t VTOR;
        std::uint32_t AIRCR;
        std::uint32_t SCR;
        std::uint32_t CCR;
        std::uint8_t  SHP[12U];
        std::uint32_t SHCSR;
        std::uint32_t CFSR;
        std::uint32_t HFSR;
        std::uint32_t DFSR;
        std::uint32_t MMFAR;
        std::uint32_t BFAR;
        std::uint32_t AFSR;
        std::uint32_t PFR[2U];
        std::uint32_t DFR;
        std::uint32_t ADR;
        std::uint32_t MMFR[4U];
        std::uint32_t ISAR[5U];
        std::uint32_t RESERVED0[5U];
        std::uint32_t CPACR;
    };
    extern volatile SCB_t *g_SCB;
}

#endif