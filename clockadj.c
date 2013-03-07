/**
 * @file clockadj.c
 * @note Copyright (C) 2013 Richard Cochran <richardcochran@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <string.h>

#include "clockadj.h"
#include "missing.h"
#include "print.h"

#define NS_PER_SEC 1000000000LL

void clockadj_set_freq(clockid_t clkid, double freq)
{
	struct timex tx;
	memset(&tx, 0, sizeof(tx));
	tx.modes = ADJ_FREQUENCY;
	tx.freq = (long) (freq * 65.536);
	if (clock_adjtime(clkid, &tx) < 0)
		pr_err("failed to adjust the clock: %m");
}

double clockadj_get_freq(clockid_t clkid)
{
	double f = 0.0;
	struct timex tx;
	memset(&tx, 0, sizeof(tx));
	if (clock_adjtime(clkid, &tx) < 0)
		pr_err("failed to read out the clock frequency adjustment: %m");
	else
		f = tx.freq / 65.536;
	return f;
}

void clockadj_step(clockid_t clkid, int64_t step)
{
	struct timex tx;
	int sign = 1;
	if (step < 0) {
		sign = -1;
		step *= -1;
	}
	memset(&tx, 0, sizeof(tx));
	tx.modes = ADJ_SETOFFSET | ADJ_NANO;
	tx.time.tv_sec  = sign * (step / NS_PER_SEC);
	tx.time.tv_usec = sign * (step % NS_PER_SEC);
	/*
	 * The value of a timeval is the sum of its fields, but the
	 * field tv_usec must always be non-negative.
	 */
	if (tx.time.tv_usec < 0) {
		tx.time.tv_sec  -= 1;
		tx.time.tv_usec += 1000000000;
	}
	if (clock_adjtime(clkid, &tx) < 0)
		pr_err("failed to step clock: %m");
}
