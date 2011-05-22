/***************************************************************************
 *            util.h
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

#ifndef __UTIL_H__
#	define __UTIL_H__
#	include "main.h"
	extern GtkWidget *window, *statusbar, *plot1, *plot2, *plot3, *visl, *dsl, *chil;
	extern GtkWidget *bsr, *bsp, *isr, *isp, *tc, *tw, *zw, *jind, *jind2, *kind; 
	extern GArray *bsra, *bspa, *isra, *ispa, *tca, *twa, *zwa, *vis, *doms, *chp, *byr;
	extern GArray *x, *yb, *xsb, *ysb, *nx2, *bxr, *byr, *bsz, *bnx, *delf, *stars, *specs;
	extern GSList *group2;
	extern guint jdim, kdim, jdimx, kdimx, jdimxf, kdimxf, satl, trc, flags, flagd;
	extern gint lc, mx;
	void help(GtkWidget*, gpointer);
	void about(GtkWidget*, gpointer);
	void upg(GtkWidget*, gpointer);
	void pltmv(PlotLinear*, gpointer);
	void pltmvp(PlotPolar*, gpointer);
	void upj(GtkWidget*, gpointer);
	void upk(GtkWidget*, gpointer);
	void upa1(GtkWidget*, gpointer);
	void upa2(GtkWidget*, gpointer);
	void reset(GtkWidget*, gpointer);
	void reset2(GtkWidget*, gpointer);
#endif
