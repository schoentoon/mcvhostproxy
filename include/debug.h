/*  mcvhostproxy
 *  Copyright (C) 2013  Toon Schoenmakers
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef NO_DEBUG
#  define DEBUG(level, format, ...)
#else
   void __internal_debug(unsigned char level, const char* format, ...);
#  define DEBUG(level, format, ...) __internal_debug(level, format, ##__VA_ARGS__);
#endif

unsigned char debug;

#endif //_DEBUG_H