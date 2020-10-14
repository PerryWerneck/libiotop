/* SPDX-License-Identifer: GPL-2.0-or-later

Copyright (C) 2014  Vyacheslav Trushkin
Copyright (C) 2020  Boian Bonev

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

/**
 * @brief Get/Set iotop properties.
 *
 */

 #include <libiotop-internals.h>

 void iotop_set_flag(iotop *handle, IOTOP_FLAG flag, int value) {
	handle->config.opts[flag] = value;
 }

 int iotop_get_flag(iotop *handle, IOTOP_FLAG flag) {
 	return handle->config.opts[flag];
 }

 const char * iotop_get_version() {
	return PACKAGE_VERSION;
 }

 void iotop_set_param(iotop *handle, IOTOP_PARAM param, int value) {
	handle->param.params[param] = value;
 }

 void iotop_set_presentation_method(iotop *hSession, iotop_presentation_method callback) {
	hSession->callback.presentation = callback;
 }

 int iotop_get_param(iotop *handle, IOTOP_PARAM param) {
 	return handle->param.params[param];
 }

 void iotop_present(iotop *hSession) {
	if(hSession->callback.presentation) {
		hSession->callback.presentation(&hSession->view);
	}
	hSession->view.refresh=0;
 }
