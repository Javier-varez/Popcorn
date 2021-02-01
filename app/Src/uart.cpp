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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <cstring>

#include "Inc/uart.h"

namespace App {

Uart::Uart() {
    memset(&m_huart, 0, sizeof(m_huart));
}

void Uart::Init() {
    __HAL_RCC_USART2_CLK_ENABLE();

    m_huart.Instance = USART2;
    m_huart.Init.BaudRate = UART_BAUDRATE;
    m_huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    m_huart.Init.WordLength = UART_WORDLENGTH_8B;
    m_huart.Init.StopBits = UART_STOPBITS_1;
    m_huart.Init.Parity = UART_PARITY_NONE;
    m_huart.Init.Mode = UART_MODE_TX_RX;
    m_huart.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&m_huart);
}

void Uart::Send(const char* str) {
    char *noConstStr = const_cast<char*>(str);
    HAL_UART_Transmit(&m_huart,
        reinterpret_cast<uint8_t*>(noConstStr),
        std::strlen(str), 1000);
}

}  // namespace App

void HAL_UART_MspInit(UART_HandleTypeDef *m_huart) {
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio;
    gpio.Pin = GPIO_PIN_2;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOA, &gpio);
    gpio.Pin = GPIO_PIN_3;
    gpio.Mode = GPIO_MODE_AF_INPUT;
    HAL_GPIO_Init(GPIOA, &gpio);
}

