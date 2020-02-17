/* 
 * This file is part of the Cortex-M Scheduler
 * Copyright (c) 2020 Javier Alvarez
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INC_SYSCALL_H_
#define INC_SYSCALL_H_

#include <cstdint>

typedef void (*task_func)(void*);

// Application Hooks
void App_SysTick_Hook();

class SyscallTest;
namespace OS {
enum class Priority {
    IDLE = 0,
    Level_0,
    Level_1,
    Level_2,
    Level_3,
    Level_4,
    Level_5,
    Level_6,
    Level_7,
    Level_8,
    Level_9
};

class Blockable;
class Syscall {
 public:
    void CreateTask(task_func func,
        std::uintptr_t arg,
        Priority priority,
        const char* name,
        std::uint32_t stack_size);
    void DestroyTask();
    void Sleep(std::uint32_t ticks);
    void StartOS();
    void Yield();
    void RegisterError();
    void Lock(const Blockable& blockable, bool acquired);
    static Syscall& Instance();

 private:
    void Wait(const Blockable&);
    friend class Blockable;
    friend class ::SyscallTest;
};
}  // namespace OS

#endif  // INC_SYSCALL_H_
