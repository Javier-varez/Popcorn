
#ifndef CORTEX_M_PORT_H_
#define CORTEX_M_PORT_H_

#include "platform.h"
#include "syscall_idx.h"
#include "kernel.h"

CLINKAGE void SVC_Handler();

class MCUTest;
namespace Hw
{
    class MCU;
    extern MCU* g_mcu;

    class MCU
    {
    public:
        explicit MCU();
        TEST_VIRTUAL void Initialize();

        template<OS::SyscallIdx svc_code>
        static void SupervisorCall()
        {
            #ifdef UNITTEST
            g_mcu->SupervisorCall(svc_code);
            #else
            asm volatile("svc %[opcode]"
             : : [opcode] "i" (static_cast<uint32_t>(svc_code)));
            #endif
        }
        TEST_VIRTUAL void TriggerPendSV();
    protected:
        TEST_VIRTUAL void SupervisorCall(OS::SyscallIdx svc);
        TEST_VIRTUAL void HandleSVC(struct OS::auto_task_stack_frame* args);
        static void HandleSVC_Static(struct OS::auto_task_stack_frame* args);
    private:
        OS::SyscallIdx GetSVCCode(std::uint8_t* pc);

        friend void ::SVC_Handler();
        friend class ::MCUTest;
    };
} // namespace Hw

#endif