/* SPDX-License-Identifer: GPL-2.0-or-later

Copyright (C) 2014  Vyacheslav Trushkin
Copyright (C) 2020  Boian Bonev

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

/**
 * @brief Update stats.
 *
 */

 #include <libiotop-internals.h>

 int iotop_view_refresh(iotop_view *view, int processes, const iotop_filter_method filter) {

	if (view->ps)
		arr_free(view->ps);

	view->ps = view->cs;
	view->act.read_bytes_o=view->act.read_bytes;
	view->act.write_bytes_o=view->act.write_bytes;
	if (view->act.ts_c)
		view->act.have_o=1;
	view->act.ts_o=view->act.ts_c;

	view->cs=fetch_data(processes,filter);
	if (!view->ps) {
		view->ps=view->cs;
		view->cs=fetch_data(processes,filter);
	}
	get_vm_counters(&view->act.read_bytes,&view->act.write_bytes);

	return view->refresh=1;
}

