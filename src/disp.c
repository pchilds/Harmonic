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
	PlotLinear *plt;
	PlotPolar *plt2;
	GdkColor clr;
	GArray *car, *cag, *cab, *caa;
	gdouble xi, xf, mny, mxy, rcn, thc, iv;
	gdouble *ptr;
	gint dx, dx2, j, k, bt1, bt2, bt3, pry;
	guint16 alp;
	gchar *str;

	switch (gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook3)))
	{
		case 2:
		if ((flags&PROC_POL)!=0)
		{
			plt2=PLOT_POLAR(plot3);
			g_free(plt2->rlab);
			g_free(plt2->thlab);
			pango_font_description_free(plt2->lfont);
			pango_font_description_free(plt2->afont);
			(plt2->rlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry5)));
			(plt2->thlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry6)));
			str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt5)));
			(plt2->lfont)=pango_font_description_from_string(str);
			g_free(str);
			str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt6)));
			(plt2->afont)=pango_font_description_from_string(str);
			g_free(str);
			k=MAX(jdimxf*kdimxf, jdimx*kdimx);
			car=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k);
			cag=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k);
			cab=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k);
			caa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k);
			dx=(plt2->rd->len);
			for(j=0; (j<jdimxf*kdimxf)||(j<jdimx*kdimx); j++)
			{
				dx2=fmod(j, dx);
				iv=g_array_index((plt2->rd), gdouble, dx2);
				g_array_append_val(car, iv);
				iv=g_array_index((plt2->gr), gdouble, dx2);
				g_array_append_val(cag, iv);
				iv=g_array_index((plt2->bl), gdouble, dx2);
				g_array_append_val(cab, iv);
				iv=g_array_index((plt2->al), gdouble, dx2);
				g_array_append_val(caa, iv);
			}
			{g_array_free(rd3, TRUE); g_array_free(gr3, TRUE); g_array_free(bl3, TRUE); g_array_free(al3, TRUE);}
			gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(colour3), &clr);
			g_object_get(G_OBJECT(plot3), "thmin", &xi, "thmax", &xf, "rmin", &mny, "rmax", &mxy, "rcnt", &rcn, "thcnt", &thc, NULL);
			bt3=gtk_combo_box_get_active(GTK_COMBO_BOX(ck4));
			pry=0;
			if ((bt3*8)!=(flagd&(DISP_CHP|DISP_DMS)))
			{
				g_array_free(byr, TRUE);
				j=(vis->len);
				byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), j);
				if (bt3==0)
				{
					for (k=0; k<j; k++)
					{
						iv=g_array_index(vis, gdouble, k);
						g_array_append_val(byr, iv);
					}
					flagd&=(DISP_MRK|DISP_MRJ|DISP_MIJ);
				}
				else if (bt3==1)
				{
					for (k=0; k<j; k++)
					{
						iv=g_array_index(doms, gdouble, k);
						g_array_append_val(byr, iv);
					}
					{flagd&=(DISP_DMS|DISP_MRK|DISP_MRJ|DISP_MIJ); flagd|=DISP_DMS;}
				}
				else
				{
					for (k=0; k<j; k++)
					{
						iv=g_array_index(chp, gdouble, k);
						g_array_append_val(byr, iv);
					}
					{flagd&=(DISP_CHP|DISP_MRK|DISP_MRJ|DISP_MIJ); flagd|=DISP_CHP;}
				}
				(plt2->rdata)=byr;
				pry=1;
			}
			bt1=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2));
			bt2=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3));
			if (bt1)
			{
				if (bt2)/* multi over both */
				{
					j=jdim3*kdimxf;
					j+=kdim2;
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
						dx=g_array_index(bsz, gint, 0);
						g_array_free(bsz, TRUE);
						bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf*kdimxf);
						g_array_append_val(bsz, dx);
						g_array_free(bnx, TRUE);
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf*kdimxf);
						dx2=0;
						g_array_append_val(bnx, dx2);
						xi=g_array_index(bxr, gdouble, 0);
						mny=g_array_index(byr, gdouble, 0);
						{xf=xi; mxy=mny;}
						for (k=1; k<dx; k++)
						{
							iv=g_array_index(bxr, gdouble, k);
							if (iv>xf) xf=iv;
							else if (iv<xi) xi=iv;
							iv=g_array_index(byr, gdouble, k);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						j=1;
						while (j<jdimxf*kdimxf)
						{
							for (k=0; k<dx; k++)
							{
								iv=g_array_index(byr, gdouble, k+(j*dx));
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, dx);
							dx2+=dx;
							g_array_append_val(bnx, dx2);
							j++;
						}
						(plt2->sizes)=bsz;
						(plt2->ind)=bnx;
						{pry=1; flagd|=(DISP_MRK|DISP_MRJ);}
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
						dx=g_array_index(bsz, gint, 0);
						g_array_free(bsz, TRUE);
						bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
						g_array_append_val(bsz, dx);
						g_array_free(bnx, TRUE);
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
						dx2=dx*kdim2;
						g_array_append_val(bnx, dx2);
						xi=g_array_index(bxr, gdouble, 0);
						mny=g_array_index(byr, gdouble, dx2);
						{xf=xi; mxy=mny;}
						for (k=1; k<dx; k++)
						{
							iv=g_array_index(bxr, gdouble, k);
							if (iv>xf) xf=iv;
							else if (iv<xi) xi=iv;
							iv=g_array_index(byr, gdouble, k+dx2);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						j=1;
						while (j<jdimxf)
						{
							{dx2+=dx*kdimxf; j++;}
							g_array_append_val(bnx, dx2);
							g_array_append_val(bsz, dx);
							for (k=0; k<dx; k++)
							{
								iv=g_array_index(byr, gdouble, k+dx2);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
						}
						(plt2->sizes)=bsz;
						(plt2->ind)=bnx;
						{pry=1; flagd&=(DISP_CHP|DISP_DMS|DISP_MRJ|DISP_MIJ); flagd|=DISP_MRJ;}
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
					dx=g_array_index(bsz, gint, 0);
					g_array_free(bsz, TRUE);
					bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
					g_array_append_val(bsz, dx);
					g_array_free(bnx, TRUE);
					bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
					dx2=dx*jdim3*kdimxf;
					g_array_append_val(bnx, dx2);
					xi=g_array_index(bxr, gdouble, 0);
					mny=g_array_index(byr, gdouble, dx2);
					{xf=xi; mxy=mny;}
					for (j=1; j<dx; j++)
					{
						iv=g_array_index(bxr, gdouble, j);
						if (iv>xf) xf=iv;
						else if (iv<xi) xi=iv;
						iv=g_array_index(byr, gdouble, j+dx2);
						if (iv>mxy) mxy=iv;
						else if (iv<mny) mny=iv;
					}
					k=1;
					while (k<kdimxf)
					{
						{dx2+=dx; k++;}
						g_array_append_val(bnx, dx2);
						g_array_append_val(bsz, dx);
						for (j=0; j<dx; j++)
						{
							iv=g_array_index(byr, gdouble, j+dx2);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
					}
					(plt2->sizes)=bsz;
					(plt2->ind)=bnx;
					{pry=1; flagd&=(DISP_CHP|DISP_DMS|DISP_MRK|DISP_MIJ); flagd|=DISP_MRK;}
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
					dx=g_array_index(bsz, gint, 0);
					g_array_free(bsz, TRUE);
					bsz=g_array_new(FALSE, FALSE, sizeof(gint));
					g_array_append_val(bsz, dx);
					(plt2->sizes)=bsz;
					j=dx*(kdim2+(jdim3*kdimxf));
					g_array_free(bnx, TRUE);
					bnx=g_array_new(FALSE, FALSE, sizeof(gint));
					g_array_append_val(bnx, j);
					(plt2->ind)=bnx;
					xi=g_array_index(bxr, gdouble, 0);
					mny=g_array_index(byr, gdouble, j);
					{xf=xi; mxy=mny; k=j+1;}
					j+=dx;
					while (k<j)
					{
						iv=g_array_index(byr, gdouble, k);
						if (mxy<iv) mxy=iv;
						else if (mny>iv) mny=iv;
						iv=g_array_index(bxr, gdouble, k);
						if (xf<iv) xf=iv;
						else if (xi>iv) xi=iv;						
						k++;
					}
					{pry=1; flagd&=(DISP_CHP|DISP_DMS|DISP_MIJ);}
				}
			}
			rd3=g_array_new(FALSE, FALSE, sizeof(gdouble));
			gr3=g_array_new(FALSE, FALSE, sizeof(gdouble));
			bl3=g_array_new(FALSE, FALSE, sizeof(gdouble));
			al3=g_array_new(FALSE, FALSE, sizeof(gdouble));
			for (j=0; j<(car->len); j++)
			{
				iv=g_array_index(car, gdouble, j);
				g_array_append_val(rd3, iv);
				iv=g_array_index(cag, gdouble, j);
				g_array_append_val(gr3, iv);
				iv=g_array_index(cab, gdouble, j);
				g_array_append_val(bl3, iv);
				iv=g_array_index(caa, gdouble, j);
				g_array_append_val(al3, iv);
			}
			{g_array_free(car, TRUE); g_array_free(cag, TRUE); g_array_free(cab, TRUE); g_array_free(caa, TRUE);}
			(plt2->rd)=rd3;
			(plt2->gr)=gr3;
			(plt2->bl)=bl3;
			(plt2->al)=al3;
			if (pry) plot_polar_update_scale_pretty(plot3, xi, xf, mny, mxy);
			else plot_polar_update_scale(plot3, xi, xf, mny, mxy, rcn, thc);
		}
		else if ((flags&PROC_BAT)!=0)
		{
			plt=PLOT_LINEAR(plot3);
			g_free(plt->xlab);
			g_free(plt->ylab);
			pango_font_description_free(plt->lfont);
			pango_font_description_free(plt->afont);
			(plt->xlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry5)));
			(plt->ylab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry6)));
			str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt5)));
			(plt->lfont)=pango_font_description_from_string(str);
			g_free(str);
			str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt6)));
			(plt->afont)=pango_font_description_from_string(str);
			g_free(str);
			k=MAX(jdimxf*kdimxf, jdimx*kdimx);
			car=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k);
			cag=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k);
			cab=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k);
			caa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k);
			dx=(plt->rd->len);
			for(j=0; (j<jdimxf*kdimxf)||(j<jdimx*kdimx); j++)
			{
				dx2=fmod(j, dx);
				iv=g_array_index((plt->rd), gdouble, dx2);
				g_array_append_val(car, iv);
				iv=g_array_index((plt->gr), gdouble, dx2);
				g_array_append_val(cag, iv);
				iv=g_array_index((plt->bl), gdouble, dx2);
				g_array_append_val(cab, iv);
				iv=g_array_index((plt->al), gdouble, dx2);
				g_array_append_val(caa, iv);
			}
			{g_array_free(rd3, TRUE); g_array_free(gr3, TRUE); g_array_free(bl3, TRUE); g_array_free(al3, TRUE);}
			gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(colour3), &clr);
			g_object_get(G_OBJECT(plot3), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
			bt3=gtk_combo_box_get_active(GTK_COMBO_BOX(ck4));
			pry=0;
			if ((bt3*8)!=(flagd&(DISP_CHP|DISP_DMS)))
			{
				g_array_free(byr, TRUE);
				j=(vis->len);
				byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), j);
				if (bt3==0)
				{
					for (k=0; k<j; k++)
					{
						iv=g_array_index(vis, gdouble, k);
						g_array_append_val(byr, iv);
					}
					flagd&=(DISP_MRK|DISP_MRJ|DISP_MIJ);
				}
				else if (bt3==1)
				{
					for (k=0; k<j; k++)
					{
						iv=g_array_index(doms, gdouble, k);
						g_array_append_val(byr, iv);
					}
					{flagd&=(DISP_DMS|DISP_MRK|DISP_MRJ|DISP_MIJ); flagd|=DISP_DMS;}
				}
				else
				{
					for (k=0; k<j; k++)
					{
						iv=g_array_index(chp, gdouble, k);
						g_array_append_val(byr, iv);
					}
					{flagd&=(DISP_CHP|DISP_MRK|DISP_MRJ|DISP_MIJ); flagd|=DISP_CHP;}
				}
				(plt->ydata)=byr;
				pry=1;
			}
			bt1=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2));
			bt2=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3));
			if (bt1)
			{
				if (bt2)/* multi over both */
				{
					if (((flagd&DISP_MRK)==0)||((flagd&DISP_MRJ)==0)||(pry!=0))
					{
						dx=g_array_index(bsz, gint, 0);
						g_array_free(bsz, TRUE);
						bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf*kdimxf);
						g_array_append_val(bsz, dx);
						g_array_free(bnx, TRUE);
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf*kdimxf);
						dx2=0;
						g_array_append_val(bnx, dx2);
						xi=g_array_index(bxr, gdouble, 0);
						mny=g_array_index(byr, gdouble, 0);
						{xf=xi; mxy=mny;}
						for (k=1; k<dx; k++)
						{
							iv=g_array_index(bxr, gdouble, k);
							if (iv>xf) xf=iv;
							else if (iv<xi) xi=iv;
							iv=g_array_index(byr, gdouble, k);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						j=1;
						while (j<jdimxf*kdimxf)
						{
							for (k=0; k<dx; k++)
							{
								iv=g_array_index(byr, gdouble, k+(j*dx));
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, dx);
							dx2+=dx;
							g_array_append_val(bnx, dx2);
							j++;
						}
						(plt->sizes)=bsz;
						(plt->ind)=bnx;
						{pry=1; flagd|=(DISP_MRK|DISP_MRJ);}
					}
					j=jdim3*kdimxf;
					j+=kdim2;
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
				}
				else/* multi over j */
				{
					if (((flagd&DISP_MRK)!=0)||((flagd&DISP_MRJ)==0)||(pry!=0))
					{
						dx=g_array_index(bsz, gint, 0);
						g_array_free(bsz, TRUE);
						bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
						g_array_append_val(bsz, dx);
						g_array_free(bnx, TRUE);
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
						dx2=dx*kdim2;
						g_array_append_val(bnx, dx2);
						xi=g_array_index(bxr, gdouble, 0);
						mny=g_array_index(byr, gdouble, dx2);
						{xf=xi; mxy=mny;}
						for (k=1; k<dx; k++)
						{
							iv=g_array_index(bxr, gdouble, k);
							if (iv>xf) xf=iv;
							else if (iv<xi) xi=iv;
							iv=g_array_index(byr, gdouble, k+dx2);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						j=1;
						while (j<jdimxf)
						{
							{dx2+=dx*kdimxf; j++;}
							g_array_append_val(bnx, dx2);
							g_array_append_val(bsz, dx);
							for (k=0; k<dx; k++)
							{
								iv=g_array_index(byr, gdouble, k+dx2);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
						}
						(plt->sizes)=bsz;
						(plt->ind)=bnx;
						{pry=1; flagd&=(DISP_CHP|DISP_DMS|DISP_MRJ|DISP_MIJ); flagd|=DISP_MRJ;}
					}
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
				}
			}
			else if (bt2)/* multi over k */
			{
				if (((flagd&DISP_MRK)==0)||((flagd&DISP_MRJ)!=0)||(pry!=0))
				{
					dx=g_array_index(bsz, gint, 0);
					g_array_free(bsz, TRUE);
					bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
					g_array_append_val(bsz, dx);
					g_array_free(bnx, TRUE);
					bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
					dx2=dx*jdim3*kdimxf;
					g_array_append_val(bnx, dx2);
					xi=g_array_index(bxr, gdouble, 0);
					mny=g_array_index(byr, gdouble, dx2);
					{xf=xi; mxy=mny;}
					for (j=1; j<dx; j++)
					{
						iv=g_array_index(bxr, gdouble, j);
						if (iv>xf) xf=iv;
						else if (iv<xi) xi=iv;
						iv=g_array_index(byr, gdouble, j+dx2);
						if (iv>mxy) mxy=iv;
						else if (iv<mny) mny=iv;
					}
					k=1;
					while (k<kdimxf)
					{
						{dx2+=dx; k++;}
						g_array_append_val(bnx, dx2);
						g_array_append_val(bsz, dx);
						for (j=0; j<dx; j++)
						{
							iv=g_array_index(byr, gdouble, j+dx2);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
					}
					(plt->sizes)=bsz;
					(plt->ind)=bnx;
					{pry=1; flagd&=(DISP_CHP|DISP_DMS|DISP_MRK|DISP_MIJ); flagd|=DISP_MRK;}
				}
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
			}
			else/* single */
			{
				if (((flagd&(DISP_MRK|DISP_MRJ))!=0)||(pry!=0))
				{
					dx=g_array_index(bsz, gint, 0);
					g_array_free(bsz, TRUE);
					bsz=g_array_new(FALSE, FALSE, sizeof(gint));
					g_array_append_val(bsz, dx);
					(plt->sizes)=bsz;
					j=dx*(kdim2+(jdim3*kdimxf));
					g_array_free(bnx, TRUE);
					bnx=g_array_new(FALSE, FALSE, sizeof(gint));
					g_array_append_val(bnx, j);
					(plt->ind)=bnx;
					xi=g_array_index(bxr, gdouble, 0);
					mny=g_array_index(byr, gdouble, j);
					{mxy=mny; xf=xi; k=j+1;}
					j+=dx;
					while (k<j)
					{
						iv=g_array_index(byr, gdouble, k);
						if (mxy<iv) mxy=iv;
						else if (mny>iv) mny=iv;
						iv=g_array_index(bxr, gdouble, k);
						if (xf<iv) xf=iv;
						else if (xi>iv) xi=iv;						
						k++;
					}
					{pry=1; flagd&=(DISP_CHP|DISP_DMS|DISP_MIJ);}
				}
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
			}
			rd3=g_array_new(FALSE, FALSE, sizeof(gdouble));
			gr3=g_array_new(FALSE, FALSE, sizeof(gdouble));
			bl3=g_array_new(FALSE, FALSE, sizeof(gdouble));
			al3=g_array_new(FALSE, FALSE, sizeof(gdouble));
			for (j=0; j<(car->len); j++)
			{
				iv=g_array_index(car, gdouble, j);
				g_array_append_val(rd3, iv);
				iv=g_array_index(cag, gdouble, j);
				g_array_append_val(gr3, iv);
				iv=g_array_index(cab, gdouble, j);
				g_array_append_val(bl3, iv);
				iv=g_array_index(caa, gdouble, j);
				g_array_append_val(al3, iv);
			}
			{g_array_free(car, TRUE); g_array_free(cag, TRUE); g_array_free(cab, TRUE); g_array_free(caa, TRUE);}
			(plt->rd)=rd3;
			(plt->gr)=gr3;
			(plt->bl)=bl3;
			(plt->al)=al3;
			if (pry) plot_linear_update_scale_pretty(plot3, xi, xf, mny, mxy);
			else plot_linear_update_scale(plot3, xi, xf, mny, mxy);
		}
		else
		{
			flagd&=DISP_MIJ;
			j=gtk_combo_box_get_active(GTK_COMBO_BOX(ck4));
			flagd|=j*8;
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2))) flagd|=DISP_MRJ;
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3))) flagd|=DISP_MRK;
		}
		break;
		case 1:
		plt=PLOT_LINEAR(plot2);
		g_free(plt->xlab);
		g_free(plt->ylab);
		pango_font_description_free(plt->lfont);
		pango_font_description_free(plt->afont);
		(plt->xlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry3)));
		(plt->ylab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry4)));
		str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt3)));
		(plt->lfont)=pango_font_description_from_string(str);
		g_free(str);
		str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt4)));
		(plt->afont)=pango_font_description_from_string(str);
		g_free(str);
		gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(colour2), &clr);
		g_object_get(G_OBJECT(plot2), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
		k=MAX(jdimxf, jdimx);
		car=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k);
		cag=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k);
		cab=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k);
		caa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), k);
		dx=(plt->rd->len);
		for(j=0; (j<jdimxf)||(j<jdimx); j++)
		{
			dx2=fmod(j, dx);
			iv=g_array_index((plt->rd), gdouble, dx2);
			g_array_append_val(car, iv);
			iv=g_array_index((plt->gr), gdouble, dx2);
			g_array_append_val(cag, iv);
			iv=g_array_index((plt->bl), gdouble, dx2);
			g_array_append_val(cab, iv);
			iv=g_array_index((plt->al), gdouble, dx2);
			g_array_append_val(caa, iv);
		}
		{g_array_free(rd2, TRUE); g_array_free(gr2, TRUE); g_array_free(bl2, TRUE); g_array_free(al2, TRUE);}
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
			if ((flagd&DISP_MIJ)==0)
			{
				flagd^=DISP_MIJ;
				if ((flags&PROC_TRS)!=0)
				{
					{xi=0; mny=0;}
					dx2=0;
					dx=g_array_index(sz2, gint, 0);
					{g_array_free(nx2, TRUE); g_array_free(sz2, TRUE);}
					nx2=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
					sz2=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
					xf=g_array_index(xsb, gdouble, dx-1);
					mxy=g_array_index(ysb, gdouble, 0);
					for (j=1; j<dx; j++)
					{
						iv=g_array_index(ysb, gdouble, j);
						if (mxy<iv) mxy=iv;
					}
					g_array_append_val(sz2, dx);
					g_array_append_val(nx2, dx2);
					k=1;
					while (k<jdimxf)
					{
						iv=g_array_index(ysb, gdouble, k*dx);
						if (mxy<iv) mxy=iv;
						for (j=1; j<dx; j++)
						{
							iv=g_array_index(ysb, gdouble, j+(k*dx));
							if (mxy<iv) mxy=iv;
						}
						g_array_append_val(sz2, dx);
						dx2+=dx;
						g_array_append_val(nx2, dx2);
						k++;
					}
					(plt->sizes)=sz2;
					(plt->ind)=nx2;
					pry=1;
				}
			}
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
			if ((flagd&DISP_MIJ)!=0)
			{
				flagd^=DISP_MIJ;
				if ((flags&PROC_TRS)!=0)
				{
					dx=g_array_index(sz2, gint, 0);
					g_array_free(sz2, TRUE);
					sz2=g_array_new(FALSE, FALSE, sizeof(gint));
					g_array_append_val(sz2, dx);
					(plt->sizes)=sz2;
					j=dx*jdim;
					g_array_free(nx2, TRUE);
					nx2=g_array_new(FALSE, FALSE, sizeof(gint));
					g_array_append_val(nx2, j);
					(plt->ind)=nx2;
					{xi=0; mny=0;}
					xf=g_array_index(xsb, gdouble, dx-1);
					mxy=g_array_index(ysb, gdouble, j);
					k=j+1;
					while (k<j+dx)
					{
						iv=g_array_index(ysb, gdouble, k);
						if (mxy<iv) mxy=iv;
						k++;
					}
					pry=1;
				}
			}
		}
		rd2=g_array_new(FALSE, FALSE, sizeof(gdouble));
		gr2=g_array_new(FALSE, FALSE, sizeof(gdouble));
		bl2=g_array_new(FALSE, FALSE, sizeof(gdouble));
		al2=g_array_new(FALSE, FALSE, sizeof(gdouble));
		for (j=0; j<(car->len); j++)
		{
			iv=g_array_index(car, gdouble, j);
			g_array_append_val(rd2, iv);
			iv=g_array_index(cag, gdouble, j);
			g_array_append_val(gr2, iv);
			iv=g_array_index(cab, gdouble, j);
			g_array_append_val(bl2, iv);
			iv=g_array_index(caa, gdouble, j);
			g_array_append_val(al2, iv);
		}
		{g_array_free(car, TRUE); g_array_free(cag, TRUE); g_array_free(cab, TRUE); g_array_free(caa, TRUE);}
		(plt->rd)=rd2;
		(plt->gr)=gr2;
		(plt->bl)=bl2;
		(plt->al)=al2;
		if (pry) plot_linear_update_scale_pretty(plot2, xi, xf, mny, mxy);
		else plot_linear_update_scale(plot2, xi, xf, mny, mxy);
		break;
		default:
		plt=PLOT_LINEAR(plot1);
		g_free(plt->xlab);
		g_free(plt->ylab);
		pango_font_description_free(plt->lfont);
		pango_font_description_free(plt->afont);
		{g_array_free(rd1, TRUE); g_array_free(gr1, TRUE); g_array_free(bl1, TRUE);}
		(plt->xlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry1)));
		(plt->ylab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry2)));
		str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt1)));
		(plt->lfont)=pango_font_description_from_string(str);
		g_free(str);
		str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt2)));
		(plt->afont)=pango_font_description_from_string(str);
		g_free(str);
		gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(colour1), &clr);
		rd1=g_array_new(FALSE, FALSE, sizeof(gdouble));
		gr1=g_array_new(FALSE, FALSE, sizeof(gdouble));
		bl1=g_array_new(FALSE, FALSE, sizeof(gdouble));
		iv=((gdouble) (clr.red))/65535;
		g_array_append_val(rd1, iv);
		iv=((gdouble) (clr.green))/65535;
		g_array_append_val(gr1, iv);
		iv=((gdouble) (clr.blue))/65535;
		g_array_append_val(bl1, iv);
		(plt->rd)=rd1;
		(plt->gr)=gr1;
		(plt->bl)=bl1;
		g_object_get(G_OBJECT(plot1), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
		plot_linear_update_scale(plot1, xi, xf, mny, mxy);
		break;
	}
}

