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

	struct _iotop {

		int maxpidlen;

		/// @brief Callback pointers.
		struct {
			iotop_presentation_method presentation;
		} callback;

		iotop_view view;

		/// xxxid_info.c
		int nl_sock;
		int nl_fam_id;

	};

	/// @brief Global Handle (temporary)
	extern iotop * hSession;

	void arr_free(struct xxxid_stats_arr *pa);
	void free_stats(struct xxxid_stats *s);

	typedef int (*filter_callback)(struct xxxid_stats *);
	struct xxxid_stats_arr *fetch_data(int processes,filter_callback);

	/* vmstat.c */

	int get_vm_counters(uint64_t *pgpgin,uint64_t *pgpgou);

	/* ioprio.c */

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

	void nl_init(iotop *handle);
	void nl_fini(iotop *handle);
	int nl_xxxid_info(pid_t xxxid,struct xxxid_stats *stats);

	char *read_cmdline(int pid,int isshort);

	/* arr.c */

	struct xxxid_stats_arr *arr_alloc(void);
	int arr_add(struct xxxid_stats_arr *a,struct xxxid_stats *s);
	// void arr_sort(struct xxxid_stats_arr *pa,int (*cb)(const void *a,const void *b));

#endif // LIBIOTOP_INTERNALS_H_INCLUDED
