/*
 * This file is part of FeatureHashing
 * Copyright (C) 2015 Wush Wu
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bswap_32.h"

#ifdef BSWAP_32_IMPLEMENTATION
uint32_t bswap_32(uint32_t x) {
  return ((x & 0xff000000) >> 24) |
         ((x & 0x00ff0000) >>  8) |
         ((x & 0x0000ff00) <<  8) |
         ((x & 0x000000ff) << 24);
}
#endif //BSWAP_32_IMPLEMENTATION
