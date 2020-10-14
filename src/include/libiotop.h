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

	typedef struct _iotop iotop;
	typedef struct _iotop_view iotop_view;
	typedef void  (*iotop_update_callback)(iotop_view *view);

	/// @brief Set 'view' method.
	void iotop_set_update_callback(iotop *hSession, iotop_update_callback callback);

	/// @brief Get library version.
	const char * iotop_get_version();

	/// @brief Initialize a new iotop session.
	iotop 	* iotop_new();

	/// @brief Ends iotop session, release memory.
	void	  iotop_free(iotop *handle);

	/// @brief Set flag.
	void	  iotop_set_flag(iotop *handle, IOTOP_FLAG flag, int value);

	/// @brief Get flag.
	int 	  iotop_get_flag(iotop *handle, IOTOP_FLAG flag);

	/// @brief Set param.
	void	  iotop_set_param(iotop *handle, IOTOP_PARAM param, int value);

	/// @brief Get param.
	int 	  iotop_get_param(iotop *handle, IOTOP_PARAM param);




#endif // LIBIOTOP_H_INCLUDED