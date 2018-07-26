/**
 * @file common.c
 * @author Salvatore Barone
 * @email salvator.barone@gmail.com
 * @date 15 08 2016
 *
 * @copyright
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the License, or any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "common.h"

void DelayUS(uint32_t us) {
		uint32_t multiplier = CLOCK_FREQUENCY_HZ / 4000000;
	    us = us * multiplier;
	    while (us--);
}
