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

#ifndef OS_INC_UNIQUE_LOCK_H_
#define OS_INC_UNIQUE_LOCK_H_

namespace OS {
template<class T>
class UniqueLock {
 public:
    explicit inline UniqueLock(T& mutex) : // NOLINT
        m_mutex(mutex) {
        m_mutex.Lock();
    }

    inline ~UniqueLock() {
        m_mutex.Unlock();
    }

 private:
    T& m_mutex;
};
}  // namespace OS

#endif  // OS_INC_UNIQUE_LOCK_H_