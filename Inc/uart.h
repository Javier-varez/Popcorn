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

#ifndef INC_UART_H_
#define INC_UART_H_

#include <stm32f1xx_hal.h>

namespace App {
class Uart {
 public:
    Uart();
    void Init();

    void Send(const char *);

    // Copy is not allowed
    Uart(const Uart&) = delete;
    Uart operator=(const Uart&) = delete;

 private:
    UART_HandleTypeDef huart;
    constexpr static uint32_t UART_BAUDRATE = 115200;
};
}  // namespace App

#endif  // INC_UART_H_
