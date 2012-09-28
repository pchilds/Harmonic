/***************************************************************************
 *            disp.c
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

#include "disp.h"

GtkWidget *helpwin, *notebook3, *entry1, *entry2, *butt1, *butt2, *ck5, *colour1;
GtkWidget *entry3, *entry4, *butt3, *butt4, *ck1, *jind3, *ck6, *colour2;
GtkWidget *entry5, *entry6, *butt5, *butt6, *ck2, *ck3, *jind4, *ck4, *kind2, *ck7, *colour3;
GdkColormap *cmp;
gint jdim2, jdim3, kdim2;

void dpa(GtkWidget *widget, gpointer data)
{
	GArray *caa, *cab, *cag, *car, *nx, *st, *sz;
	gchar *str, *str2;
	GdkColor clr;
	gdouble iv, mny, mxy, rcn, thc, xi, xf;
	gdouble *ptr;
	gint bt1, bt2, bt3, dx, dx2, j, jdimx, k, l, pry, st4, sz4;
	GtkPlot *pt;
	GtkPlotLinear *plt;
	GtkPlotPolar *plt2;
	guint16 alp;
	PangoFontDescription *ds1, *ds2;

	switch (gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook3)))
	{
		case 2:
		pt=GTK_PLOT(plot3);
		ds1=pango_font_description_from_string(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt5)));
		ds2=pango_font_description_from_string(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt6)));
		gtk_plot_set_font(pt, ds1, ds2);
		{pango_font_description_free(ds1); pango_font_description_free(ds2);}
		{sz4=g_array_index(pt->sizes, gint, 0); st4=g_array_index(pt->stride, gint, 0);}
		{car=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), st4); cag=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), st4); cab=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), st4); caa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), st4);}
		dx=(pt->rd->len);
		for(j=0; j<st4; j++)
		{
			dx2=fmod(st4, dx);
			iv=g_array_index((pt->rd), gdouble, dx2);
			g_array_append_val(car, iv);
			iv=g_array_index((pt->gr), gdouble, dx2);
			g_array_append_val(cag, iv);
			iv=g_array_index((pt->bl), gdouble, dx2);
			g_array_append_val(cab, iv);
			iv=g_array_index((pt->al), gdouble, dx2);
			g_array_append_val(caa, iv);
		}
		gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(colour3), &clr);
		pry=0;
		bt1=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2));
		bt2=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3));
		bt3=gtk_combo_box_get_active(GTK_COMBO_BOX(ck4));
		if ((bt3*8)!=(flagd&(DISP_CHP|DISP_DMS)))/*change in result type*/
		{
			if (bt3==0) flagd&=(DISP_MRK|DISP_MRJ|DISP_MIJ);
			else if (bt3==1) {flagd&=(DISP_DMS|DISP_MRK|DISP_MRJ|DISP_MIJ); flagd|=DISP_DMS;}
			else {flagd&=(DISP_CHP|DISP_MRK|DISP_MRJ|DISP_MIJ); flagd|=DISP_CHP;}
			pry=1;
		}
		if ((flags&PROC_POL)!=0)
		{
			plt2=GTK_PLOT_POLAR(plot3);
			{str=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry5))); str2=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry6)));}
			gtk_plot_polar_set_label(plt2, str, str2);
			{g_free(str); g_free(str2);}
			g_object_get(G_OBJECT(plot3), "thmin", &xi, "thmax", &xf, "rmin", &mny, "rmax", &mxy, "rcnt", &rcn, "thcnt", &thc, NULL);
			if (bt1)
			{
				if (bt2)/* multi over both */
				{
					j=(jdim3*kdimxf)+kdim2;
					ptr=&g_array_index(car, gdouble, j);
					iv=((gdouble) (clr.red))/65535;
					*ptr=iv;
					ptr=&g_array_index(cag, gdouble, j);
					iv=((gdouble) (clr.green))/65535;
					*ptr=iv;
					ptr=&g_array_index(cab, gdouble, j);
					iv=((gdouble) (clr.blue))/65535;
					*ptr=iv;
					ptr=&g_array_index(caa, gdouble, j);
					alp=gtk_color_selection_get_current_alpha(GTK_COLOR_SELECTION(colour3));
					iv=((gdouble) alp)/65535;
					*ptr=iv;
					if (((flagd&DISP_MRK)==0)||((flagd&DISP_MRJ)==0)||(pry!=0))
					{
						{nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), st4); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), st4); st=g_array_sized_new(FALSE, FALSE, sizeof(gint), st4);}
						j=bt3*sz4*st4;
						{xi=g_array_index(plt2->thdata, gdouble, 0); mny=g_array_index(plt2->rdata, gdouble, j);}
						{xf=xi; mxy=mny;}
						for (k=1; k<sz4*st4; k++)
						{
							iv=g_array_index(plt2->thdata, gdouble, j+k);
							if (iv>xf) xf=iv;
							else if (iv<xi) xi=iv;
							iv=g_array_index(plt2->rdata, gdouble, j+k);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						for (k=j; k<j+st4; k++)
						{
							g_array_append_val(nx, k);
							g_array_append_val(sz, sz4);
							g_array_append_val(st, st4);
						}
						{pry=1; flagd|=(DISP_MRK|DISP_MRJ);}
						gtk_plot_set_indices(pt, nx, sz, st);
						{g_array_unref(nx); g_array_unref(sz); g_array_unref(st);}
					}
				}
				else /* multi over j */
				{
					ptr=&g_array_index(car, gdouble, jdim3);
					iv=((gdouble) (clr.red))/65535;
					*ptr=iv;
					ptr=&g_array_index(cag, gdouble, jdim3);
					iv=((gdouble) (clr.green))/65535;
					*ptr=iv;
					ptr=&g_array_index(cab, gdouble, jdim3);
					iv=((gdouble) (clr.blue))/65535;
					*ptr=iv;
					ptr=&g_array_index(caa, gdouble, jdim3);
					alp=gtk_color_selection_get_current_alpha(GTK_COLOR_SELECTION(colour3));
					iv=((gdouble) alp)/65535;
					*ptr=iv;
					if (((flagd&DISP_MRK)!=0)||((flagd&DISP_MRJ)==0)||(pry!=0))
					{
						jdimx=st4/kdimxf;
						{nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx); st=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx);}
						j=kdim2+(bt3*sz4*st4);
						{xi=g_array_index(plt2->thdata, gdouble, j); mny=g_array_index(plt2->rdata, gdouble, j);}
						{xf=xi; mxy=mny;}
						for (k=kdimxf; k<sz4*st4; k+=kdimxf)
						{
							iv=g_array_index(plt2->thdata, gdouble, j+k);
							if (iv>xf) xf=iv;
							else if (iv<xi) xi=iv;
							iv=g_array_index(plt2->rdata, gdouble, j+k);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						for (k=j; k<j+st4; k+=kdimxf)
						{
							g_array_append_val(nx, k);
							g_array_append_val(sz, sz4);
							g_array_append_val(st, st4);
						}
						{pry=1; flagd&=(DISP_CHP|DISP_DMS|DISP_MRJ|DISP_MIJ); flagd|=DISP_MRJ;}
						gtk_plot_set_indices(pt, nx, sz, st);
						{g_array_unref(nx); g_array_unref(sz); g_array_unref(st);}
					}
				}
			}
			else if (bt2)/* multi over k */
			{
				ptr=&g_array_index(car, gdouble, kdim2);
				iv=((gdouble) (clr.red))/65535;
				*ptr=iv;
				ptr=&g_array_index(cag, gdouble, kdim2);
				iv=((gdouble) (clr.green))/65535;
				*ptr=iv;
				ptr=&g_array_index(cab, gdouble, kdim2);
				iv=((gdouble) (clr.blue))/65535;
				*ptr=iv;
				ptr=&g_array_index(caa, gdouble, kdim2);
				alp=gtk_color_selection_get_current_alpha(GTK_COLOR_SELECTION(colour3));
				iv=((gdouble) alp)/65535;
				*ptr=iv;
				if (((flagd&DISP_MRK)==0)||((flagd&DISP_MRJ)!=0)||(pry!=0))
				{
					{nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf); st=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);}
					j=(jdim3*kdimxf)+(bt3*sz4*st4);
					{xi=g_array_index(plt2->thdata, gdouble, j); mny=g_array_index(plt2->rdata, gdouble, j);}
					{xf=xi; mxy=mny;}
					k=1;
					while (k<kdimxf)
					{
						iv=g_array_index(plt2->thdata, gdouble, j+k);
						if (iv>xf) xf=iv;
						else if (iv<xi) xi=iv;
						iv=g_array_index(plt2->rdata, gdouble, j+(k++));
						if (iv>mxy) mxy=iv;
						else if (iv<mny) mny=iv;
					}
					l=st4;
					while (l<st4*sz4)
					{
						k=1;
						while (k<kdimxf)
						{
							iv=g_array_index(plt2->thdata, gdouble, j+k+l);
							if (iv>xf) xf=iv;
							else if (iv<xi) xi=iv;
							iv=g_array_index(plt2->rdata, gdouble, j+(k++)+l);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						l+=st4;
					}
					for (k=j; k<j+kdimxf; k++)
					{
						g_array_append_val(nx, k);
						g_array_append_val(sz, sz4);
						g_array_append_val(st, st4);
					}
					{pry=1; flagd&=(DISP_CHP|DISP_DMS|DISP_MRK|DISP_MIJ); flagd|=DISP_MRK;}
					gtk_plot_set_indices(pt, nx, sz, st);
					{g_array_unref(nx); g_array_unref(sz); g_array_unref(st);}
				}
			}
			else/* single */
			{
				ptr=&g_array_index(car, gdouble, 0);
				iv=((gdouble) (clr.red))/65535;
				*ptr=iv;
				ptr=&g_array_index(cag, gdouble, 0);
				iv=((gdouble) (clr.green))/65535;
				*ptr=iv;
				ptr=&g_array_index(cab, gdouble, 0);
				iv=((gdouble) (clr.blue))/65535;
				*ptr=iv;
				ptr=&g_array_index(caa, gdouble, 0);
				*ptr=1;
				if (((flagd&(DISP_MRK|DISP_MRJ))!=0)||(pry!=0))
				{
					{nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1); st=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1);}
					j=kdim2+(jdim3*kdimxf)+(bt3*sz4*st4);
					g_array_append_val(nx, j);
					g_array_append_val(sz, sz4);
					g_array_append_val(st, st4);
					{xi=g_array_index(plt2->thdata, gdouble, j); mny=g_array_index(plt2->rdata, gdouble, j);}
					{xf=xi; mxy=mny;}
					for (k=st4; k<sz4*st4; k+=st4)
					{
						iv=g_array_index(plt2->thdata, gdouble, j+k);
						if (iv>xf) xf=iv;
						else if (iv<xi) xi=iv;
						iv=g_array_index(plt2->rdata, gdouble, j+k);
						if (iv>mxy) mxy=iv;
						else if (iv<mny) mny=iv;
					}
					{pry=1; flagd&=(DISP_CHP|DISP_DMS|DISP_MIJ);}
					gtk_plot_set_indices(pt, nx, sz, st);
					{g_array_unref(nx); g_array_unref(sz); g_array_unref(st);}
				}
			}
			gtk_plot_set_colour(pt, car, cag, cab, caa);
			{g_array_unref(car); g_array_unref(cag); g_array_unref(cab); g_array_unref(caa);}
			if (pry) gtk_plot_polar_update_scale_pretty(plot3, xi, xf, mny, mxy);
			else gtk_plot_polar_update_scale(plot3, xi, xf, mny, mxy, rcn, thc);
		}
		else
		{
			plt=GTK_PLOT_LINEAR(plot3);
			{str=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry5))); str2=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry6)));}
			gtk_plot_linear_set_label(plt, str, str2);
			{g_free(str); g_free(str2);}
			g_object_get(G_OBJECT(plot3), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
			if (bt1)
			{
				if (bt2)/* multi over both */
				{
					j=(jdim3*kdimxf)+kdim2;
					ptr=&g_array_index(car, gdouble, j);
					iv=((gdouble) (clr.red))/65535;
					*ptr=iv;
					ptr=&g_array_index(cag, gdouble, j);
					iv=((gdouble) (clr.green))/65535;
					*ptr=iv;
					ptr=&g_array_index(cab, gdouble, j);
					iv=((gdouble) (clr.blue))/65535;
					*ptr=iv;
					ptr=&g_array_index(caa, gdouble, j);
					alp=gtk_color_selection_get_current_alpha(GTK_COLOR_SELECTION(colour3));
					iv=((gdouble) alp)/65535;
					*ptr=iv;
					if (((flagd&DISP_MRK)==0)||((flagd&DISP_MRJ)==0)||(pry!=0))
					{
						{nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), st4); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), st4); st=g_array_sized_new(FALSE, FALSE, sizeof(gint), st4);}
						j=bt3*sz4*st4;
						{xi=g_array_index(plt->xdata, gdouble, 0); mny=g_array_index(plt->ydata, gdouble, j);}
						{xf=xi; mxy=mny;}
						for (k=1; k<sz4*st4; k++)
						{
							iv=g_array_index(plt->xdata, gdouble, j+k);
							if (iv>xf) xf=iv;
							else if (iv<xi) xi=iv;
							iv=g_array_index(plt->ydata, gdouble, j+k);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						for (k=j; k<j+st4; k++)
						{
							g_array_append_val(nx, k);
							g_array_append_val(sz, sz4);
							g_array_append_val(st, st4);
						}
						{pry=1; flagd|=(DISP_MRK|DISP_MRJ);}
						gtk_plot_set_indices(pt, nx, sz, st);
						{g_array_unref(nx); g_array_unref(sz); g_array_unref(st);}
					}
				}
				else/* multi over j */
				{
					ptr=&g_array_index(car, gdouble, jdim3);
					iv=((gdouble) (clr.red))/65535;
					*ptr=iv;
					ptr=&g_array_index(cag, gdouble, jdim3);
					iv=((gdouble) (clr.green))/65535;
					*ptr=iv;
					ptr=&g_array_index(cab, gdouble, jdim3);
					iv=((gdouble) (clr.blue))/65535;
					*ptr=iv;
					ptr=&g_array_index(caa, gdouble, jdim3);
					alp=gtk_color_selection_get_current_alpha(GTK_COLOR_SELECTION(colour3));
					iv=((gdouble) alp)/65535;
					*ptr=iv;
					if (((flagd&DISP_MRK)!=0)||((flagd&DISP_MRJ)==0)||(pry!=0))
					{
						jdimx=st4/kdimxf;
						{nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx); st=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx);}
						j=kdim2+(bt3*sz4*st4);
						{xi=g_array_index(plt->xdata, gdouble, j); mny=g_array_index(plt->ydata, gdouble, j);}
						{xf=xi; mxy=mny;}
						for (k=kdimxf; k<sz4*st4; k+=kdimxf)
						{
							iv=g_array_index(plt->xdata, gdouble, j+k);
							if (iv>xf) xf=iv;
							else if (iv<xi) xi=iv;
							iv=g_array_index(plt->ydata, gdouble, j+k);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						for (k=j; k<j+st4; k+=kdimxf)
						{
							g_array_append_val(nx, k);
							g_array_append_val(sz, sz4);
							g_array_append_val(st, st4);
						}
						{pry=1; flagd&=(DISP_CHP|DISP_DMS|DISP_MRJ|DISP_MIJ); flagd|=DISP_MRJ;}
						gtk_plot_set_indices(pt, nx, sz, st);
						{g_array_unref(nx); g_array_unref(sz); g_array_unref(st);}
					}
				}
			}
			else if (bt2)/* multi over k */
			{
				ptr=&g_array_index(car, gdouble, kdim2);
				iv=((gdouble) (clr.red))/65535;
				*ptr=iv;
				ptr=&g_array_index(cag, gdouble, kdim2);
				iv=((gdouble) (clr.green))/65535;
				*ptr=iv;
				ptr=&g_array_index(cab, gdouble, kdim2);
				iv=((gdouble) (clr.blue))/65535;
				*ptr=iv;
				ptr=&g_array_index(caa, gdouble, kdim2);
				alp=gtk_color_selection_get_current_alpha(GTK_COLOR_SELECTION(colour3));
				iv=((gdouble) alp)/65535;
				*ptr=iv;
				if (((flagd&DISP_MRK)==0)||((flagd&DISP_MRJ)!=0)||(pry!=0))
				{
					{nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf); st=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);}
					j=(jdim3*kdimxf)+(bt3*sz4*st4);
					{xi=g_array_index(plt->xdata, gdouble, j); mny=g_array_index(plt->ydata, gdouble, j);}
					{xf=xi; mxy=mny;}
					k=1;
					while (k<kdimxf)
					{
						iv=g_array_index(plt->xdata, gdouble, j+k);
						if (iv>xf) xf=iv;
						else if (iv<xi) xi=iv;
						iv=g_array_index(plt->ydata, gdouble, j+(k++));
						if (iv>mxy) mxy=iv;
						else if (iv<mny) mny=iv;
					}
					l=st4;
					while (l<st4*sz4)
					{
						k=1;
						while (k<kdimxf)
						{
							iv=g_array_index(plt->xdata, gdouble, j+k+l);
							if (iv>xf) xf=iv;
							else if (iv<xi) xi=iv;
							iv=g_array_index(plt->ydata, gdouble, j+(k++)+l);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						l+=st4;
					}
					for (k=j; k<j+kdimxf; k++)
					{
						g_array_append_val(nx, k);
						g_array_append_val(sz, sz4);
						g_array_append_val(st, st4);
					}
					{pry=1; flagd&=(DISP_CHP|DISP_DMS|DISP_MRK|DISP_MIJ); flagd|=DISP_MRK;}
					gtk_plot_set_indices(pt, nx, sz, st);
					{g_array_unref(nx); g_array_unref(sz); g_array_unref(st);}
				}
			}
			else/* single */
			{
				ptr=&g_array_index(car, gdouble, 0);
				iv=((gdouble) (clr.red))/65535;
				*ptr=iv;
				ptr=&g_array_index(cag, gdouble, 0);
				iv=((gdouble) (clr.green))/65535;
				*ptr=iv;
				ptr=&g_array_index(cab, gdouble, 0);
				iv=((gdouble) (clr.blue))/65535;
				*ptr=iv;
				ptr=&g_array_index(caa, gdouble, 0);
				*ptr=1;
				if (((flagd&(DISP_MRK|DISP_MRJ))!=0)||(pry!=0))
				{
					{nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1); st=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1);}
					j=kdim2+(jdim3*kdimxf)+(bt3*sz4*st4);
					g_array_append_val(nx, j);
					g_array_append_val(sz, sz4);
					g_array_append_val(st, st4);
					{xi=g_array_index(plt->xdata, gdouble, j); mny=g_array_index(plt->ydata, gdouble, j);}
					{xf=xi; mxy=mny;}
					for (k=st4; k<sz4*st4; k+=st4)
					{
						iv=g_array_index(plt->xdata, gdouble, j+k);
						if (iv>xf) xf=iv;
						else if (iv<xi) xi=iv;
						iv=g_array_index(plt->ydata, gdouble, j+k);
						if (iv>mxy) mxy=iv;
						else if (iv<mny) mny=iv;
					}
					{pry=1; flagd&=(DISP_CHP|DISP_DMS|DISP_MIJ);}
					gtk_plot_set_indices(pt, nx, sz, st);
					{g_array_unref(nx); g_array_unref(sz); g_array_unref(st);}
				}
			}
			gtk_plot_set_colour(pt, car, cag, cab, caa);
			{g_array_unref(car); g_array_unref(cag); g_array_unref(cab); g_array_unref(caa);}
			if (pry==0) gtk_plot_linear_update_scale(plot3, xi, xf, mny, mxy);
			else if (pry>0) gtk_plot_linear_update_scale_pretty(plot3, xi, xf, mny, mxy);
		}
		break;
		case 1:
		plt=GTK_PLOT_LINEAR(plot2);
		pt=GTK_PLOT(plot2);
		{str=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry3))); str2=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry4)));}
		gtk_plot_linear_set_label(plt, str, str2);
		{g_free(str); g_free(str2);}
		ds1=pango_font_description_from_string(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt3)));
		ds2=pango_font_description_from_string(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt4)));
		gtk_plot_set_font(pt, ds1, ds2);
		{pango_font_description_free(ds1); pango_font_description_free(ds2);}
		gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(colour2), &clr);
		g_object_get(G_OBJECT(plot2), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
		sz4=g_array_index(pt->sizes, gint, 0);
		if (sz4>0)
		{
			jdimx=(plt->ydata->len)/(2*sz4);
			k=MAX(jdimx, (bsra->len));
		}
		else {jdimx=1; k=(bsra->len);}
		{car=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k); cag=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k); cab=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k); caa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k);}
		dx=(pt->rd->len);
		for(j=0; j<k; j++)
		{
			dx2=fmod(j, dx);
			iv=g_array_index((pt->rd), gdouble, dx2);
			g_array_append_val(car, iv);
			iv=g_array_index((pt->gr), gdouble, dx2);
			g_array_append_val(cag, iv);
			iv=g_array_index((pt->bl), gdouble, dx2);
			g_array_append_val(cab, iv);
			iv=g_array_index((pt->al), gdouble, dx2);
			g_array_append_val(caa, iv);
		}
		pry=0;
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck1)))
		{
			j=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(jind3));
			ptr=&g_array_index(car, gdouble, j);
			iv=((gdouble) (clr.red))/65535;
			*ptr=iv;
			ptr=&g_array_index(cag, gdouble, j);
			iv=((gdouble) (clr.green))/65535;
			*ptr=iv;
			ptr=&g_array_index(cab, gdouble, j);
			iv=((gdouble) (clr.blue))/65535;
			*ptr=iv;
			ptr=&g_array_index(caa, gdouble, j);
			alp=gtk_color_selection_get_current_alpha(GTK_COLOR_SELECTION(colour2));
			iv=((gdouble) alp)/65535;
			*ptr=iv;
			if ((flagd&DISP_MIJ)==0)/*changed to multiplot*/
			{
				flagd^=DISP_MIJ;
				if ((flags&PROC_TRS)!=0)
				{
					{dx2=2; xi=0; mny=0; xf=g_array_index(plt->xdata, gdouble, (2*sz4)-1); mxy=g_array_index(plt->ydata, gdouble, 0);}
					{nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx);}
					for (j=2; j<plt->ydata->len; j+=2)
					{
						iv=g_array_index(plt->ydata, gdouble, j);
						if (mxy<iv) mxy=iv;
					}
					for (j=0; j<jdimx; j++)
					{
						g_array_append_val(sz, dx2);
						g_array_append_val(sz, sz4);
						k=2*j*sz4;
						g_array_append_val(nx, k);
					}
					gtk_plot_set_indices(pt, nx, sz, st);
					{g_array_unref(nx); g_array_unref(sz); g_array_unref(st);}
					pry=1;
				}
				else pry=-1;
			}
			else if ((flags&PROC_TRS)==0) pry=-1;
		}
		else
		{
			ptr=&g_array_index(car, gdouble, 0);
			iv=((gdouble) (clr.red))/65535;
			*ptr=iv;
			ptr=&g_array_index(cag, gdouble, 0);
			iv=((gdouble) (clr.green))/65535;
			*ptr=iv;
			ptr=&g_array_index(cab, gdouble, 0);
			iv=((gdouble) (clr.blue))/65535;
			*ptr=iv;
			ptr=&g_array_index(caa, gdouble, 0);
			*ptr=1;
			if ((flagd&DISP_MIJ)!=0)/*change to single plot*/
			{
				flagd^=DISP_MIJ;
				if ((flags&PROC_TRS)!=0)
				{
					{nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1); st=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1);}
					g_array_append_val(sz, sz4);
					j=2*sz4*jdim2;
					g_array_append_val(nx, j);
					{xi=0; mny=0; xf=g_array_index(plt->xdata, gdouble, j-1+(2*sz4)); mxy=g_array_index(plt->ydata, gdouble, j);}
					for (k=2; k<2*sz4; k+=2)
					{
						iv=g_array_index(plt->ydata, gdouble, j+k);
						if (mxy<iv) mxy=iv;
					}
					k=2;
					g_array_append_val(st, k);
					gtk_plot_set_indices(pt, nx, sz, st);
					{g_array_unref(nx); g_array_unref(sz); g_array_unref(st);}
					pry=1;
				}
				else pry=-1;
			}
			else if ((flags&PROC_TRS)==0) pry=-1;
		}
		gtk_plot_set_colour(pt, car, cag, cab, caa);
		{g_array_unref(car); g_array_unref(cag); g_array_unref(cab); g_array_unref(caa);}
		if (pry==0) gtk_plot_linear_update_scale(plot2, xi, xf, mny, mxy);
		else if (pry>0) gtk_plot_linear_update_scale_pretty(plot2, xi, xf, mny, mxy);
		break;
		default:
		plt=GTK_PLOT_LINEAR(plot1);
		pt=GTK_PLOT(plot1);
		{str=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry1))); str2=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry2)));}
		gtk_plot_linear_set_label(plt, str, str2);
		{g_free(str); g_free(str2);}
		ds1=pango_font_description_from_string(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt1)));
		ds2=pango_font_description_from_string(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt2)));
		gtk_plot_set_font(pt, ds1, ds2);
		{pango_font_description_free(ds1); pango_font_description_free(ds2);}
		{car=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 1); cag=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 1); cab=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 1); caa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 1);}
		gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(colour1), &clr);
		iv=((gdouble) (clr.red))/65535;
		g_array_append_val(car, iv);
		iv=((gdouble) (clr.green))/65535;
		g_array_append_val(cag, iv);
		iv=((gdouble) (clr.blue))/65535;
		g_array_append_val(cab, iv);
		iv=1;
		g_array_append_val(caa, iv);
		gtk_plot_set_colour(pt, car, cag, cab, caa);
		{g_array_unref(car); g_array_unref(cag); g_array_unref(cab); g_array_unref(caa);}
		g_object_get(G_OBJECT(plot1), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
		gtk_plot_linear_update_scale(plot1, xi, xf, mny, mxy);
		break;
	}
}

