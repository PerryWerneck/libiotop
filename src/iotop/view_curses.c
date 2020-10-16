/* SPDX-License-Identifer: GPL-2.0-or-later

Copyright (C) 2014  Vyacheslav Trushkin
Copyright (C) 2020  Boian Bonev

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

#include "iotop.h"

// allow ncurses printf-like arguments checking
#define GCC_PRINTF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>
#include <locale.h>
#include <langinfo.h>

#define HEADER_XXS_FORMAT "%4.0f%s%s/%4.0f%s%s|%4.0f%s%s/%4.0f%s%s"
#define HEADER_XS_FORMAT "TR:%4.0f%s%sW:%4.0f%s%s|CR:%4.0f%s%sW:%4.0f%s%s"
#define HEADER_S_FORMAT "T R:%7.2f%s%s W:%7.2f%s%s|C R:%7.2f%s%s W:%7.2f%s%s"
#define HEADER_M_FORMAT "T Read:%7.2f%s%s Write:%7.2f%s%s|C Read:%7.2f%s%s Write:%7.2f%s%s"
#define HEADER_L_FORMAT "Total Read:%7.2f %s%s Write:%7.2f %s%s|Current Read:%7.2f %s%s Write:%7.2f %s%s"
#define HEADER_XL_FORMAT "Total Read: %7.2f %s%s Write: %7.2f %s%s | Current Read: %7.2f %s%s Write: %7.2f %s%s"

static int in_ionice=0;
static char ionice_id[50];
static int ionice_cl=1; // select what to edit class(1) or prio(0)
static int ionice_class=IOPRIO_CLASS_RT;
static int ionice_prio=0;
static int ionice_id_changed=0;
static int has_unicode=0;
static int unicode=1;
static double hist_t_r[HISTORY_CNT]={0};
static double hist_t_w[HISTORY_CNT]={0};
static double hist_a_r[HISTORY_CNT]={0};
static double hist_a_w[HISTORY_CNT]={0};

static const char *column_name[]={
	"xID", // unused, value varies - PID or TID
	"PRIO",
	"USER",
	"DISK READ",
	"DISK WRITE",
	"SWAPIN",
	"IO",
	"GRAPH",
	"COMMAND",
};

// Braille unicode pseudo graph - 2x5 levels graph per character
static const char *br_graph[5][5]={
	{"⠀","⢀","⢠","⢰","⢸",},
	{"⡀","⣀","⣠","⣰","⣸",},
	{"⡄","⣄","⣤","⣴","⣼",},
	{"⡆","⣆","⣦","⣶","⣾",},
	{"⡇","⣇","⣧","⣷","⣿",},
};

// ASCII pseudo graph - 1x5 levels graph per character
static const char *as_graph[5]={" ","_",".",":","|",};

static const int column_width[]={
	0,  // PID/TID
	5,  // PRIO
	10, // USER
	12, // READ
	12, // WRITE
	9,  // SWAPIN
	9,  // IO
	0,  // GRAPH
	0,  // COMMAND
};

#define __COLUMN_NAME(i) ((i)==0?(config.f.processes?"PID":"TID"):column_name[(i)])
#define __SAFE_INDEX(i) ((((i)%IOTOP_SORT_BY_MAX)+IOTOP_SORT_BY_MAX)%IOTOP_SORT_BY_MAX)
#define COLUMN_NAME(i) __COLUMN_NAME(__SAFE_INDEX(i))
#define COLUMN_L(i) COLUMN_NAME((i)-1)
#define COLUMN_R(i) COLUMN_NAME((i)+1)
#define SORT_CHAR(x) ((config.f.sort_by==x)?(config.f.sort_order==IOTOP_SORT_ASC?'<':'>'):' ')

#define TIMEDIFF_IN_S(sta,end) ((((sta)==(end))||(sta)==0)?0.0001:(((end)-(sta))/1000.0))

static void view_curses(struct xxxid_stats_arr *cs, struct xxxid_stats_arr *ps,struct act_stats *act,int roll) {
	double time_s=TIMEDIFF_IN_S(act->ts_o,act->ts_c);
	static const uint8_t iohist_z[HISTORY_CNT]={0};

//	int diff_len=create_diff(cs,ps,time_s);
	int diff_len = iotop_get_diff_len(iotop_get_active_session());


	double total_read,total_write;
	double total_a_read,total_a_write;
	char pg_t_r[HISTORY_POS*5]={0};
	char pg_t_w[HISTORY_POS*5]={0};
	char pg_a_r[HISTORY_POS*5]={0};
	char pg_a_w[HISTORY_POS*5]={0};
	char str_read[4],str_write[4];
	char str_a_read[4],str_a_write[4];
	char *head1row_format="";
	int promptx=0,prompty=0,show;
	int nohelp=config.f.nohelp;
	double mx_t_r=1000.0;
	double mx_t_w=1000.0;
	double mx_a_r=1000.0;
	double mx_a_w=1000.0;
	int line,lastline;
	int ionicepos=1;
	int shrink_dm=0;
	int head1row=0;
	int maxcmdline;
	int gr_width_h;
	int gr_width;
	int maxy;
	int maxx;
	int i,j;

	maxy=getmaxy(stdscr);
	maxx=getmaxx(stdscr);

	calc_total(cs,&total_read,&total_write);
	calc_a_total(act,&total_a_read,&total_a_write,time_s);

	if (roll) {
		memmove(hist_t_r+1,hist_t_r,sizeof hist_t_r-sizeof *hist_t_r);
		memmove(hist_t_w+1,hist_t_w,sizeof hist_t_w-sizeof *hist_t_w);
		memmove(hist_a_r+1,hist_a_r,sizeof hist_a_r-sizeof *hist_a_r);
		memmove(hist_a_w+1,hist_a_w,sizeof hist_a_w-sizeof *hist_a_w);
		hist_t_r[0]=total_read;
		hist_t_w[0]=total_write;
		hist_a_r[0]=total_a_read;
		hist_a_w[0]=total_a_write;
	}

	humanize_val(&total_read,str_read,1);
	humanize_val(&total_write,str_write,1);
	humanize_val(&total_a_read,str_a_read,0);
	humanize_val(&total_a_write,str_a_write,0);

	maxcmdline=maxx;
	if (!config.f.hidepid)
		maxcmdline-=maxpidlen+2;
	if (!config.f.hideprio)
		maxcmdline-=column_width[IOTOP_SORT_BY_PRIO];
	if (!config.f.hideuser)
		maxcmdline-=column_width[IOTOP_SORT_BY_USER];
	if (!config.f.hideread)
		maxcmdline-=column_width[IOTOP_SORT_BY_READ];
	if (!config.f.hidewrite)
		maxcmdline-=column_width[IOTOP_SORT_BY_WRITE];
	if (!config.f.hideswapin)
		maxcmdline-=column_width[IOTOP_SORT_BY_SWAPIN];
	if (!config.f.hideio)
		maxcmdline-=column_width[IOTOP_SORT_BY_IO];
	gr_width=maxcmdline/4;
	if (gr_width<5)
		gr_width=5;
	if (gr_width>HISTORY_POS)
		gr_width=HISTORY_POS;
	if (!config.f.hidegraph)
		maxcmdline-=gr_width+1;
	if (maxcmdline<0)
		maxcmdline=0;

	gr_width_h=gr_width;
	if (maxy<10||maxx<(int)strlen(HEADER1_FORMAT)+2*(7-5+3-2+(!config.f.hidegraph?gr_width_h+1:0)-2)) {
		int size_off;

		head1row=1;

		gr_width_h/=2;
		if (gr_width_h<3)
			gr_width_h=3;

		size_off=7-5+3-2+(!config.f.hidegraph?gr_width_h+1:0)-2;

		if (maxx>=(int)strlen(HEADER_XL_FORMAT)+4*size_off)
			head1row_format=HEADER_XL_FORMAT;
		else
			if (maxx>=(int)strlen(HEADER_L_FORMAT)+4*size_off)
				head1row_format=HEADER_L_FORMAT;
			else
				if (maxx>=(int)strlen(HEADER_M_FORMAT)+4*size_off)
					head1row_format=HEADER_M_FORMAT;
				else
					if (maxx>=(int)strlen(HEADER_S_FORMAT)+4*(size_off-2)) {
						head1row_format=HEADER_S_FORMAT;
						shrink_dm=1;
						size_off-=2;
					} else
						if (maxx>=(int)strlen(HEADER_XS_FORMAT)+4*(size_off-5)) {
							head1row_format=HEADER_XS_FORMAT;
							shrink_dm=1;
							size_off-=5;
						} else {
							head1row_format=HEADER_XXS_FORMAT;
							shrink_dm=1;
							size_off-=5;
						}
		if (!config.f.hidegraph)
			while (gr_width_h<gr_width&&maxx>=(int)strlen(head1row_format)+4*(size_off+1)) {
				size_off++;
				gr_width_h++;
			}
	}

	for (i=0;i<((has_unicode&&unicode)?gr_width_h*2:gr_width_h);i++) {
		if (mx_t_r<hist_t_r[i])
			mx_t_r=hist_t_r[i];
		if (mx_t_w<hist_t_w[i])
			mx_t_w=hist_t_w[i];
		if (mx_a_r<hist_a_r[i])
			mx_a_r=hist_a_r[i];
		if (mx_a_w<hist_a_w[i])
			mx_a_w=hist_a_w[i];
	}
	strcpy(pg_t_r," ");
	strcpy(pg_t_w," ");
	strcpy(pg_a_r," ");
	strcpy(pg_a_w," ");
	for (j=0;j<gr_width_h;j++) {
		if (has_unicode&&unicode) {
			strcat(pg_t_r,br_graph[value2scale(hist_t_r[j*2],mx_t_r)][value2scale(hist_t_r[j*2+1],mx_t_r)]);
			strcat(pg_t_w,br_graph[value2scale(hist_t_w[j*2],mx_t_w)][value2scale(hist_t_w[j*2+1],mx_t_w)]);
			strcat(pg_a_r,br_graph[value2scale(hist_a_r[j*2],mx_a_r)][value2scale(hist_a_r[j*2+1],mx_a_r)]);
			strcat(pg_a_w,br_graph[value2scale(hist_a_w[j*2],mx_a_w)][value2scale(hist_a_w[j*2+1],mx_a_w)]);
		} else {
			strcat(pg_t_r,as_graph[value2scale(hist_t_r[j],mx_t_r)]);
			strcat(pg_t_w,as_graph[value2scale(hist_t_w[j],mx_t_w)]);
			strcat(pg_a_r,as_graph[value2scale(hist_a_r[j],mx_a_r)]);
			strcat(pg_a_w,as_graph[value2scale(hist_a_w[j],mx_a_w)]);
		}
	}

	if (head1row) {
		ionicepos=0;
		if (!in_ionice) {
			if (shrink_dm) {
				str_read[1]=0;
				str_write[1]=0;
				str_a_read[1]=0;
				str_a_write[1]=0;
			}
			mvhline(0,0,' ',maxx);
			mvprintw(0,0,head1row_format,total_read,str_read,
						!config.f.hidegraph?pg_t_r:"",total_write,str_write,
						!config.f.hidegraph?pg_t_w:"",total_a_read,str_a_read,
						!config.f.hidegraph?pg_a_r:"",total_a_write,str_a_write,
						!config.f.hidegraph?pg_a_w:"");
			show=FALSE;
		}
	} else {
		mvhline(0,0,' ',maxx);
		mvprintw(0,0,HEADER1_FORMAT,total_read,str_read,
				!config.f.hidegraph?pg_t_r:"",total_write,str_write,
				!config.f.hidegraph?pg_t_w:"");

		if (!in_ionice) {
			mvhline(1,0,' ',maxx);
			mvprintw(1,0,HEADER2_FORMAT,total_a_read,str_a_read,
						!config.f.hidegraph?pg_a_r:"",total_a_write,str_a_write,
						!config.f.hidegraph?pg_a_w:"");
			show=FALSE;
		}
	}
	if (in_ionice) {
		mvhline(ionicepos,0,' ',maxx);
		mvprintw(ionicepos,0,"%s: ",COLUMN_NAME(0));
		attron(A_BOLD);
		printw("%s",ionice_id);
		attroff(A_BOLD);
		getyx(stdscr,promptx,prompty);

		if (strlen(ionice_id)) {
			struct xxxid_stats *p=NULL;
			pid_t id=atoi(ionice_id);

			if (id&&(p=arr_find(cs,id))) {
				printw(" Current: ");
				attron(A_BOLD);
				printw("%s",str_ioprio(p->io_prio));
				attroff(A_BOLD);
				printw(" Change to: ");

				if (ionice_id_changed) {
					ionice_id_changed=0;
					ionice_class=ioprio2class(p->io_prio);
					ionice_prio=ioprio2prio(p->io_prio);
				}

				attron(A_BOLD);
				if (ionice_cl)
					attron(A_REVERSE);
				printw("%s",str_ioprio_class[ionice_class]);
				if (ionice_cl)
					attroff(A_REVERSE);
				printw("/");
				if (!ionice_cl)
					attron(A_REVERSE);
				printw("%d",ionice_prio);
				if (!ionice_cl)
					attroff(A_REVERSE);
				attroff(A_BOLD);
			} else
				printw(" (invalid %s)",COLUMN_NAME(0));
		} else
			printw(" (select %s)",COLUMN_NAME(0));
		printw(" ");
		attron(A_REVERSE);
		printw("[use 0-9/bksp for %s, tab and arrows for prio]",COLUMN_NAME(0));
		attroff(A_REVERSE);
		show=TRUE;
	}

	attron(A_REVERSE);
	mvhline(ionicepos+1,0,' ',maxx);
	move(ionicepos+1,0);

	for (i=0;i<IOTOP_SORT_BY_MAX;i++) {
		int wt,wi=column_width[i];
		char t[50];

		if (i==IOTOP_SORT_BY_PID)
			wi=maxpidlen+2;
		if (i==IOTOP_SORT_BY_GRAPH)
			wi=gr_width+1;
		if (i==IOTOP_SORT_BY_COMMAND)
			wi=maxcmdline;

		if (config.opts[&config.f.hidepid-config.opts+i])
			continue;

		wt=strlen(COLUMN_NAME(i));
		if (wt>wi-1)
			wt=wi-1;
		if (config.f.sort_by==i)
			attron(A_BOLD);
		snprintf(t,sizeof t,"%-*.*s%c",wt,wt,COLUMN_NAME(i),SORT_CHAR(i));
		printw("%-*.*s",wi,wi,t);
		if (config.f.sort_by==i)
			attroff(A_BOLD);
	}
	attroff(A_REVERSE);

//	iotop_sort_cb(NULL,(void *)(long)((has_unicode&&unicode)?gr_width*2:gr_width));
//	arr_sort(cs,iotop_sort_cb);

	iotop_sort_stats(
		cs,
		config.f.sort_by,
		config.f.sort_order,
		((has_unicode&&unicode)?gr_width*2:gr_width)
	);

	if (maxy<10)
		nohelp=1;
	line=ionicepos+2;
	lastline=line;
	for (i=0;cs->sor&&i<diff_len;i++) {
		struct xxxid_stats *s=cs->sor[i];
		double read_val=config.f.accumulated?s->read_val_acc:s->read_val;
		double write_val=config.f.accumulated?s->write_val_acc:s->write_val;
		char iohist[HISTORY_POS*5];
		char read_str[4],write_str[4];
		char *pw_name,*cmdline;

		// visible history is non-zero
		if (config.f.only&&!memcmp(s->iohist,iohist_z,(has_unicode&&unicode)?gr_width*2:gr_width))
			continue;

		humanize_val(&read_val,read_str,1);
		humanize_val(&write_val,write_str,1);

		pw_name=u8strpadt(s->pw_name,9);
		cmdline=u8strpadt(config.f.fullcmdline?s->cmdline2:s->cmdline1,maxcmdline);

		if (!config.f.hidegraph) {
			*iohist=0;
			for (j=0;j<gr_width;j++)
				if (has_unicode&&unicode)
					strcat(iohist,br_graph[s->iohist[j*2]][s->iohist[j*2+1]]);
				else
					strcat(iohist,as_graph[s->iohist[j]]);
			strcat(iohist," ");
		}

		mvhline(line,0,' ',maxx);
		move(line,0);
		if (!config.f.hidepid)
			printw("%*i  ",maxpidlen,s->tid);
		if (!config.f.hideprio)
			printw("%4s ",str_ioprio(s->io_prio));
		if (!config.f.hideuser)
			printw("%s ",pw_name?pw_name:"(null)");
		if (!config.f.hideread)
			printw("%7.2f %-3.3s ",read_val,read_str);
		if (!config.f.hidewrite)
			printw("%7.2f %-3.3s ",write_val,write_str);
		if (!config.f.hideswapin)
			printw("%6.2f %% ",s->swapin_val);
		if (!config.f.hideio)
			printw("%6.2f %% ",s->blkio_val);
		printw("%s",!config.f.hidegraph?iohist:"");
		if (!config.f.hidecmd)
			printw("%s",cmdline);

		if (pw_name)
			free(pw_name);
		if (cmdline)
			free(cmdline);

		line++;
		lastline=line;
		if (line>maxy-(nohelp?1:3)) // do not draw out of screen, keep 2 lines for help
			break;
	}
	for (line=lastline;line<=maxy-(nohelp?1:3);line++) // always draw empty lines
		mvhline(line,0,' ',maxx);

	if (!nohelp) {
		attron(A_REVERSE);

		mvhline(maxy-2,0,' ',maxx);
		mvhline(maxy-1,0,' ',maxx);
		mvprintw(maxy-2,0,"%s","keys: ^L: refresh ");
		attron(A_UNDERLINE);
		printw("q");
		attroff(A_UNDERLINE);
		printw(": quit ");
		attron(A_UNDERLINE);
		printw("i");
		attroff(A_UNDERLINE);
		printw(": ionice ");
		attron(A_UNDERLINE);
		printw("o");
		attroff(A_UNDERLINE);
		printw(": %s ",config.f.only?"all":"active");
		attron(A_UNDERLINE);
		printw("p");
		attroff(A_UNDERLINE);
		printw(": %s ",config.f.processes?"threads":"procs");
		attron(A_UNDERLINE);
		printw("a");
		attroff(A_UNDERLINE);
		printw(": %s ",config.f.accumulated?"bandwidth":"accum");
		attron(A_UNDERLINE);
		printw("s");
		attroff(A_UNDERLINE);
		printw(": %s ",!config.f.hidegraph?"no-graph":"graph");
		if (has_unicode) {
			attron(A_UNDERLINE);
			printw("u");
			attroff(A_UNDERLINE);
			printw(": %s ",unicode?"ASCII":"UTF");
		}
		attron(A_UNDERLINE);
		printw("h");
		attroff(A_UNDERLINE);
		printw(": help");

		mvprintw(maxy-1,0,"sort: ");
		attron(A_UNDERLINE);
		printw("r");
		attroff(A_UNDERLINE);
		printw(": %s ",config.f.sort_order==IOTOP_SORT_ASC?"desc":"asc");
		attron(A_UNDERLINE);
		printw("left");
		attroff(A_UNDERLINE);
		printw(": %s ",COLUMN_L(config.f.sort_by));
		attron(A_UNDERLINE);
		printw("right");
		attroff(A_UNDERLINE);
		printw(": %s ",COLUMN_R(config.f.sort_by));
		attron(A_UNDERLINE);
		printw("home");
		attroff(A_UNDERLINE);
		printw(": %s ",COLUMN_L(1));
		attron(A_UNDERLINE);
		printw("end");
		attroff(A_UNDERLINE);
		printw(": %s ",COLUMN_L(0));

		attron(A_UNDERLINE);
		printw("1-9");
		attroff(A_UNDERLINE);
		printw(": toggle column");

		attroff(A_REVERSE);
	}

	if (show)
		move(promptx,prompty);
	curs_set(show);
	refresh();
}

static int curses_key(int ch) {
	switch (ch) {
		case 'q':
		case 'Q':
			if (in_ionice) {
				in_ionice=0;
				break;
			}
			return 1;
		case ' ':
		case 'r':
		case 'R':
			config.f.sort_order=(config.f.sort_order==IOTOP_SORT_ASC)?IOTOP_SORT_DESC:IOTOP_SORT_ASC;
			break;
		case KEY_HOME:
			if (in_ionice)
				ionice_cl=1;
			else
				config.f.sort_by=0;
			break;
		case KEY_END:
			if (in_ionice)
				ionice_cl=0;
			else
				config.f.sort_by=IOTOP_SORT_BY_MAX-1;
			break;
		case KEY_RIGHT:
			if (in_ionice)
				ionice_cl=!ionice_cl;
			else
				if (++config.f.sort_by==IOTOP_SORT_BY_MAX)
					config.f.sort_by=IOTOP_SORT_BY_PID;
			break;
		case KEY_LEFT:
			if (in_ionice)
				ionice_cl=!ionice_cl;
			else
				if (--config.f.sort_by==-1)
					config.f.sort_by=IOTOP_SORT_BY_MAX-1;
			break;
		case KEY_UP:
			if (in_ionice) {
				if (ionice_cl) {
					ionice_class++;
					if (ionice_class>=IOPRIO_CLASS_MAX)
						ionice_class=IOPRIO_CLASS_MIN;
				} else {
					ionice_prio++;
					if (ionice_prio>7)
						ionice_prio=0;
				}
			}
			break;
		case KEY_DOWN:
			if (in_ionice) {
				if (ionice_cl) {
					ionice_class--;
					if (ionice_class<IOPRIO_CLASS_MIN)
						ionice_class=IOPRIO_CLASS_MAX-1;
				} else {
					ionice_prio--;
					if (ionice_prio<0)
						ionice_prio=7;
				}
			}
			break;
		case 'o':
		case 'O':
			config.f.only=!config.f.only;
			break;
		case 'p':
		case 'P':
			config.f.processes=!config.f.processes;
			break;
		case 'a':
		case 'A':
			config.f.accumulated=!config.f.accumulated;
			break;
		case '?':
		case 'h':
		case 'H':
			config.f.nohelp=!config.f.nohelp;
			break;
		case 'c':
		case 'C':
			config.f.fullcmdline=!config.f.fullcmdline;
			break;
		case 'i':
		case 'I':
			in_ionice=1;
			ionice_id[0]=0;
			ionice_cl=1;
			ionice_id_changed=1;
			break;
		case 'u':
		case 'U':
			unicode=!unicode;
			break;
		case 27: // ESC
			in_ionice=0;
			break;
		case '\r': // CR
		case KEY_ENTER:
			if (in_ionice) {
				pid_t pgid=atoi(ionice_id);
				int who=IOPRIO_WHO_PROCESS;

				if (config.f.processes) {
					pgid=getpgid(pgid);
					who=IOPRIO_WHO_PGRP;
				}
				in_ionice=0;
				set_ioprio(who,pgid,ionice_class,ionice_prio);
			}
			break;
		case '\t': // TAB
			if (in_ionice)
				ionice_cl=!ionice_cl;
			break;
		case KEY_BACKSPACE:
			if (in_ionice==1) {
				int idlen=strlen(ionice_id);

				if (idlen) {
					ionice_id[idlen-1]=0;
					ionice_id_changed=1;
				}
			}
			break;
		case '0'...'9':
			if (in_ionice==1) {
				size_t idlen=strlen(ionice_id);

				if (idlen<sizeof ionice_id-1) {
					ionice_id[idlen++]=ch;
					ionice_id[idlen]=0;
					ionice_id_changed=1;
				}
			} else
				if (ch>='1'&&ch<='9')
					config.opts[&config.f.hidepid-config.opts+ch-'1']^=1;
			break;
		case KEY_CTRL_L:
			redrawwin(stdscr);
		case KEY_REFRESH:
		case KEY_RESIZE:
			break;
		default:
			return -1;
	}
	return 0;
}

static void curses_update_callback(iotop *hSession) {

	// TODO: Refactory view_curses
	iotop_view * view = iotop_get_view(hSession);
	view_curses(view->cs,view->ps,&view->act,view->refresh);
}

void view_curses_init(void) {

	iotop_set_presentation_method(iotop_get_active_session(),curses_update_callback);

	if (strcmp(getenv("TERM"),"linux")) {
		if (setlocale(LC_CTYPE,"C.UTF-8"))
			has_unicode=1;
		else
			if (setlocale(LC_CTYPE,""))
				if (!strcmp("UTF-8",nl_langinfo(CODESET)))
					has_unicode=1;
	}
	initscr();
	keypad(stdscr,TRUE);
	nonl();
	cbreak();
	halfdelay(2);
	noecho();
	curs_set(FALSE);
	nodelay(stdscr,TRUE);
}

void view_curses_fini(void) {
	endwin();
}

void view_curses_loop(void) {

	iotop * hSession = iotop_get_active_session();

	uint64_t bef = 0;
	int k=ERR;
	int refresh = 0;

	for (;;) {
		uint64_t now=monotime();

		if (bef+1000*params.delay<now) {
			bef=now;
			refresh = iotop_refresh(hSession,config.f.processes,filter1);
		}

		if (refresh&&k==ERR)
			k=KEY_REFRESH;

		if (k!=ERR) {
			int kres;

			if ((kres=curses_key(k))>0)
				break;

			if (kres==0) {
				iotop_present(hSession);
			}
		}
		if ((params.iter>-1)&&((--params.iter)==0))
			break;
		k=getch();
	}
}

