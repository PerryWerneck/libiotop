/* SPDX-License-Identifer: GPL-2.0-or-later

Copyright (C) 2014  Vyacheslav Trushkin
Copyright (C) 2020  Boian Bonev

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

/**
 * @brief Implement library internal sort.
 *
 */

#include "iotop.h"
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

struct sort_args {
	IOTOP_SORT_OPTION	type;
	int					order;
	int					grlen;
};

int compare(const struct xxxid_stats **ppa,struct xxxid_stats **ppb,struct sort_args *args) {
	const struct xxxid_stats *pa,*pb;
	int res=0;

	if (!ppa) {
		args->grlen=(long)ppb;
		return 0;
	}

	pa=*ppa;
	pb=*ppb;

	switch (args->type) {
		case IOTOP_SORT_BY_GRAPH: {
			int aa=0,ab=0;
			int i;

			if (args->grlen==0)
				args->grlen=HISTORY_CNT;
			for (i=0;i<args->grlen;i++) {
				aa+=pa->iohist[i];
				ab+=pb->iohist[i];
			}
			res=aa-ab;
			break;
		}
		case IOTOP_SORT_BY_PRIO:
			res=pa->io_prio-pb->io_prio;
			break;
		case IOTOP_SORT_BY_COMMAND:
			res=strcmp(config.f.fullcmdline?pa->cmdline2:pa->cmdline1,config.f.fullcmdline?pb->cmdline2:pb->cmdline1);
			break;
		case IOTOP_SORT_BY_PID:
			res=pa->tid-pb->tid;
			break;
		case IOTOP_SORT_BY_USER:
			res=strcmp(pa->pw_name,pb->pw_name);
			break;
		case IOTOP_SORT_BY_READ:
			if (config.f.accumulated)
				res=pa->read_val_acc>pb->read_val_acc?1:pa->read_val_acc<pb->read_val_acc?-1:0;
			else
				res=pa->read_val>pb->read_val?1:pa->read_val<pb->read_val?-1:0;
			break;
		case IOTOP_SORT_BY_WRITE:
			if (config.f.accumulated)
				res=pa->write_val_acc>pb->write_val_acc?1:pa->write_val_acc<pb->write_val_acc?-1:0;
			else
				res=pa->write_val>pb->write_val?1:pa->write_val<pb->write_val?-1:0;
			break;
		case IOTOP_SORT_BY_SWAPIN:
			res=pa->swapin_val>pb->swapin_val?1:pa->swapin_val<pb->swapin_val?-1:0;
			break;
		case IOTOP_SORT_BY_IO:
			res=pa->blkio_val>pb->blkio_val?1:pa->blkio_val<pb->blkio_val?-1:0;
			break;

		case IOTOP_SORT_BY_MAX:
			break;

	}
	res*=args->order;
	return res;
}


void iotop_sort_stats(struct xxxid_stats_arr *pa, IOTOP_SORT_OPTION option, IOTOP_SORT_ORDER order, int grlen) {

	if (!pa)
		return;

	if (pa->sor)
		free(pa->sor);

	pa->sor=NULL;

	if (!pa->length)
		return;

	pa->sor=calloc(pa->length,sizeof *pa->arr);

	if (!pa->sor)
		return;

	memcpy(pa->sor,pa->arr,pa->length*sizeof *pa->arr);

	struct sort_args args = {
		.grlen = grlen,
		.type = option,
		.order = (order ? 1 : -1) // SORT_ASC is bit 0=1, else should reverse sort
	};

	qsort_r(
		pa->sor,
		pa->length,
		sizeof *pa->sor,
		(int (*)(const void *, const void *, void *)) compare,
		&args
	);

}