void dpo(GtkWidget *widget, gpointer data)
{
	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook3), 0);
	dpa(NULL, NULL);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook3), 1);
	dpa(NULL, NULL);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook3), 2);
	dpa(NULL, NULL);
	gtk_widget_destroy(helpwin);
}

void upc2(GtkWidget *widget, gpointer data)
{
	GdkColor clr2;
	gint dx, val;
	GtkPlot *pt;
	guint alp2=65535;

	pt=GTK_PLOT(plot2);
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck1)))
	{
		val=(pt->rd->len);
		dx=fmod(jdim2, val);
		(clr2.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, dx));
		(clr2.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, dx));
		(clr2.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, dx));
		gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour2), TRUE);
		gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour2), alp2);
		gtk_widget_show(jind3);
	}
	else
	{
		(clr2.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, 0));
		(clr2.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, 0));
		(clr2.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, 0));
		gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour2), FALSE);
		gtk_widget_hide(jind3);
	}
	gdk_colormap_alloc_color(cmp, &clr2, FALSE, TRUE);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colour2), &clr2);
}

void upc3(GtkWidget *widget, gpointer data)
{
	GdkColor clr3;
	gint dx, val;
	GtkPlot *pt;
	guint alp3=65535;

	pt=GTK_PLOT(plot3);
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2)))
	{
		val=(pt->rd->len);
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3)))
		{
			dx=fmod(jdim3, val);
			gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour3), TRUE);
			gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
		}
		else dx=fmod((jdim3*kdimxf)+kdim2, val);
		(clr3.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, dx));
		(clr3.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, dx));
		(clr3.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, dx));
		gtk_widget_show(jind4);
	}
	else
	{
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3)))
		{
			(clr3.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, 0));
			(clr3.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, 0));
			(clr3.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, 0));
			gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour3), FALSE);
			gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
		}
		else
		{
			val=(pt->rd->len);
			dx=fmod(kdim2, val);
			(clr3.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, dx));
			(clr3.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, dx));
			(clr3.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, dx));
		}
		gtk_widget_hide(jind4);
	}
	gdk_colormap_alloc_color(cmp, &clr3, FALSE, TRUE);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colour3), &clr3);
}

