/***************************************************************************
 *            main.h
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

#ifndef __MAIN_H__
#	define __MAIN_H__
#	include <gtk/gtk.h>
#	include <glib/gi18n.h>
#	include <math.h>
#	include <gtkplot-2.0/gtkplotlinear.h>
#	include <gtkplot-2.0/gtkplotpolar.h>
#	include "config.h"
	G_BEGIN_DECLS
#	define DZE 0.00001 /* divide by zero threshold */
#	define NZE -0.00001 /* negative of this */
#	define MY_2PI 6.2831853071795864769252867665590057683943387987502
#	define NMY_2PI -6.2831853071795864769252867665590057683943387987502
#	define NMY_PI -3.1415926535897932384626433832795028841971693993751
#	define LNTOT 0.23025850929940456840179914546843642076011014886288 /* (ln10)/10 */
	typedef enum
	{
		PROC_OPN=1<<0,
		PROC_TRS=1<<1,
		PROC_PRS=1<<2,
		PROC_BAT=1<<3,
		PROC_CHP=1<<4,
		PROC_OFT=1<<5,
		PROC_POL=1<<6,
		PROC_COM=1<<7,
		PROC_RI=1<<8
	} ProcState;
	typedef enum
	{
		DISP_MIJ=1<<0,
		DISP_MRJ=1<<1,
		DISP_MRK=1<<2,
		DISP_DMS=1<<3,
		DISP_CHP=1<<4,
		DISP_PHI=1<<5
	} DispState;
	G_END_DECLS
#endif
