/***************************************************************************
 *            disp.h
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

#ifndef __DISP_H__
#	define __DISP_H__
#	include "main.h"
	extern GtkWidget *window, *plot1, *plot2, *plot3, *notebook2, *bat, *chi;
	extern GArray *bxr, *byr, *bsz, *bnx, *msr, *vis, *doms, *chp, *delf, *xsb, *ysb, *stars;
	extern GArray *sz, *nx, *sz2, *nx2, *rd1, *gr1, *bl1, *al1, *rd2, *gr2, *bl2, *al2, *rd3, *gr3, *bl3, *al3;
	extern guint jdim, kdim, jdimx, kdimx, jdimxf, kdimxf, flags, flagd;
	extern gint mx;
	extern void upj(GtkWidget*, gpointer);
	extern void upk(GtkWidget*, gpointer);
	void dpa(GtkWidget*, gpointer);
	void dpo(GtkWidget*, gpointer);
	void upc2(GtkWidget*, gpointer);
	void upc3(GtkWidget*, gpointer);
	void upc4(GtkWidget*, gpointer);
	void dpr(GtkWidget*, gpointer);
#endif
