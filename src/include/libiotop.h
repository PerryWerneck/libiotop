/* SPDX-License-Identifer: GPL-2.0-or-later

Copyright (C) 2014  Vyacheslav Trushkin
Copyright (C) 2020  Boian Bonev

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

/**
 * @brief Libiotop common definitions.
 *
 */

#ifndef LIBIOTOP_H_INCLUDED

	#define LIBIOTOP_H_INCLUDED

	#include <unistd.h>
	#include <stdint.h>

	#define IOTOP_API		__attribute__((visibility("default")))
	#define IOTOP_PRIVATE	__attribute__((visibility("hidden")))

	/// @brief Options.
	typedef enum iotop_flag {
		iotop_batch_mode,
		iotop_only,
		iotop_processes,
		iotop_accumulated,
		iotop_kilobytes,
		iotop_timestamp,
		iotop_quiet,
		iotop_nohelp,
		iotop_ullcmdline,
		iotop_hidepid,
		iotop_hideprio,
		iotop_hideuser,
		iotop_hideread,
		iotop_hidewrite,
		iotop_hideswapin,
		iotop_hideio,
		iotop_hidegraph,
		iotop_hidecmd,
		iotop_sort_by,
		iotop_sort_order,

		iotop_flag_custom

	} IOTOP_FLAG;

	typedef enum {
		IOTOP_SORT_BY_PID,
		IOTOP_SORT_BY_PRIO,
		IOTOP_SORT_BY_USER,
		IOTOP_SORT_BY_READ,
		IOTOP_SORT_BY_WRITE,
		IOTOP_SORT_BY_SWAPIN,
		IOTOP_SORT_BY_IO,
		IOTOP_SORT_BY_GRAPH,
		IOTOP_SORT_BY_COMMAND,
		IOTOP_SORT_BY_MAX

	} IOTOP_SORT_OPTION;

	typedef enum {
		IOTOP_SORT_DESC,
		IOTOP_SORT_ASC
	} IOTOP_SORT_ORDER;

	typedef enum {
		IOTOP_PARAM_ITER,
		IOTOP_PARAM_DELAY,
		IOTOP_PARAM_PID,
		IOTOP_PARAM_USER_ID,

		IOTOP_PARAM_CUSTOM
	} IOTOP_PARAM;

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

	#define HISTORY_POS 60
	#define HISTORY_CNT (HISTORY_POS*2)

	struct act_stats {
		uint64_t read_bytes;
		uint64_t write_bytes;
		uint64_t read_bytes_o;
		uint64_t write_bytes_o;
		uint64_t ts_c;
		uint64_t ts_o;
		uint8_t have_o;
	};

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

	struct _iotop_view {
		int refresh;
		struct xxxid_stats_arr *ps;
		struct xxxid_stats_arr *cs;
		struct act_stats act;
	};

	typedef struct _iotop iotop;
	typedef struct _iotop_view iotop_view;

	typedef void (*iotop_presentation_method)(iotop * handle);
	typedef int (*iotop_filter_method)(struct xxxid_stats *s);
	typedef void (*iotop_present_stat)(const struct xxxid_stats *s);

	/// @brief Get active session.
	IOTOP_API iotop * iotop_get_active_session();

	/// @brief Navigate thru stats.
	IOTOP_API void iotop_stat_foreach(iotop *handle, const iotop_present_stat callback);

	/// @brief Set 'view' method.
	IOTOP_API void iotop_set_presentation_method(iotop *hSession, const iotop_presentation_method callback);

	/// @brief Present
	IOTOP_API void iotop_present(iotop *hSession);

	/// @brief Get view.
	IOTOP_API iotop_view * iotop_get_view(iotop *hSession);

	/// @brief Get library version.
	IOTOP_API const char * iotop_get_version();

	/// @brief Initialize a new iotop session.
	IOTOP_API iotop 	* iotop_new();

	/// @brief Ends iotop session, release memory.
	IOTOP_API void	  iotop_free(iotop *handle);

	/// @brief Set flag.
	IOTOP_API void	  iotop_set_flag(iotop *handle, IOTOP_FLAG flag, int value);

	/// @brief Get flag.
	IOTOP_API int 	  iotop_get_flag(iotop *handle, IOTOP_FLAG flag);

	/// @brief Set param.
	//IOTOP_API void	  iotop_set_param(iotop *handle, IOTOP_PARAM param, int value);

	/// @brief Get param.
	//IOTOP_API int 	  iotop_get_param(iotop *handle, IOTOP_PARAM param);

	/// @brief Update values.
	IOTOP_API int iotop_refresh(iotop *handle, int processes, const iotop_filter_method filter);

	struct xxxid_stats *arr_find(struct xxxid_stats_arr *pa,pid_t tid);

	IOTOP_API int iotop_get_diff_len(iotop *hSession);

	/* ioprio.c */
	int get_ioprio(pid_t pid);
	const char *str_ioprio(int io_prio);
	int ioprio_value(int class,int prio);
	int set_ioprio(int which,int who,int ioprio_class,int ioprio_prio);
	int ioprio2class(int ioprio);
	int ioprio2prio(int ioprio);

#endif // LIBIOTOP_H_INCLUDED
