/*
 * This file is part of the Popcorn
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

#ifndef POPCORN_API_SYSCALL_H_
#define POPCORN_API_SYSCALL_H_

#include <cstdint>

#include "popcorn/API/isyscall.h"

namespace Popcorn {
/**
 * @brief Entrypoint to the kernel. User API must use these
 *        functions to interface with the kernel. This can be
 *        seen as the public API for the kernel.
 */
class Syscall : private ISyscall {
 public:
  /**
   * @brief Creates a new tasks and adds it to the ready list.
   * @param func Task function that will run when the task is scheduled.
   * @param arg Argument for the task. It can be used to pass data to
   *            the task and customize the operation of the task function.
   * @param priority The level of priority for the current task.
   * @param name The name of the task. Only used internally and for debugging.
   * @param stack_size Size in bytes for the stack that will be allocated
   *                   for the newly created task.
   */
  void CreateTask(task_func func, void* arg, Priority priority,
                  const char* name, std::uint32_t stack_size) override;

  /**
   * @brief Removes the task from the ready list. It will stop the task
   *        and free all associated resources
   */
  void DestroyTask() override;

  /**
   * @brief Sleep the current task for the specified number of ticks.
   * @param ticks Number of ticks for which the task should be asleep
   *              and not scheduled to run by the kernel.
   */
  void Sleep(std::uint32_t ticks) override;

  /**
   * @brief Starts the scheduler operation. This function is not
   *        expected to return. When called, the system will start
   *        scheduling tasks or run the Idle task if none were created.
   */
  void StartOS() override;

  /**
   * @brief Asks the scheduler to perform the scheduling and start running
   *        the task with the largest priority in the ready state.
   */
  void Yield() override;

  /**
   * @brief Informs the kernel of an error in userspace.
   */
  void RegisterError() override;

  /**
   * @brief Informs the kernel when a lockable resource is acquired/released.
   * @param lockable the locked/unlocked resource.
   * @param acquired true if it was acquried, false otherwise.
   */
  void Lock(Lockable& lockable, bool acquired) override;

  /**
   * @brief Obtains the singleton instance of the Syscall class.
   * @return the singleton instance of Syscall.
   */
  static Syscall& Instance();

  // Avoid copy and move
  Syscall(const Syscall&) = delete;
  Syscall& operator=(const Syscall&) = delete;
  Syscall(Syscall&&) = delete;
  Syscall& operator=(Syscall&&) = delete;

 private:
  /**
   * @brief private default constructor
   *
   * Should only be instantiated internally by this class.
   */
  Syscall() = default;

  /**
   * @brief Called by a lockable resource when a task blocks
   *        trying to acquire an already locked resource.
   * @param lockable the reference to the lockable resource
   *                 that originated the syscall
   *
   * Should only be called internally by the Lockable class,
   * which is the reason why it is declared private
   */
  void Wait(const Lockable& lockable) override;

  /**
   * @brief The Lockable class needs to be declared as a friend
   *        to access the Wait method.
   */
  friend class Lockable;
  friend class SyscallTest;
};
}  // namespace Popcorn

#endif  // POPCORN_API_SYSCALL_H_