void dpo(GtkWidget *widget, gpointer data)
{
	PlotLinear *plt, *plt2, *plt3;
	PlotPolar *plt4;
	gchar *str;

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
	guint alp2=65535;
	gint dx, val;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck1)))
	{
		val=(rd2->len);
		dx=fmod(jdim2, val);
		(clr2.red)=(guint16) (65535*g_array_index(rd2, gdouble, dx));
		(clr2.green)=(guint16) (65535*g_array_index(gr2, gdouble, dx));
		(clr2.blue)=(guint16) (65535*g_array_index(bl2, gdouble, dx));
		gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour2), TRUE);
		gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour2), alp2);
		gtk_widget_show(jind3);
	}
	else
	{
		(clr2.red)=(guint16) (65535*g_array_index(rd2, gdouble, 0));
		(clr2.green)=(guint16) (65535*g_array_index(gr2, gdouble, 0));
		(clr2.blue)=(guint16) (65535*g_array_index(bl2, gdouble, 0));
		gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour2), FALSE);
		gtk_widget_hide(jind3);
	}
	gdk_colormap_alloc_color(cmp, &clr2, FALSE, TRUE);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colour2), &clr2);
}

void upc3(GtkWidget *widget, gpointer data)
{
	GdkColor clr3;
	guint alp3=65535;
	gint dx, val;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2)))
	{
		val=(rd3->len);
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3)))
		{
			dx=fmod(jdim3, val);
			gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour3), TRUE);
			gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
		}
		else dx=fmod((jdim3*kdimxf)+kdim2, val);
		(clr3.red)=(guint16) (65535*g_array_index(rd3, gdouble, dx));
		(clr3.green)=(guint16) (65535*g_array_index(gr3, gdouble, dx));
		(clr3.blue)=(guint16) (65535*g_array_index(bl3, gdouble, dx));
		gtk_widget_show(jind4);
	}
	else
	{
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3)))
		{
			(clr3.red)=(guint16) (65535*g_array_index(rd3, gdouble, 0));
			(clr3.green)=(guint16) (65535*g_array_index(gr3, gdouble, 0));
			(clr3.blue)=(guint16) (65535*g_array_index(bl3, gdouble, 0));
			gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour3), FALSE);
			gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
		}
		else
		{
			val=(rd3->len);
			dx=fmod(kdim2, val);
			(clr3.red)=(guint16) (65535*g_array_index(rd3, gdouble, dx));
			(clr3.green)=(guint16) (65535*g_array_index(gr3, gdouble, dx));
			(clr3.blue)=(guint16) (65535*g_array_index(bl3, gdouble, dx));
		}
		gtk_widget_hide(jind4);
	}
	gdk_colormap_alloc_color(cmp, &clr3, FALSE, TRUE);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colour3), &clr3);
}

