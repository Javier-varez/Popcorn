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

#ifndef POPCORN_CORE_LOCKABLE_H_
#define POPCORN_CORE_LOCKABLE_H_

namespace Popcorn {
/**
 * @brief Implements a basic lockable type.
 *
 * Use this as a parent class for mutex, semaphore,
 * queues and anything that can lock on a resource.
 * It is internally used to keep track of the blocked task
 * and perform priority inheritance.
 */
class Lockable {
 protected:
  /**
   * @brief Blocks waiting for this resource
   *
   * Call from the child object when waiting for the
   * resource to be available
   */
  void Block();

  /**
   * @brief Inform the kernel about the acquired lock
   *
   * Call from the child object when the locked resource
   * has been acquired.
   */
  void LockAcquired();

  /**
   * @brief Inform the kernel about the released lock.
   *
   * Call from the child object when the locked resource
   * has been released.
   */
  void LockReleased();

 private:
  /**
   * @brief Setter for m_blocker.
   * @param tcb pointer to the task locking this resource.
   */
  void SetBlockerTask(struct task_control_block* tcb);

  /**
   * @brief Getter for the task blocking the resource.
   * @return Pointer to the task locking the resource.
   *         It can be nullptr if not locked.
   */
  struct task_control_block* GetBlockerTask() const;

  struct task_control_block *m_blocker;

  /**
   * @brief The kernel needs to call private methods
   *        for priority inheritance reasons.
   */
  friend class Kernel;
};
}  // namespace Popcorn

#endif  // POPCORN_CORE_LOCKABLE_H_
