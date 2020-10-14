/* SPDX-License-Identifer: GPL-2.0-or-later

Copyright (C) 2014  Vyacheslav Trushkin
Copyright (C) 2020  Boian Bonev

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

#ifndef LIBIOTOP_INTERNALS_H_INCLUDED

	#define LIBIOTOP_INTERNALS_H_INCLUDED

	#include <config.h>

	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif

	#ifndef _POSIX_C_SOURCE
		#define _POSIX_C_SOURCE
	#endif

	#ifndef _DEFAULT_SOURCE
		#define _DEFAULT_SOURCE
	#endif

	#include <libiotop.h>
	#include <stdint.h>
	#include <unistd.h>

	#define PROC_LIST_SZ_INC 1024
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

	struct _iotop {

		union {
			struct {
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
			int opts[iotop_flag_custom];

		} config;

		union {
			struct {
				int iter;
				int delay;
				int pid;
				int user_id;
			} p;
			int params[IOTOP_PARAM_CUSTOM];
		} param;

		int maxpidlen;

		/// @brief Callback pointers.
		struct {
			iotop_presentation_method presentation;
		} callback;

		struct _iotop_view {
			int refresh;
			struct xxxid_stats_arr *ps;
			struct xxxid_stats_arr *cs;
			struct act_stats act;
		} view;

		/// xxxid_info.c
		int nl_sock;
		int nl_fam_id;

	};

	/// @brief Update values.
	int iotop_view_refresh(iotop_view *view);

	/// @brief Global Handle (temporary)
	extern iotop * hSession;

	int filter1(struct xxxid_stats *s);
	void arr_free(struct xxxid_stats_arr *pa);
	void free_stats(struct xxxid_stats *s);

	typedef int (*filter_callback)(struct xxxid_stats *);
	struct xxxid_stats_arr *fetch_data(int processes,filter_callback);

	/* vmstat.c */

	int get_vm_counters(uint64_t *pgpgin,uint64_t *pgpgou);

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

	struct pidgen *openpidgen(int flags);
	void closepidgen(struct pidgen *pg);
	int pidgen_next(struct pidgen *pg);
	char *u8strpadt(const char *s,size_t len);

	void nl_init(iotop *handle);
	void nl_fini(iotop *handle);
	int nl_xxxid_info(pid_t xxxid,struct xxxid_stats *stats);

	/* ioprio.c */
	int get_ioprio(pid_t pid);
	const char *str_ioprio(int io_prio);
	int ioprio_value(int class,int prio);
	int set_ioprio(int which,int who,int ioprio_class,int ioprio_prio);
	int ioprio2class(int ioprio);
	int ioprio2prio(int ioprio);

	char *read_cmdline(int pid,int isshort);

	/* arr.c */

	struct xxxid_stats_arr *arr_alloc(void);
	int arr_add(struct xxxid_stats_arr *a,struct xxxid_stats *s);
	struct xxxid_stats *arr_find(struct xxxid_stats_arr *pa,pid_t tid);
	void arr_sort(struct xxxid_stats_arr *pa,int (*cb)(const void *a,const void *b));

#endif // LIBIOTOP_INTERNALS_H_INCLUDED
