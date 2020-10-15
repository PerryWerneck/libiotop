/* SPDX-License-Identifer: GPL-2.0-or-later

Copyright (C) 2014  Vyacheslav Trushkin
Copyright (C) 2020  Boian Bonev

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

/**
 * @brief Create/destroy iotop handle.
 *
 */

 #include <libiotop-internals.h>
 #include <malloc.h>
 #include <string.h> // memset

 // TODO: Remove it after migration.
 iotop * hSession = NULL;

 iotop * iotop_get_active_session() {
	return hSession;
 }

 iotop * iotop_new() {

	// Allocate handle
	iotop * handle = malloc(sizeof(iotop));
	memset(handle,00,sizeof(iotop));

	handle->nl_sock = -1;
	nl_init(handle);

	// Store on global
	hSession = handle;

	// And return it.
	return handle;

 }

 void iotop_free(iotop *handle) {

	nl_fini(handle);
	free(handle);
 }

