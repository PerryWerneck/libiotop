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

#include <sys/types.h>
#include <stdint.h>

#include <libiotop-internals.h> // TODO: Remove it after migration

#define VERSION "1.15"

/*
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
*/

/*
typedef struct {
	int iter;
	int delay;
	int pid;
	int user_id;
} params_t;
*/

//extern config_t config;
//extern params_t params;
//extern int maxpidlen;


#define HISTORY_POS 60
#define HISTORY_CNT (HISTORY_POS*2)

struct xxxid_stats {
	pid_t tid;
	uint64_t swapin_delay_total; // nanoseconds
	uint64_t blkio_delay_total; // nanoseconds
	uint64_t read_bytes;
	uint64_t write_bytes;

	double blkio_val;
	double swapin_val;
	double read_val;
	double write_val;
	double read_val_acc;
	double write_val_acc;

	int io_prio;

	int euid;
	char *cmdline1;
	char *cmdline2;
	char *pw_name;

	uint8_t iohist[HISTORY_CNT];
};

#define PROC_LIST_SZ_INC 1024

struct xxxid_stats_arr {
	struct xxxid_stats **arr;
	struct xxxid_stats **sor;
	int length;
	int size;
};

struct act_stats {
	uint64_t read_bytes;
	uint64_t write_bytes;
	uint64_t read_bytes_o;
	uint64_t write_bytes_o;
	uint64_t ts_c;
	uint64_t ts_o;
	uint8_t have_o;
};

void nl_init(void);
void nl_fini(void);

int nl_xxxid_info(pid_t xxxid,struct xxxid_stats *stats);

typedef int (*filter_callback)(struct xxxid_stats *);

struct xxxid_stats_arr *fetch_data(int processes,filter_callback);
void free_stats(struct xxxid_stats *s);

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

/* utils.c */

enum {
	PIDGEN_FLAGS_PROC,
	PIDGEN_FLAGS_TASK
};

struct pidgen {
	void *__proc;
	void *__task;
	int __flags;
};

char *read_cmdline(int pid,int isshort);

struct pidgen *openpidgen(int flags);
void closepidgen(struct pidgen *pg);
int pidgen_next(struct pidgen *pg);
int64_t monotime(void);
char *u8strpadt(const char *s,size_t len);

/* ioprio.c */

enum {
	IOPRIO_CLASS_NONE,
	IOPRIO_CLASS_RT,
	IOPRIO_CLASS_BE,
	IOPRIO_CLASS_IDLE,
	IOPRIO_CLASS_MAX,
	IOPRIO_CLASS_MIN=IOPRIO_CLASS_RT,
};

enum {
	IOPRIO_WHO_PROCESS=1,
	IOPRIO_WHO_PGRP,
	IOPRIO_WHO_USER
};

enum {
	SORT_BY_PID,
	SORT_BY_PRIO,
	SORT_BY_USER,
	SORT_BY_READ,
	SORT_BY_WRITE,
	SORT_BY_SWAPIN,
	SORT_BY_IO,
	SORT_BY_GRAPH,
	SORT_BY_COMMAND,
	SORT_BY_MAX
};

enum {
	SORT_DESC,
	SORT_ASC
};

extern const char *str_ioprio_class[];

int get_ioprio(pid_t pid);
const char *str_ioprio(int io_prio);
int ioprio_value(int class,int prio);
int set_ioprio(int which,int who,int ioprio_class,int ioprio_prio);
int ioprio2class(int ioprio);
int ioprio2prio(int ioprio);

/* vmstat.c */

int get_vm_counters(uint64_t *pgpgin,uint64_t *pgpgou);

/* checks.c */

int system_checks(void);

/* arr.c */

struct xxxid_stats_arr *arr_alloc(void);
int arr_add(struct xxxid_stats_arr *a,struct xxxid_stats *s);
struct xxxid_stats *arr_find(struct xxxid_stats_arr *pa,pid_t tid);
void arr_free(struct xxxid_stats_arr *pa);
void arr_sort(struct xxxid_stats_arr *pa,int (*cb)(const void *a,const void *b));

#define HEADER1_FORMAT "  Total DISK READ: %7.2f %s%s |   Total DISK WRITE: %7.2f %s%s"
#define HEADER2_FORMAT "Current DISK READ: %7.2f %s%s | Current DISK WRITE: %7.2f %s%s"

void calc_total(struct xxxid_stats_arr *cs,double *read,double *write);
void calc_a_total(struct act_stats *act,double *read,double *write,double time_s);
void humanize_val(double *value,char *str,int allow_accum);
int iotop_sort_cb(const void *a,const void *b);
int create_diff(struct xxxid_stats_arr *cs,struct xxxid_stats_arr *ps,double time_s);
int value2scale(double val,double mx);
int filter1(struct xxxid_stats *s);

#ifndef KEY_CTRL_L
#define KEY_CTRL_L 014
#endif

#endif // __IOTOP_H__

