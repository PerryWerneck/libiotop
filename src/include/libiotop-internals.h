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
	#include <libiotop.h>
	#include <stdint.h>

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

	struct _iotop_view {
		int refresh;
		struct xxxid_stats_arr *ps;
		struct xxxid_stats_arr *cs;
		struct act_stats act;
	};

	/// @brief Global Handle (temporary)
	extern iotop * hSession;


#endif // LIBIOTOP_INTERNALS_H_INCLUDED
