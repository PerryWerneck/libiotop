/* SPDX-License-Identifer: GPL-2.0-or-later

Copyright (C) 2014  Vyacheslav Trushkin
Copyright (C) 2020  Boian Bonev

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

#ifndef LIBIOTOP_INTERNALS_H_INCLUDED

	#define LIBIOTOP_INTERNALS_H_INCLUDED

	#include <libiotop.h>

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

		struct {
			int iter;
			int delay;
			int pid;
			int user_id;
		} params;

		int maxpidlen;

	};

	/// @brief Global Handle (temporary)
	extern iotop * hSession;


#endif // LIBIOTOP_INTERNALS_H_INCLUDED
