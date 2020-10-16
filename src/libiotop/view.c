/* SPDX-License-Identifer: GPL-2.0-or-later

Copyright (C) 2014  Vyacheslav Trushkin
Copyright (C) 2020  Boian Bonev

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

/**
 * @brief Visualization tools.
 *
 */

 #include <libiotop-internals.h>
 #include <string.h>
 #include <stdio.h>

 #define RRV(to,from) (((to)<(from))?(~0ULL)-(to)+(from):(to)-(from))
 #define RRVf(pto,pfrom,fld) RRV(pto->fld,pfrom->fld)

 double iotop_get_time_s(iotop *hSession) {

	#define TIMEDIFF_IN_S(sta,end) ((((sta)==(end))||(sta)==0)?0.0001:(((end)-(sta))/1000.0))
 	return TIMEDIFF_IN_S(hSession->view.act.ts_o,hSession->view.act.ts_c);

 }

 static int value2scale(double val,double mx) {
	val=100.0*val/mx;

	if (val>75)
		return 4;
	if (val>50)
		return 3;
	if (val>25)
		return 2;
	if (val>0)
		return 1;
	return 0;
 }

 static int create_diff(iotop *hSession, struct xxxid_stats_arr *cs, struct xxxid_stats_arr *ps,double time_s) {

	// @todo (perry#5#): Move the entire code to iotop_get_diff_len

	int diff_size=cs->length;
	int n=0;

	for (n=0;cs->arr&&n<cs->length;n++) {
		struct xxxid_stats *c;
		struct xxxid_stats *p;
		double rv,wv;
		char temp[12];

		c=cs->arr[n];
		p=arr_find(ps,c->tid);

		if (!p) { // new process or task
			c->blkio_val=0;
			c->swapin_val=0;
			c->read_val=0;
			c->write_val=0;
			c->read_val_acc=0;
			c->write_val_acc=0;

			snprintf(temp,sizeof temp,"%i",c->tid);
			hSession->maxpidlen = hSession->maxpidlen<(int)strlen(temp)?(int)strlen(temp):hSession->maxpidlen;
			continue;
		}

		// round robin value
		c->blkio_val=(double)RRVf(c,p,blkio_delay_total)/(time_s*10000000.0);
		if (c->blkio_val>100)
			c->blkio_val=100;

		c->swapin_val=(double)RRVf(c,p,swapin_delay_total)/(time_s*10000000.0);
		if (c->swapin_val>100)
			c->swapin_val=100;

		rv=(double)RRVf(c,p,read_bytes);
		wv=(double)RRVf(c,p,write_bytes);

		c->read_val=rv/time_s;
		c->write_val=wv/time_s;

		c->read_val_acc=p->read_val_acc+rv;
		c->write_val_acc=p->write_val_acc+wv;

		memcpy(c->iohist+1,p->iohist,sizeof c->iohist-sizeof *c->iohist);
		c->iohist[0]=value2scale(c->blkio_val,100.0);

		snprintf(temp,sizeof temp,"%i",c->tid);
		hSession->maxpidlen = hSession->maxpidlen<(int)strlen(temp)?(int)strlen(temp) : hSession->maxpidlen;
	}

	return diff_size;
 }

 void iotop_set_presentation_method(iotop *hSession, iotop_presentation_method callback) {
	hSession->callback.presentation = callback;
 }

 void iotop_present(iotop *hSession) {
	if(hSession->callback.presentation) {
		hSession->callback.presentation(hSession);
	}
	hSession->view.refresh=0;
 }

 int iotop_get_diff_len(iotop *hSession) {
 	double time_s=iotop_get_time_s(hSession);
	return create_diff(hSession, hSession->view.cs, hSession->view.ps, time_s);
}

 void iotop_stat_foreach(iotop *hSession, const iotop_present_stat callback) {

	int i;

	int diff_len= iotop_get_diff_len(hSession);

	for (i=0; hSession->view.cs->sor && i < diff_len; i++) {
		callback(hSession->view.cs->sor[i]);
	}

 }

