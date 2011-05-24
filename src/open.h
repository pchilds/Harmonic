/***************************************************************************
 *            open.h
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

#ifndef __OPEN_H__
#	define __OPEN_H__
#	include "main.h"
#	include "util.h"
#	include <fftw3.h>
	extern GtkWidget *window, *statusbar, *notebook, *notebook2, *plot1, *plot3, *zpd, *fst, *tracmenu, *rest, *visl, *dsl, *tr, *pr;
	extern GtkWidget *bat, *chi, *trans, *anosa, *sws, *wll, *oft, *twopionx, *lcmp, *dBs, *neg, *plot1, *plot3;
	extern GArray *bxr, *byr, *bsz, *bnx, *msr, *vis, *doms, *chp, *x, *yb, *specs, *sz, *nx;
	extern GArray *bsra, *bspa, *isra, *ispa, *tca, *twa, *zwa;
	extern GSList *group2;
	extern gint lc, mx;
	extern guint jdim, kdim, jdimx, kdimx, jdimxf, kdimxf, flags, flagd, satl, trc;
	extern gulong pr_id;
	extern gdouble oe;
	extern gchar *fold, *folr;
	extern void trs(GtkWidget*, gpointer);
	extern void pltmv(PlotLinear*, gpointer);
	extern void pltmvp(PlotPolar*, gpointer);
	extern void upg(GtkWidget*, gpointer);
	void opd(GtkWidget*, gpointer);
	void upt(GtkWidget*, gpointer);
#endif
