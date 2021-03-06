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
	extern GtkWidget *chil, *dsl, *plot1, *plot2, *plot3, *statusbar, *visl, *window;
	extern GtkWidget *bsr, *bsp, *isr, *isp, *jind, *jind2, *kind, *tc, *tw, *zw; 
	extern GArray *bsra, *bspa, *isra, *ispa, *tca, *twa, *zwa;
	extern GArray *chp, *doms, *vis;
	extern GSList *group2;
	extern guint flagd, flags, kdimxf;
	extern gulong j1_id, j2_id, k_id, bsr_id, bsp_id, isr_id, isp_id, tc_id, tw_id, zw_id;
#	ifdef USE_GDOC
	void help(GtkWidget*, gpointer);
#	endif
	void about(GtkWidget*, gpointer);
	void upg(GtkWidget*, gpointer);
	void pltmv(GtkPlotLinear*, gpointer);
	void pltmvp(GtkPlotPolar*, gpointer);
	void upj(GtkWidget*, gpointer);
	void upk(GtkWidget*, gpointer);
	void upa1(GtkWidget*, gpointer);
	void upa2(GtkWidget*, gpointer);
	void reset(GtkWidget*, gpointer);
	void reset2(GtkWidget*, gpointer);
#endif
