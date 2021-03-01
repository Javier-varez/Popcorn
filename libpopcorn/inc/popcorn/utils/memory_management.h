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

#ifndef POPCORN_UTILS_MEMORY_MANAGEMENT_H_
#define POPCORN_UTILS_MEMORY_MANAGEMENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define MEMORY_POOL_SIZE        (4096)

void* OsMalloc(size_t size);
void OsFree(void* ptr);

#ifdef __cplusplus
}
#endif

#endif  // POPCORN_UTILS_MEMORY_MANAGEMENT_H_