void upc4(GtkWidget *widget, gpointer data)
{
	GdkColor clr3;
	gint dx, val;
	GtkPlot *pt;
	guint alp3=65535;

	pt=GTK_PLOT(plot3);
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3)))
	{
		val=(pt->rd->len);
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2)))
		{
			dx=fmod(kdim2, val);
			gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour3), TRUE);
			gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
		}
		else dx=fmod((jdim3*kdimxf)+kdim2, val);
		(clr3.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, dx));
		(clr3.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, dx));
		(clr3.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, dx));
		gtk_widget_show(kind2);
	}
	else
	{
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2)))
		{
			(clr3.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, 0));
			(clr3.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, 0));
			(clr3.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, 0));
			gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour3), FALSE);
			gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
		}
		else
		{
			val=(pt->rd->len);
			dx=fmod(jdim3, val);
			(clr3.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, dx));
			(clr3.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, dx));
			(clr3.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, dx));
		}
		gtk_widget_hide(kind2);
	}
	gdk_colormap_alloc_color(cmp, &clr3, FALSE, TRUE);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colour3), &clr3);
}

void upj3(GtkWidget *widget, gpointer data)
{
	GdkColor clr2;
	gint dx, val;
	GtkPlot *pt;
	guint alp2;

	pt=GTK_PLOT(plot2);
	jdim2=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	val=(pt->rd->len);
	dx=fmod(jdim2, val);
	(clr2.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, dx));
	(clr2.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, dx));
	(clr2.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, dx));
	alp2=(guint16) (65535*g_array_index(pt->al, gdouble, dx));
	gdk_colormap_alloc_color(cmp, &clr2, FALSE, TRUE);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colour2), &clr2);
	gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour2), alp2);
}