void upc4(GtkWidget *widget, gpointer data)
{
	GdkColor clr3;
	guint alp3=65535;
	gint dx, val;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3)))
	{
		val=(rd3->len);
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2)))
		{
			dx=fmod(kdim2, val);
			gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour3), TRUE);
			gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
		}
		else dx=fmod((jdim3*kdimxf)+kdim2, val);
		(clr3.red)=(guint16) (65535*g_array_index(rd3, gdouble, dx));
		(clr3.green)=(guint16) (65535*g_array_index(gr3, gdouble, dx));
		(clr3.blue)=(guint16) (65535*g_array_index(bl3, gdouble, dx));
		gtk_widget_show(kind2);
	}
	else
	{
		if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2)))
		{
			(clr3.red)=(guint16) (65535*g_array_index(rd3, gdouble, 0));
			(clr3.green)=(guint16) (65535*g_array_index(gr3, gdouble, 0));
			(clr3.blue)=(guint16) (65535*g_array_index(bl3, gdouble, 0));
			gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour3), FALSE);
			gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
		}
		else
		{
			val=(rd3->len);
			dx=fmod(jdim3, val);
			(clr3.red)=(guint16) (65535*g_array_index(rd3, gdouble, dx));
			(clr3.green)=(guint16) (65535*g_array_index(gr3, gdouble, dx));
			(clr3.blue)=(guint16) (65535*g_array_index(bl3, gdouble, dx));
		}
		gtk_widget_hide(kind2);
	}
	gdk_colormap_alloc_color(cmp, &clr3, FALSE, TRUE);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colour3), &clr3);
}

