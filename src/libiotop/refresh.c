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
 #include <time.h>

 int iotop_refresh(iotop *hSession, int processes, const iotop_filter_method filter) {

	if (hSession->view.ps)
		arr_free(hSession->view.ps);

	hSession->view.ps = hSession->view.cs;
	hSession->view.act.read_bytes_o=hSession->view.act.read_bytes;
	hSession->view.act.write_bytes_o=hSession->view.act.write_bytes;
	if (hSession->view.act.ts_c)
		hSession->view.act.have_o=1;
	hSession->view.act.ts_o=hSession->view.act.ts_c;

	hSession->view.cs=fetch_data(processes,filter);
	if (!hSession->view.ps) {
		hSession->view.ps=hSession->view.cs;
		hSession->view.cs=fetch_data(processes,filter);
	}

	get_vm_counters(&hSession->view.act.read_bytes,&hSession->view.act.write_bytes);

	// Monotime
	{
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC,&ts);
		hSession->view.act.ts_c = ts.tv_sec*1000;
		hSession->view.act.ts_c += ts.tv_nsec/1000000;
	}

	return hSession->view.refresh=1;

}