void upj4(GtkWidget* widget, gpointer data)
{
	GdkColor clr3;
	gint dx, val;
	GtkPlot *pt;
	guint alp3;

	pt=GTK_PLOT(plot3);
	jdim3=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	val=(pt->rd->len);
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3))) dx=fmod((jdim3*kdimxf)+kdim2, val);
	else dx=fmod(jdim3, val);
	(clr3.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, dx));
	(clr3.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, dx));
	(clr3.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, dx));
	alp3=(guint16) (65535*g_array_index(pt->al, gdouble, dx));
	gdk_colormap_alloc_color(cmp, &clr3, FALSE, TRUE);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colour3), &clr3);
	gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
}

void upk2(GtkWidget *widget, gpointer data)
{
	GdkColor clr3;
	gint dx, val;
	GtkPlot *pt;
	guint alp3;

	pt=GTK_PLOT(plot3);
	kdim2=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	val=(pt->rd->len);
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2))) dx=fmod((jdim3*kdimxf)+kdim2, val);
	else dx=fmod(kdim2, val);
	(clr3.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, dx));
	(clr3.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, dx));
	(clr3.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, dx));
	alp3=(guint16) (65535*g_array_index(pt->al, gdouble, dx));
	gdk_colormap_alloc_color(cmp, &clr3, FALSE, TRUE);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colour3), &clr3);
	gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
}