void upj3(GtkWidget *widget, gpointer data)
{
	GdkColor clr2;
	guint alp2;
	gint dx, val;

	jdim2=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	val=(rd2->len);
	dx=fmod(jdim2, val);
	(clr2.red)=(guint16) (65535*g_array_index(rd2, gdouble, dx));
	(clr2.green)=(guint16) (65535*g_array_index(gr2, gdouble, dx));
	(clr2.blue)=(guint16) (65535*g_array_index(bl2, gdouble, dx));
	alp2=(guint16) (65535*g_array_index(al2, gdouble, dx));
	gdk_colormap_alloc_color(cmp, &clr2, FALSE, TRUE);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colour2), &clr2);
	gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour2), alp2);
}

void upj4(GtkWidget *widget, gpointer data)
{
	GdkColor clr3;
	guint alp3;
	gint dx, val;

	jdim3=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	val=(rd3->len);
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3))) dx=fmod((jdim3*kdimxf)+kdim2, val);
	else dx=fmod(jdim3, val);
	(clr3.red)=(guint16) (65535*g_array_index(rd3, gdouble, dx));
	(clr3.green)=(guint16) (65535*g_array_index(gr3, gdouble, dx));
	(clr3.blue)=(guint16) (65535*g_array_index(bl3, gdouble, dx));
	alp3=(guint16) (65535*g_array_index(al3, gdouble, dx));
	gdk_colormap_alloc_color(cmp, &clr3, FALSE, TRUE);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colour3), &clr3);
	gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
}

