/**
 * This file is part of the RVPC project 
 * https://github.com/OLIMEX/RVPC
 * https://www.olimex.com/Products/Retro-Computers/RVPC/open-source-hardware
 * 
 * Copyright (c) 2024 Olimex Ltd
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

#ifndef __PRINT_H
#define __PRINT_H

void cursor_down();
void write(char c);
void print(const char *str);
void println(const char *str);
void printfstr(const char* format, ...) __attribute__((__format__ (__printf__, 1, 2)));
void printfln(const char* format, ...) __attribute__((__format__ (__printf__, 1, 2)));

#endif // __PRINT_H
