
#include "cortex-m_port.h"
#include "os_config.h"
#include "kernel.h"

#ifndef UNITTEST
extern "C" {
// Use proper header for MCU Here
#include "stm32f103xb.h"
}
#endif

namespace Hw
{
    MCU* g_mcu = nullptr;

    MCU::MCU() {
        g_mcu = this;
    }

    void MCU::Initialize()
    {
        #ifndef UNITTEST
        // Set OS IRQ priorities
        __NVIC_SetPriority(SysTick_IRQn, 0xFF); // Minimum priority for SysTick
        __NVIC_SetPriority(PendSV_IRQn,  0xFF); // Minimum priority for PendSV
        __NVIC_SetPriority(SVCall_IRQn,  0x00); // Maximum priority for SVC

        // Turn on stack alignment to 8 bytes on exception entry
        // On entry, the stacked value of the XPSR register will have
        // bit 9 set to 1 if the stack was aligned to 8 bytes.
        SCB->CCR |= SCB_CCR_STKALIGN_Msk;
        #endif
    }

    void MCU::TriggerPendSV()
    {
        #ifndef UNITTEST
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
        #endif
    }

    void MCU::SupervisorCall(OS::SyscallIdx svc)
    {

    }

    void MCU::HandleSVC_Static(struct OS::auto_task_stack_frame* args)
    {
        g_mcu->HandleSVC(args);
    }

    OS::SyscallIdx MCU::GetSVCCode(std::uint8_t* pc)
    {
        // First byte of svc instruction
        return static_cast<OS::SyscallIdx>(pc[-sizeof(std::uint16_t)]);
    }
    
    void MCU::HandleSVC(struct OS::auto_task_stack_frame* args)
    {
        std::uint8_t* pc = (std::uint8_t*)args->pc;
        OS::SyscallIdx svc_code = GetSVCCode(pc);
        // The arguments for the original function calls will be in
        // r0 to r3. If more arguments are required, they are placed in
        // the stack in order.
        // On exception entry, the stack must be 8 byte aligned (if STKALIGN is 1) 
        // and it is possible that the processor introduced padding in addition to
        // just performing the stacking of registers. This is why we need to check
        // bit 9 of the stacked XPSR register. This bit will be set to 1 if it was
        // 4 byte aligned on exception entry.
        uint32_t* original_call_stack = (uint32_t*)(args+1);
        bool fourByteAlignedOnEntry = (args->xpsr & (1 << 9)) != 0;
        if (fourByteAlignedOnEntry)
        {
            original_call_stack += 1;
        }
        switch (svc_code)
        {
        case OS::SyscallIdx::StartOS:
            OS::g_kernel->StartOS();
            break;
        
        case OS::SyscallIdx::CreateTask:
            OS::g_kernel->CreateTask(
                (task_func)args->r1,
                (uintptr_t)args->r2, 
                (enum OS::Priority)args->r3, 
                (char*)*original_call_stack); // this argument is now on the stack
            break;
        case OS::SyscallIdx::Sleep:
            OS::g_kernel->Sleep((uint32_t)args->r1);
            break;
        case OS::SyscallIdx::DestroyTask:
            OS::g_kernel->DestroyTask();
            break;
        case OS::SyscallIdx::Yield:
            OS::g_kernel->Yield();
            break;
        case OS::SyscallIdx::Wait:
            OS::g_kernel->Wait((const OS::Blockable*)args->r1);
            break;
        case OS::SyscallIdx::RegisterError:
        default:
            // TODO: Handle Register Error SVC call and register backtrace
            OS::g_kernel->RegisterError(args);
            break;
        }
    }
}