void upk2(GtkWidget *widget, gpointer data)
{
	GdkColor clr3;
	guint alp3;
	gint dx, val;

	kdim2=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	val=(rd3->len);
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2))) dx=fmod((jdim3*kdimxf)+kdim2, val);
	else dx=fmod(kdim2, val);
	(clr3.red)=(guint16) (65535*g_array_index(rd3, gdouble, dx));
	(clr3.green)=(guint16) (65535*g_array_index(gr3, gdouble, dx));
	(clr3.blue)=(guint16) (65535*g_array_index(bl3, gdouble, dx));
	alp3=(guint16) (65535*g_array_index(al3, gdouble, dx));
	gdk_colormap_alloc_color(cmp, &clr3, FALSE, TRUE);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(colour3), &clr3);
	gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
}

void dpr(GtkWidget *widget, gpointer data)
{
	GtkWidget *butt, *hbox, *vbox, *table, *label, *spr;
	GtkAdjustment *adj;
	PlotLinear *plt, *plt2, *plt3;
	PlotPolar *plt4;
	AtkObject *atk_widget, *atk_label;
	AtkRelationSet *relation_set;
	AtkRelation *relation;
	AtkObject *targets[1];
	GdkColor clr1, clr2, clr3;
	guint alp2=65535, alp3=65535;
	gint val, dx;
	gdouble iv;
	gchar *str;

	{jdim2=jdim; jdim3=jdim; kdim2=kdim;}
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
	plt=PLOT_LINEAR(plot1);
	label=gtk_label_new(_("Axis label font:"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	str=pango_font_description_to_string(plt->lfont);
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
	relation_set=atk_object_ref_relation_set(atk_label);
	targets[0]=atk_widget;
	relation=atk_relation_new(targets, 1, ATK_RELATION_LABEL_FOR);
	atk_relation_set_add(relation_set, relation);
	g_object_unref(G_OBJECT(relation));
	label=gtk_label_new(_("Tick label font:"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	str=pango_font_description_to_string(plt->afont);
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
	relation_set=atk_object_ref_relation_set(atk_label);
	targets[0]=atk_widget;
	relation=atk_relation_new(targets, 1, ATK_RELATION_LABEL_FOR);
	atk_relation_set_add(relation_set, relation);
	g_object_unref(G_OBJECT(relation));
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
	relation_set=atk_object_ref_relation_set(atk_label);
	targets[0]=atk_widget;
	relation=atk_relation_new(targets, 1, ATK_RELATION_LABEL_FOR);
	atk_relation_set_add(relation_set, relation);
	g_object_unref(G_OBJECT(relation));
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
	relation_set=atk_object_ref_relation_set(atk_label);
	targets[0]=atk_widget;
	relation=atk_relation_new(targets, 1, ATK_RELATION_LABEL_FOR);
	atk_relation_set_add(relation_set, relation);
	g_object_unref(G_OBJECT(relation));
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
		gtk_combo_box_set_active(GTK_COMBO_BOX(ck5), (flagd&(DISP_PHI))/32);
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
	(clr1.red)=(guint16) (65535*g_array_index(rd1, gdouble, 0));
	(clr1.green)=(guint16) (65535*g_array_index(gr1, gdouble, 0));
	(clr1.blue)=(guint16) (65535*g_array_index(bl1, gdouble, 0));
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
	plt2=PLOT_LINEAR(plot2);
	label=gtk_label_new(_("Axis label font:"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	str=pango_font_description_to_string(plt2->lfont);
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
	relation_set=atk_object_ref_relation_set(atk_label);
	targets[0]=atk_widget;
	relation=atk_relation_new(targets, 1, ATK_RELATION_LABEL_FOR);
	atk_relation_set_add(relation_set, relation);
	g_object_unref(G_OBJECT(relation));
	label=gtk_label_new(_("Tick label font:"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	str=pango_font_description_to_string(plt2->afont);
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
	relation_set=atk_object_ref_relation_set(atk_label);
	targets[0]=atk_widget;
	relation=atk_relation_new(targets, 1, ATK_RELATION_LABEL_FOR);
	atk_relation_set_add(relation_set, relation);
	g_object_unref(G_OBJECT(relation));
	label=gtk_label_new(_("X axis text:"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	entry3=gtk_entry_new();
	str=g_strdup(plt2->xlab);
	gtk_entry_set_text(GTK_ENTRY(entry3), str);
	g_free(str);
	gtk_widget_show(entry3);
	gtk_table_attach(GTK_TABLE(table), entry3, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	atk_widget=gtk_widget_get_accessible(entry3);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	relation_set=atk_object_ref_relation_set(atk_label);
	targets[0]=atk_widget;
	relation=atk_relation_new(targets, 1, ATK_RELATION_LABEL_FOR);
	atk_relation_set_add(relation_set, relation);
	g_object_unref(G_OBJECT(relation));
	label=gtk_label_new(_("Y axis text:"));
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	entry4=gtk_entry_new();
	str=g_strdup(plt2->ylab);
	gtk_entry_set_text(GTK_ENTRY(entry4), str);
	g_free(str);
	gtk_widget_show(entry4);
	gtk_table_attach(GTK_TABLE(table), entry4, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	atk_widget=gtk_widget_get_accessible(entry4);
	atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
	relation_set=atk_object_ref_relation_set(atk_label);
	targets[0]=atk_widget;
	relation=atk_relation_new(targets, 1, ATK_RELATION_LABEL_FOR);
	atk_relation_set_add(relation_set, relation);
	g_object_unref(G_OBJECT(relation));
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
		gtk_combo_box_set_active(GTK_COMBO_BOX(ck6), (flagd&(DISP_PHI))/32);
		gtk_widget_show(ck6);
	}
	ck1=gtk_check_button_new_with_label(_("Multiple plots for Inverse Domain"));
	gtk_widget_show(ck1);
	adj=(GtkAdjustment*) gtk_adjustment_new(jdim, 0, jdimx-1, 1.0, 5.0, 0.0);
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
		val=(rd2->len);
		dx=fmod(jdim, val);
		(clr2.red)=(guint16) (65535*g_array_index(rd2, gdouble, dx));
		(clr2.green)=(guint16) (65535*g_array_index(gr2, gdouble, dx));
		(clr2.blue)=(guint16) (65535*g_array_index(bl2, gdouble, dx));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck1), TRUE);
		gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour2), TRUE);
		gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour2), alp2);
		gtk_widget_show(jind3);
	}
	else
	{
		(clr2.red)=(guint16) (65535*g_array_index(rd2, gdouble, 0));
		(clr2.green)=(guint16) (65535*g_array_index(gr2, gdouble, 0));
		(clr2.blue)=(guint16) (65535*g_array_index(bl2, gdouble, 0));
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
	vbox=gtk_vbox_new(FALSE, 2);
	gtk_widget_show(vbox);
	if ((flags&PROC_BAT)!=0)
	{
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
			plt4=PLOT_POLAR(plot3);
			label=gtk_label_new(_("Radial axis text:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			str=g_strdup(plt4->rlab);
			gtk_entry_set_text(GTK_ENTRY(entry5), str);
			g_free(str);
			atk_widget=gtk_widget_get_accessible(entry5);
			atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
			relation_set=atk_object_ref_relation_set(atk_label);
			targets[0]=atk_widget;
			relation=atk_relation_new(targets, 1, ATK_RELATION_LABEL_FOR);
			atk_relation_set_add(relation_set, relation);
			g_object_unref(G_OBJECT(relation));
			label=gtk_label_new(_("Azimuthal axis text:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			str=g_strdup(plt4->thlab);
			gtk_entry_set_text(GTK_ENTRY(entry6), str);
			g_free(str);
			str=pango_font_description_to_string(plt4->lfont);
			butt5=gtk_font_button_new_with_font(str);
			g_free(str);
			str=pango_font_description_to_string(plt4->afont);
		}
		else
		{
			plt3=PLOT_LINEAR(plot3);
			label=gtk_label_new(_("X axis text:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			str=g_strdup(plt3->xlab);
			gtk_entry_set_text(GTK_ENTRY(entry5), str);
			g_free(str);
			atk_widget=gtk_widget_get_accessible(entry5);
			atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
			relation_set=atk_object_ref_relation_set(atk_label);
			targets[0]=atk_widget;
			relation=atk_relation_new(targets, 1, ATK_RELATION_LABEL_FOR);
			atk_relation_set_add(relation_set, relation);
			g_object_unref(G_OBJECT(relation));
			label=gtk_label_new(_("Y axis text:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			str=g_strdup(plt3->ylab);
			gtk_entry_set_text(GTK_ENTRY(entry6), str);
			g_free(str);
			str=pango_font_description_to_string(plt3->lfont);
			butt5=gtk_font_button_new_with_font(str);
			g_free(str);
			str=pango_font_description_to_string(plt3->afont);
		}
		butt6=gtk_font_button_new_with_font(str);
		g_free(str);
		atk_widget=gtk_widget_get_accessible(entry6);
		atk_label=gtk_widget_get_accessible(GTK_WIDGET(label));
		relation_set=atk_object_ref_relation_set(atk_label);
		targets[0]=atk_widget;
		relation=atk_relation_new(targets, 1, ATK_RELATION_LABEL_FOR);
		atk_relation_set_add(relation_set, relation);
		g_object_unref(G_OBJECT(relation));
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
		relation_set=atk_object_ref_relation_set(atk_label);
		targets[0]=atk_widget;
		relation=atk_relation_new(targets, 1, ATK_RELATION_LABEL_FOR);
		atk_relation_set_add(relation_set, relation);
		g_object_unref(G_OBJECT(relation));
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
		relation_set=atk_object_ref_relation_set(atk_label);
		targets[0]=atk_widget;
		relation=atk_relation_new(targets, 1, ATK_RELATION_LABEL_FOR);
		atk_relation_set_add(relation_set, relation);
		g_object_unref(G_OBJECT(relation));
		gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
		spr=gtk_hseparator_new();
		gtk_widget_show(spr);
		gtk_box_pack_start(GTK_BOX(vbox), spr, FALSE, FALSE, 2);
	}
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
	adj=(GtkAdjustment*) gtk_adjustment_new(jdim, 0, jdimx-1, 1.0, 5.0, 0.0);
	jind4=gtk_spin_button_new(adj, 0, 0);
	g_signal_connect(G_OBJECT(jind4), "value-changed", G_CALLBACK(upj4), NULL);
	gtk_table_attach(GTK_TABLE(table), jind4, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	ck3=gtk_check_button_new_with_label(_("Multiple plots for Results over index k"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck3), (flagd&DISP_MRK));
	gtk_widget_show(ck3);
	gtk_table_attach(GTK_TABLE(table), ck3, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	adj=(GtkAdjustment*) gtk_adjustment_new(kdim, 0, kdimx-1, 1.0, 5.0, 0.0);
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
		if ((flagd&DISP_MRK)!=0)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck3), TRUE);
			gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour3), TRUE);
			gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
			gtk_widget_show(kind2);
			val=(rd3->len);
			dx=fmod((jdim*kdimxf)+kdim, val);
			(clr3.red)=(guint16) (65535*g_array_index(rd3, gdouble, dx));
			(clr3.green)=(guint16) (65535*g_array_index(gr3, gdouble, dx));
			(clr3.blue)=(guint16) (65535*g_array_index(bl3, gdouble, dx));
		}
		else
		{
			val=(rd3->len);
			dx=fmod(jdim, val);
			(clr3.red)=(guint16) (65535*g_array_index(rd3, gdouble, dx));
			(clr3.green)=(guint16) (65535*g_array_index(gr3, gdouble, dx));
			(clr3.blue)=(guint16) (65535*g_array_index(bl3, gdouble, dx));
		}
	}
	else if ((flagd&DISP_MRK)!=0)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck3), TRUE);
		gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour3), TRUE);
		gtk_color_selection_set_current_alpha(GTK_COLOR_SELECTION(colour3), alp3);
		gtk_widget_show(kind2);
		val=(rd3->len);
		dx=fmod(kdim, val);
		(clr3.red)=(guint16) (65535*g_array_index(rd3, gdouble, dx));
		(clr3.green)=(guint16) (65535*g_array_index(gr3, gdouble, dx));
		(clr3.blue)=(guint16) (65535*g_array_index(bl3, gdouble, dx));
	}
	else
	{
		(clr3.red)=(guint16) (65535*g_array_index(rd3, gdouble, 0));
		(clr3.green)=(guint16) (65535*g_array_index(gr3, gdouble, 0));
		(clr3.blue)=(guint16) (65535*g_array_index(bl3, gdouble, 0));
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
	val=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook2));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook3), val);
	gtk_widget_show(notebook3);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(helpwin)->vbox), notebook3);
	gtk_widget_show(helpwin);
}