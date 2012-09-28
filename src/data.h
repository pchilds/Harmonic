/***************************************************************************
 *            data.h
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

#ifndef __DATA_H__
#	define __DATA_H__
#	include "main.h"
#	include "util.h"
	extern GArray *vis, *doms, *chp;
	extern gchar *fold, *folr;
	extern GSList *group, *group2, *group3, *group4, *group5;
	extern GtkWidget *anosa, *dsl, *mg, *notebook, *notebook2, *plot1, *plot2, *plot3, *rest, *ri, *statusbar, *sws, *tracmenu, *visl, *window;
	extern GtkWidget *chi, *dBs, *fst, *neg, *oft, *opttri, *trans, *twopionx, *wll, *zpd;
	extern guint flags;
	extern void upg(GtkWidget*, gpointer);
	void prf(GtkPrintOperation*, GtkPrintContext*, int);
	void prt(GtkWidget*, gpointer);
	void prg(GtkWidget*, gpointer);
	void sav(GtkWidget*, gpointer);
	void sessres(GtkWidget*, gpointer);
	void sesssav(GtkWidget*, gpointer);
	void opd(GtkWidget*, gpointer);
	/*uses statusbar, plot1, notebook, notebook2, rest, visl, dsl, tracmenu, anosa, sws, mg, ri,
	 x, yb, sz, nx, specs, group2, flags, satl, lc*/
#endif
