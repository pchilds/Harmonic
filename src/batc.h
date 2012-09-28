/***************************************************************************
 *            batc.h
 *
 *  Sat Dec  4 17:18:14 2010
 *  Copyright  2010  Paul Childs
 *  <pchilds@physics.org>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#ifndef __BATC_H__
#	define __BATC_H__
#	include "main.h"
#	include "util.h"
#	include <fftw3.h>
	extern GtkWidget *fst, *notebook, *notebook2, *plot1, *plot3, *pr, *rest, *statusbar, *tr, *window, *zpd;
	extern GtkWidget *anosa, *chi, *dBs, *lcmp, *mg, *mgp, *neg, *oft, *plot3, *sws, *trans, *twopionx, *wll;
	extern GArray *chp, *doms, *vis;
	extern GArray *bsra, *bspa, *isra, *ispa, *tca, *twa, *zwa;
	extern guint flagd, flags;
	extern gdouble oe;
	extern gchar *fold, *folr;
	extern void trs(GtkWidget*, gpointer);
	extern void pltmv(GtkPlotLinear*, gpointer);
	extern void pltmvp(GtkPlotPolar*, gpointer);
	void bat(GtkWidget*, gpointer);
	void upt(GtkWidget*, gpointer);
	void tsw(GtkWidget*, gpointer);
	void tdl(GtkWidget*, gpointer);
#endif