void dpr(GtkWidget *widget, gpointer data)
{
	AtkObject *atk_label, *atk_widget;
	gchar *str;
	GdkColor clr1, clr2, clr3;
	gdouble iv;
	gint dx, jdimx, val, val2;
	GtkAdjustment *adj;
	GtkPlot *pt;
	GtkPlotLinear *plt;
	GtkPlotPolar *plt2;
	GtkWidget *butt, *hbox, *label, *spr, *table, *vbox;
	guint alp2=65535, alp3=65535;

	{jdim2=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(jind)); kdim2=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(kind)); val2=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook2));}
	jdim3=jdim2;
	helpwin=gtk_dialog_new_with_buttons(_("Display Properties"), GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, NULL);
	g_signal_connect_swapped(G_OBJECT(helpwin), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(helpwin));
	butt=gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	gtk_widget_show(butt);
	g_signal_connect_swapped(G_OBJECT(butt), "clicked", G_CALLBACK(gtk_widget_destroy), G_OBJECT(helpwin));
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(helpwin)->action_area), butt);
	butt=gtk_button_new_from_stock(GTK_STOCK_APPLY);
	gtk_widget_show(butt);
	g_signal_connect(G_OBJECT(butt), "clicked", G_CALLBACK(dpa), NULL);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(helpwin)->action_area), butt);
	butt=gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_widget_show(butt);
	g_signal_connect(G_OBJECT(butt), "clicked", G_CALLBACK(dpo), NULL);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(helpwin)->action_area), butt);
	notebook3=gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook3), GTK_POS_TOP);
	table=gtk_table_new(4, 2, FALSE);
	gtk_widget_show(table);
	plt=GTK_PLOT_LINEAR(plot1);
	pt=GTK_PLOT(plot1);
	label=gtk_label_new(_("Axis label font:"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	str=pango_font_description_to_string(pt->lfont);
	butt1=gtk_font_button_new_with_font(str);
	g_free(str);
	gtk_font_button_set_show_style(GTK_FONT_BUTTON(butt1), TRUE);
	gtk_font_button_set_show_size(GTK_FONT_BUTTON(butt1), TRUE);
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(butt1), TRUE);
	gtk_font_button_set_use_size(GTK_FONT_BUTTON(butt1), FALSE);
	gtk_font_button_set_title(GTK_FONT_BUTTON(butt1), _("Font Selection for Axis Labels"));
	gtk_widget_show(butt1);
	gtk_table_attach(GTK_TABLE(table), butt1, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	atk_widget=gtk_widget_get_accessible(butt1);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("Tick label font:"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	str=pango_font_description_to_string(pt->afont);
	butt2=gtk_font_button_new_with_font(str);
	g_free(str);
	gtk_font_button_set_show_style(GTK_FONT_BUTTON(butt2), TRUE);
	gtk_font_button_set_show_size(GTK_FONT_BUTTON(butt2), TRUE);
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(butt2), TRUE);
	gtk_font_button_set_use_size(GTK_FONT_BUTTON(butt2), FALSE);
	gtk_font_button_set_title(GTK_FONT_BUTTON(butt2), _("Font Selection for Tick Mark Labels"));
	gtk_widget_show(butt2);
	gtk_table_attach(GTK_TABLE(table), butt2, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	atk_widget=gtk_widget_get_accessible(butt2);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("X axis text:"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	entry1=gtk_entry_new();
	str=g_strdup(plt->xlab);
	gtk_entry_set_text(GTK_ENTRY(entry1), str);
	g_free(str);
	gtk_widget_show(entry1);
	gtk_table_attach(GTK_TABLE(table), entry1, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	atk_widget=gtk_widget_get_accessible(entry1);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("Y axis text:"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	entry2=gtk_entry_new();
	str=g_strdup(plt->ylab);
	gtk_entry_set_text(GTK_ENTRY(entry2), str);
	g_free(str);
	gtk_widget_show(entry2);
	gtk_table_attach(GTK_TABLE(table), entry2, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	atk_widget=gtk_widget_get_accessible(entry2);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	spr=gtk_hseparator_new();
	gtk_widget_show(spr);
	ck5=gtk_combo_box_new_text();
	if ((flags&PROC_COM)!=0)
	{
		if ((flags&PROC_RI)==0)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(ck5), _("Magnitude"));
			gtk_combo_box_append_text(GTK_COMBO_BOX(ck5), _("Phase"));
		}
		else
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(ck5), _("Real Part"));
			gtk_combo_box_append_text(GTK_COMBO_BOX(ck5), _("Imaginary Part"));
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(ck5), (flagd&(DISP_PHI))/(DISP_PHI));
		gtk_widget_show(ck5);
	}
	vbox=gtk_vbox_new(FALSE, 2);
	gtk_widget_show(vbox);
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), spr, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), ck5, FALSE, FALSE, 2);
	spr=gtk_vseparator_new();
	gtk_widget_show(spr);
	cmp=gdk_colormap_get_system();
	(clr1.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, 0));
	(clr1.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, 0));
	(clr1.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, 0));
	gdk_colormap_alloc_color(cmp, &clr1, FALSE, TRUE);
	colour1=gtk_color_selection_new();
	gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour1), FALSE);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colour1), &clr1);
	gtk_color_selection_set_has_palette(GTK_COLOR_SELECTION(colour1), TRUE);
	gtk_widget_show(colour1);
	hbox=gtk_hbox_new(FALSE, 2);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), spr, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), colour1, FALSE, FALSE, 2);
	label=gtk_label_new(_("Spectrum"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook3), hbox, label);
	table=gtk_table_new(4, 2, FALSE);
	gtk_widget_show(table);
	plt=GTK_PLOT_LINEAR(plot2);
	pt=GTK_PLOT(plot2);
	label=gtk_label_new(_("Axis label font:"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	str=pango_font_description_to_string(pt->lfont);
	butt3=gtk_font_button_new_with_font(str);
	g_free(str);
	gtk_font_button_set_show_style(GTK_FONT_BUTTON(butt3), TRUE);
	gtk_font_button_set_show_size(GTK_FONT_BUTTON(butt3), TRUE);
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(butt3), TRUE);
	gtk_font_button_set_use_size(GTK_FONT_BUTTON(butt3), FALSE);
	gtk_font_button_set_title(GTK_FONT_BUTTON(butt3), _("Font Selection for Axis Labels"));
	gtk_widget_show(butt3);
	gtk_table_attach(GTK_TABLE(table), butt3, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	atk_widget=gtk_widget_get_accessible(butt3);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("Tick label font:"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	str=pango_font_description_to_string(pt->afont);
	butt4=gtk_font_button_new_with_font(str);
	g_free(str);
	gtk_font_button_set_show_style(GTK_FONT_BUTTON(butt4), TRUE);
	gtk_font_button_set_show_size(GTK_FONT_BUTTON(butt4), TRUE);
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(butt4), TRUE);
	gtk_font_button_set_use_size(GTK_FONT_BUTTON(butt4), FALSE);
	gtk_font_button_set_title(GTK_FONT_BUTTON(butt4), _("Font Selection for Tick Mark Labels"));
	gtk_widget_show(butt4);
	gtk_table_attach(GTK_TABLE(table), butt4, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	atk_widget=gtk_widget_get_accessible(butt4);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("X axis text:"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	entry3=gtk_entry_new();
	str=g_strdup(plt->xlab);
	gtk_entry_set_text(GTK_ENTRY(entry3), str);
	g_free(str);
	gtk_widget_show(entry3);
	gtk_table_attach(GTK_TABLE(table), entry3, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	atk_widget=gtk_widget_get_accessible(entry3);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	label=gtk_label_new(_("Y axis text:"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	entry4=gtk_entry_new();
	str=g_strdup(plt->ylab);
	gtk_entry_set_text(GTK_ENTRY(entry4), str);
	g_free(str);
	gtk_widget_show(entry4);
	gtk_table_attach(GTK_TABLE(table), entry4, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	atk_widget=gtk_widget_get_accessible(entry4);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
	atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
	spr=gtk_hseparator_new();
	gtk_widget_show(spr);
	ck6=gtk_combo_box_new_text();
	if ((flags&PROC_COM)!=0)
	{
		if ((flags&PROC_RI)==0)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(ck6), _("Magnitude"));
			gtk_combo_box_append_text(GTK_COMBO_BOX(ck6), _("Phase"));
		}
		else
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(ck6), _("Real Part"));
			gtk_combo_box_append_text(GTK_COMBO_BOX(ck6), _("Imaginary Part"));
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(ck6), (flagd&(DISP_PHI))/(DISP_PHI));
		gtk_widget_show(ck6);
	}
	ck1=gtk_check_button_new_with_label(_("Multiple plots for Inverse Domain"));
	gtk_widget_show(ck1);
	jdimx=(plt->ydata->len)/(2*g_array_index(pt->sizes, gint, 0));
	adj=(GtkAdjustment*) gtk_adjustment_new(jdim2, 0, jdimx-1, 1.0, 5.0, 0.0);
	jind3=gtk_spin_button_new(adj, 0, 0);
	g_signal_connect(G_OBJECT(jind3), "value-changed", G_CALLBACK(upj3), NULL);
	hbox=gtk_hbox_new(FALSE, 2);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(hbox), ck1, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), jind3, FALSE, FALSE, 2);
	vbox=gtk_vbox_new(FALSE, 2);
	gtk_widget_show(vbox);	
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), spr, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), ck6, FALSE, FALSE, 2);
	spr=gtk_hseparator_new();
	gtk_widget_show(spr);
	gtk_box_pack_start(GTK_BOX(vbox), spr, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
	spr=gtk_vseparator_new();
	gtk_widget_show(spr);
	colour2=gtk_color_selection_new();
	if ((flagd&DISP_MIJ)!=0)
	{
		val=(pt->rd->len);
		dx=fmod(jdim2, val);
		(clr2.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, dx));
		(clr2.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, dx));
		(clr2.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, dx));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck1), TRUE);
		gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour2), TRUE);
		gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour2), alp2);
		gtk_widget_show(jind3);
	}
	else
	{
		(clr2.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, 0));
		(clr2.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, 0));
		(clr2.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, 0));
	}
	g_signal_connect(G_OBJECT(ck1), "toggled", G_CALLBACK(upc2), NULL);
	gdk_colormap_alloc_color(cmp, &clr2, FALSE, TRUE);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colour2), &clr2);
	gtk_color_selection_set_has_palette(GTK_COLOR_SELECTION(colour2), TRUE);
	gtk_widget_show(colour2);
	hbox=gtk_hbox_new(FALSE, 2);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), spr, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), colour2, FALSE, FALSE, 2);
	label=gtk_label_new(_("Inverse Spectrum"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook3), hbox, label);
	if ((flags&PROC_BAT)!=0)
	{
		vbox=gtk_vbox_new(FALSE, 2);
		gtk_widget_show(vbox);
		table=gtk_table_new(4, 2, FALSE);
		gtk_widget_show(table);
		entry5=gtk_entry_new();
		gtk_widget_show(entry5);
		gtk_table_attach(GTK_TABLE(table), entry5, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		entry6=gtk_entry_new();
		gtk_widget_show(entry6);
		gtk_table_attach(GTK_TABLE(table), entry6, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		if ((flags&PROC_POL)!=0)
		{
			plt2=GTK_PLOT_POLAR(plot3);
			label=gtk_label_new(_("Radial axis text:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			str=g_strdup(plt2->rlab);
			gtk_entry_set_text(GTK_ENTRY(entry5), str);
			g_free(str);
			atk_widget=gtk_widget_get_accessible(entry5);
			atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
			atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
			atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
			label=gtk_label_new(_("Azimuthal axis text:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			str=g_strdup(plt2->thlab);
		}
		else
		{
			plt=GTK_PLOT_LINEAR(plot3);
			label=gtk_label_new(_("X axis text:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			str=g_strdup(plt->xlab);
			gtk_entry_set_text(GTK_ENTRY(entry5), str);
			g_free(str);
			atk_widget=gtk_widget_get_accessible(entry5);
			atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
			atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
			atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
			label=gtk_label_new(_("Y axis text:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			str=g_strdup(plt->ylab);
		}
		gtk_entry_set_text(GTK_ENTRY(entry6), str);
		g_free(str);
		pt=GTK_PLOT(plot3);
		str=pango_font_description_to_string(pt->lfont);
		butt5=gtk_font_button_new_with_font(str);
		g_free(str);
		str=pango_font_description_to_string(pt->afont);
		butt6=gtk_font_button_new_with_font(str);
		g_free(str);
		atk_widget=gtk_widget_get_accessible(entry6);
		atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
		atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
		atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
		label=gtk_label_new(_("Axis label font:"));
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_font_button_set_show_style(GTK_FONT_BUTTON(butt5), TRUE);
		gtk_font_button_set_show_size(GTK_FONT_BUTTON(butt5), TRUE);
		gtk_font_button_set_use_font(GTK_FONT_BUTTON(butt5), TRUE);
		gtk_font_button_set_use_size(GTK_FONT_BUTTON(butt5), FALSE);
		gtk_font_button_set_title(GTK_FONT_BUTTON(butt5), _("Font Selection for Axis Labels"));
		gtk_widget_show(butt5);
		gtk_table_attach(GTK_TABLE(table), butt5, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		atk_widget=gtk_widget_get_accessible(butt5);
		atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
		atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
		atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
		label=gtk_label_new(_("Tick label font:"));
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_font_button_set_show_style(GTK_FONT_BUTTON(butt6), TRUE);
		gtk_font_button_set_show_size(GTK_FONT_BUTTON(butt6), TRUE);
		gtk_font_button_set_use_font(GTK_FONT_BUTTON(butt6), TRUE);
		gtk_font_button_set_use_size(GTK_FONT_BUTTON(butt6), FALSE);
		gtk_font_button_set_title(GTK_FONT_BUTTON(butt6), _("Font Selection for Tick Mark Labels"));
		gtk_widget_show(butt6);
		gtk_table_attach(GTK_TABLE(table), butt6, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		atk_widget=gtk_widget_get_accessible(butt6);
		atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
		atk_object_add_relationship(atk_label, ATK_RELATION_LABEL_FOR, atk_widget);
		atk_object_add_relationship(atk_widget, ATK_RELATION_LABELLED_BY, atk_label);
		gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
		spr=gtk_hseparator_new();
		gtk_widget_show(spr);
		gtk_box_pack_start(GTK_BOX(vbox), spr, FALSE, FALSE, 2);
		ck4=gtk_combo_box_new_text();
		gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Visibility"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Domain Shift"));
		if ((flags&PROC_CHP)!=0)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Chirp"));
			gtk_combo_box_set_active(GTK_COMBO_BOX(ck4), (flagd&(DISP_CHP|DISP_DMS))/8);
		}
		else gtk_combo_box_set_active(GTK_COMBO_BOX(ck4), (flagd&(DISP_CHP|DISP_DMS))/8);
		gtk_widget_show(ck4);
		gtk_box_pack_start(GTK_BOX(vbox), ck4, FALSE, FALSE, 2);
		ck7=gtk_combo_box_new_text();
		if ((flags&PROC_COM)!=0)
		{
			if ((flags&PROC_RI)==0)
			{
				gtk_combo_box_append_text(GTK_COMBO_BOX(ck7), _("Magnitude"));
				gtk_combo_box_append_text(GTK_COMBO_BOX(ck7), _("Phase"));
			}
			else
			{
				gtk_combo_box_append_text(GTK_COMBO_BOX(ck7), _("Real Part"));
				gtk_combo_box_append_text(GTK_COMBO_BOX(ck7), _("Imaginary Part"));
			}
			gtk_combo_box_set_active(GTK_COMBO_BOX(ck7), (flagd&(DISP_PHI))/32);
			gtk_widget_show(ck7);
		}
		gtk_box_pack_start(GTK_BOX(vbox), ck7, FALSE, FALSE, 2);
		table=gtk_table_new(2, 2, FALSE);
		gtk_widget_show(table);
		ck2=gtk_check_button_new_with_label(_("Multiple plots for Results over index j"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck2), (flagd&DISP_MRJ));
		gtk_widget_show(ck2);
		gtk_table_attach(GTK_TABLE(table), ck2, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		jdimx=g_array_index(pt->stride, gint, 0)/kdimxf;
		adj=(GtkAdjustment*) gtk_adjustment_new(jdim2, 0, jdimx-1, 1.0, 5.0, 0.0);
		jind4=gtk_spin_button_new(adj, 0, 0);
		g_signal_connect(G_OBJECT(jind4), "value-changed", G_CALLBACK(upj4), NULL);
		gtk_table_attach(GTK_TABLE(table), jind4, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		ck3=gtk_check_button_new_with_label(_("Multiple plots for Results over index k"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck3), (flagd&DISP_MRK));
		gtk_widget_show(ck3);
		gtk_table_attach(GTK_TABLE(table), ck3, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		adj=(GtkAdjustment*) gtk_adjustment_new(kdim2, 0, kdimxf-1, 1.0, 5.0, 0.0);
		kind2=gtk_spin_button_new(adj, 0, 0);
		g_signal_connect(G_OBJECT(kind2), "value-changed", G_CALLBACK(upk2), NULL);
		gtk_table_attach(GTK_TABLE(table), kind2, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
		hbox=gtk_hbox_new(FALSE, 2);
		gtk_widget_show(hbox);
		gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 2);
		spr=gtk_vseparator_new();
		gtk_widget_show(spr);
		gtk_box_pack_start(GTK_BOX(hbox), spr, FALSE, FALSE, 2);
		colour3=gtk_color_selection_new();
		if ((flagd&DISP_MRJ)!=0)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck2), TRUE);
			gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour3), TRUE);
			gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
			gtk_widget_show(jind4);
			val=(pt->rd->len);
			if ((flagd&DISP_MRK)!=0)
			{
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck3), TRUE);
				gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour3), TRUE);
				gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
				gtk_widget_show(kind2);
				dx=fmod((jdim2*kdimxf)+kdim2, val);
				(clr3.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, dx));
				(clr3.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, dx));
				(clr3.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, dx));
			}
			else
			{
				dx=fmod(jdim2, val);
				(clr3.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, dx));
				(clr3.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, dx));
				(clr3.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, dx));
			}
		}
		else if ((flagd&DISP_MRK)!=0)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck3), TRUE);
			gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour3), TRUE);
			gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
			gtk_widget_show(kind2);
			val=(pt->rd->len);
			dx=fmod(kdim2, val);
			(clr3.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, dx));
			(clr3.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, dx));
			(clr3.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, dx));
		}
		else
		{
			(clr3.red)=(guint16) (65535*g_array_index(pt->rd, gdouble, 0));
			(clr3.green)=(guint16) (65535*g_array_index(pt->gr, gdouble, 0));
			(clr3.blue)=(guint16) (65535*g_array_index(pt->bl, gdouble, 0));
		}
		g_signal_connect(G_OBJECT(ck2), "toggled", G_CALLBACK(upc3), NULL);
		g_signal_connect(G_OBJECT(ck3), "toggled", G_CALLBACK(upc4), NULL);
		gdk_colormap_alloc_color(cmp, &clr3, FALSE, TRUE);
		gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colour3), &clr3);
		gtk_color_selection_set_has_palette(GTK_COLOR_SELECTION(colour3), TRUE);
		gtk_widget_show(colour3);
		gtk_box_pack_start(GTK_BOX(hbox), colour3, FALSE, FALSE, 2);
		label=gtk_label_new(_("Analysis Results"));
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook3), hbox, label);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook3), val2);
	}
	else if (val2==2) gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook3), 1);
	else gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook3), val2);
	gtk_widget_show(notebook3);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(helpwin)->vbox), notebook3);
	gtk_widget_show(helpwin);
}
