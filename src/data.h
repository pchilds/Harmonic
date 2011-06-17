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
	extern GtkWidget *window, *statusbar, *plot1, *plot2, *plot3, *notebook2;
	extern GArray *stars, *delf, *vis, *doms, *chp, *msr;
	extern guint jdim, kdim, jdimxf, kdimxf, flags;
	extern gint mx;
	extern gchar *fold, *folr;
	void prf(GtkPrintOperation*, GtkPrintContext*, int);
	void prt(GtkWidget*, gpointer);
	void prg(GtkWidget*, gpointer);
	void sav(GtkWidget*, gpointer);
	void sessres(GtkWidget*, gpointer);
	void sesssav(GtkWidget*, gpointer);
#endif
