/* SPDX-License-Identifer: GPL-2.0-or-later

Copyright (C) 2014  Vyacheslav Trushkin
Copyright (C) 2020  Boian Bonev

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

#ifndef __IOTOP_H__
#define __IOTOP_H__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE
#endif
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <config.h>
#include <sys/types.h>
#include <stdint.h>
#include <libiotop.h>

typedef union {
	struct _flags {
		int batch_mode;
		int only;
		int processes;
		int accumulated;
		int kilobytes;
		int timestamp;
		int quiet;
		int nohelp;
		int fullcmdline;
		int hidepid;
		int hideprio;
		int hideuser;
		int hideread;
		int hidewrite;
		int hideswapin;
		int hideio;
		int hidegraph;
		int hidecmd;
		int sort_by;
		int sort_order;
	} f;
	int opts[18];
} config_t;

typedef struct {
	int iter;
	int delay;
	int pid;
	int user_id;
} params_t;

extern config_t config;
extern params_t params;
extern int maxpidlen;

typedef void (*view_loop)(void);
typedef void (*view_init)(void);
typedef void (*view_fini)(void);

void view_batch_loop(void);
void view_batch_init(void);
void view_batch_fini(void);

void view_curses_loop(void);
void view_curses_init(void);
void view_curses_fini(void);

unsigned int curses_sleep(unsigned int seconds);

extern const char *str_ioprio_class[];

/* checks.c */

int system_checks(void);

#define HEADER1_FORMAT "  Total DISK READ: %7.2f %s%s |   Total DISK WRITE: %7.2f %s%s"
#define HEADER2_FORMAT "Current DISK READ: %7.2f %s%s | Current DISK WRITE: %7.2f %s%s"

void calc_total(const struct xxxid_stats_arr *cs,double *read,double *write);
void calc_a_total(struct act_stats *act,double *read,double *write,double time_s);
void humanize_val(double *value,char *str,int allow_accum);
int iotop_sort_cb(const void *a,const void *b);
int create_diff(struct xxxid_stats_arr *cs, struct xxxid_stats_arr *ps,double time_s);
int value2scale(double val,double mx);

int64_t monotime(void);
int filter1(struct xxxid_stats *s);
void iotop_sort_stats(struct xxxid_stats_arr *cs, IOTOP_SORT_OPTION option, IOTOP_SORT_ORDER order, int grlen);
char *u8strpadt(const char *s,size_t len);

#ifndef KEY_CTRL_L
#define KEY_CTRL_L 014
#endif

#endif // __IOTOP_H__

