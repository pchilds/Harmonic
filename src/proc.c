/***************************************************************************
 *            proc.c
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

#include "proc.h"

void prs(GtkWidget *widget, gpointer data)
{
	GtkWidget *label;
	PlotLinear *plt;
	gint j, k, l, st, sp, sz2;
	gdouble idelf, iv, vzt, vt, ivd, ivdt, tcn, twd, phi, phio, phia, dst, ddp, pn, cn, tp, ct;
	gchar *str;
	gchar s[10];

	if ((flags&2)!=0)
	{
		plt=PLOT_LINEAR(plot2);
		sz2=g_array_index((plt->sizes), gint, 0);/* check placing of this with what is desired for multiplots (within for loop?) */
		g_array_free(vis, TRUE);
		g_array_free(doms, TRUE);
		vis=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimxf*kdimx);
		doms=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimxf*kdimx);
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(opttri)))
		{
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(chi)))
			{
				g_array_free(chp, TRUE);
				chp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimxf*kdimx);
				for (j=0; j<=jdimxf; j++)
				{
					iv=g_array_index(delf, gdouble, j);
					if (iv<DZE) idelf=G_MAXDOUBLE;
					else idelf=1/iv;
					/*
					 fit values to zwa
					 */
					vzt=g_array_index(stars, gdouble, 2*j*sz2);
					iv=g_array_index(zwa, gdouble, j)*idelf/2;
					for (l=1; l<iv; l++)
					{
						ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
						ivd*=ivd;
						ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
						ivdt*=ivdt;
						vzt+=sqrt(ivd+ivdt);
					}
					if (vzt<DZE) vzt=G_MAXDOUBLE;
					else vzt=l/vzt;
					for (k=0; k<=kdimx; k++)
					{
						st=ceil(g_array_index(isra, gdouble, j+(k*jdimxf))*idelf);
						sp=floor(g_array_index(ispa, gdouble, j+(k*jdimxf))*idelf);
						/*
						 fit values to twa and tca
						 */
						tcn=g_array_index(tca, gdouble, j+(k*jdimxf))*idelf;
						twd=g_array_index(twa, gdouble, j+(k*jdimxf))*idelf/2;
						if ((st<(sz2-2))&&(sp<sz2)&&((sp-st)>1))
						{
							vt=g_array_index(stars, gdouble, st+(2*j*sz2));
							ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-st);
							phia=atan2(ivdt, vt);
							vt*=vt;
							ivdt*=ivdt;
							vt=sqrt(vt+ivdt);
							ivd=g_array_index(stars, gdouble, st+1+(2*j*sz2));
							ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-st-1);
							phio=-atan2(ivdt, ivd);
							phia+=phio;
							ivd*=ivd;
							ivdt*=ivdt;
							vt+=sqrt(ivd+ivdt);
							{pn=0; cn=0; dst=0; ddp=0;}
							for (l=st+2; l<=sp; l++)
							{
								ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
								ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
								phi=atan2(ivdt, ivd);
								phio+=phi;
								if (phio>G_PI) phio-=(MY_2PI);
								else if (phio<=NMY_PI) phio+=(MY_2PI);
								if (l>(tcn-twd+0.5))
								{
									if ((l-1)<=(tcn-twd))
									{
										tp=(((gdouble) l)-tcn-0.5)/twd;
										pn+=++tp;
										dst+=tp*phio;
									}
									else if (l<=(tcn+0.5))
									{
										tp=(((gdouble) l)-tcn-0.5)/twd;
										ct=(((gdouble) l)-tcn-1)/twd;
										pn+=++tp;
										dst+=tp*phio;
										cn+=++ct;
										phia+=phio;
										ddp+=ct*phia;
									}
									else if ((l-1)<=tcn)
									{
										tp=(tcn+0.5-((gdouble) l))/twd;
										ct=(((gdouble) l)-tcn-1)/twd;
										pn+=++tp;
										dst+=tp*phio;
										cn+=++ct;
										phia+=phio;
										ddp+=ct*phia;
									}
									else if (l<(tcn+twd+0.5))
									{
										tp=(tcn+0.5-((gdouble) l))/twd;
										ct=(tcn+1-((gdouble) l))/twd;
										pn+=++tp;
										dst+=tp*phio;
										cn+=++ct;
										phia+=phio;
										ddp+=ct*phia;
									}
									else if ((l-1)<(tcn+twd))
									{
										ct=(tcn+1-((gdouble) l))/twd;
										cn+=++ct;
										phia+=phio;
										ddp+=ct*phia;
									}
								}
								phia=-phio;
								phio=-phi;
								ivd*=ivd;
								ivdt*=ivdt;
								vt+=sqrt(ivd+ivdt);
							}
							pn*=NMY_2PI*g_array_index(delf, gdouble, j);
							if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
							else dst/=pn;
							cn*=NMY_2PI*g_array_index(delf, gdouble, j);
							cn*=G_PI*g_array_index(delf, gdouble, j);
							if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
							else ddp=cn/ddp;
							vt*=vzt/(sp-st+1);
						}
						else
						{
							str=g_strdup_printf(_("Insufficient windowing range in channel %d, %d."), j, k);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							{vt=0; dst=0; ddp=0;}
						}
						g_array_append_val(vis, vt);
						g_array_append_val(doms, dst);
						g_array_append_val(chp, ddp);
					}
				}
				label=gtk_label_new(_("Chirp"));
				gtk_table_attach(GTK_TABLE(rest), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
				gtk_widget_show(label);
				if (jdim<=jdimxf)
				{
					vt=g_array_index(vis, gdouble, (jdim+(kdim*jdimxf)));
					g_snprintf(s, 7, "%f", vt);
					gtk_label_set_text(GTK_LABEL(visl), s);
					vt=g_array_index(doms, gdouble, (jdim+(kdim*jdimxf)));
					g_snprintf(s, 9, "%f", vt);
					gtk_label_set_text(GTK_LABEL(dsl), s);
					vt=g_array_index(chp, gdouble, (jdim+(kdim*jdimxf)));
					g_snprintf(s, 8, "%f", vt);
					chil=gtk_label_new(s);					
				}
				else
				{
					str=g_strdup("");
					gtk_label_set_text(GTK_LABEL(visl), str);
					gtk_label_set_text(GTK_LABEL(dsl), str);
					chil=gtk_label_new(str);
					g_free(str);
				}
				gtk_table_attach(GTK_TABLE(rest), chil, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
				gtk_widget_show(chil);
				flags|=20;
			}
			else
			{
				for (j=0; j<=jdimxf; j++)
				{
					iv=g_array_index(delf, gdouble, j);
					if (iv<DZE) idelf=G_MAXDOUBLE;
					else idelf=1/iv;
					/*
					 fit values to zwa
					 */
					vzt=g_array_index(stars, gdouble, 2*j*sz2);
					iv=g_array_index(zwa, gdouble, j)*idelf/2;
					for (l=1; l<iv; l++)
					{
						ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
						ivd*=ivd;
						ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
						ivdt*=ivdt;
						vzt+=sqrt(ivd+ivdt);
					}
					if (vzt<DZE) vzt=G_MAXDOUBLE;
					else vzt=l/vzt;
					for (k=0; k<=kdimx; k++)
					{
						st=ceil(g_array_index(isra, gdouble, j+(k*jdimxf))*idelf);
						sp=floor(g_array_index(ispa, gdouble, j+(k*jdimxf))*idelf);
						/*
						 fit values to twa and tca
						 */
						tcn=g_array_index(tca, gdouble, j+(k*jdimxf))*idelf;
						twd=g_array_index(twa, gdouble, j+(k*jdimxf))*idelf/2;
						if ((st<(sz2-1))&&(sp<sz2)&&((sp-st)>0))
						{
							vt=g_array_index(stars, gdouble, st+(2*j*sz2));
							ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-st);
							phio=-atan2(ivdt, vt);
							vt*=vt;
							ivdt*=ivdt;
							vt=sqrt(vt+ivdt);
							{dst=0; pn=0;}
							for (l=st+1; l<=sp; l++)
							{
								ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
								ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
								phi=atan2(ivdt, ivd);
								phio+=phi;
								if (phio>G_PI) phio-=(MY_2PI);
								else if (phio<=NMY_PI) phio+=(MY_2PI);
								if (l>(tcn-twd+0.5))
								{
									if (l<=(tcn+0.5))
									{
										tp=(((gdouble) l)-tcn-0.5)/twd;
										pn+=++tp;
										dst+=tp*phio;
									}
									else if (l<(tcn+twd+0.5))
									{
										tp=(tcn+0.5-((gdouble) l))/twd;
										pn+=++tp;
										dst+=tp*phio;
									}
								}
								phio=-phi;
								ivd*=ivd;
								ivdt*=ivdt;
								vt+=sqrt(ivd+ivdt);
							}
							pn*=MY_2PI*g_array_index(delf, gdouble, j);
							dst=-dst;
							dst=dst/pn;
							vt*=vzt/(sp-st+1);
						}
						else
						{
							str=g_strdup_printf(_("Insufficient windowing range in channel %d, %d."), j, k);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							{vt=0; dst=0;}
						}
						g_array_append_val(vis, vt);
						g_array_append_val(doms, dst);
					}
				}
				if (jdim<=jdimxf)
				{
					vt=g_array_index(vis, gdouble, (jdim+(kdim*jdimxf)));
					g_snprintf(s, 7, "%f", vt);
					gtk_label_set_text(GTK_LABEL(visl), s);
					vt=g_array_index(doms, gdouble, (jdim+(kdim*jdimxf)));
					g_snprintf(s, 9, "%f", vt);
					gtk_label_set_text(GTK_LABEL(dsl), s);				
				}
				else
				{
					str=g_strdup("");
					gtk_label_set_text(GTK_LABEL(visl), str);
					gtk_label_set_text(GTK_LABEL(dsl), str);
					g_free(str);
				}
				flags|=4;
			}
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(chi)))
		{
			g_array_free(chp, TRUE);
			chp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimxf*kdimx));
			for (j=0; j<=jdimxf; j++)
			{
				vzt=g_array_index(stars, gdouble, 2*j*sz2);
				iv=g_array_index(delf, gdouble, j);
				if (iv<DZE) idelf=G_MAXDOUBLE;
				else idelf=1/iv;
				iv=g_array_index(zwa, gdouble, j)*idelf/2;
				for (l=1; l<iv; l++)
				{
					ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
					ivd*=ivd;
					ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
					ivdt*=ivdt;
					vzt+=sqrt(ivd+ivdt);
				}
				if (vzt<DZE) vzt=G_MAXDOUBLE;
				else vzt=l/vzt;
				for (k=0; k<=kdimx; k++)
				{
					st=ceil(g_array_index(isra, gdouble, j+(k*jdimxf))*idelf);
					sp=floor(g_array_index(ispa, gdouble, j+(k*jdimxf))*idelf);
					tcn=g_array_index(tca, gdouble, j+(k*jdimxf))*idelf;
					twd=g_array_index(twa, gdouble, j+(k*jdimxf))*idelf/2;
					if ((st<(sz2-2))&&(sp<sz2)&&((sp-st)>1))
					{
						vt=g_array_index(stars, gdouble, st+(2*j*sz2));
						ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-st);
						phia=atan2(ivdt, vt);
						vt*=vt;
						ivdt*=ivdt;
						vt=sqrt(vt+ivdt);
						ivd=g_array_index(stars, gdouble, st+1+(2*j*sz2));
						ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-st-1);
						phio=-atan2(ivdt, ivd);
						phia+=phio;
						ivd*=ivd;
						ivdt*=ivdt;
						vt+=sqrt(ivd+ivdt);
						{pn=0; cn=0; dst=0; ddp=0;}
						for (l=st+2; l<=sp; l++)
						{
							ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
							ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
							phi=atan2(ivdt, ivd);
							phio+=phi;
							if (phio>G_PI) phio-=(MY_2PI);
							else if (phio<=NMY_PI) phio+=(MY_2PI);
							if (l>(tcn-twd+0.5))
							{
								if ((l-1)<=(tcn-twd))
								{
									tp=(((gdouble) l)-tcn-0.5)/twd;
									pn+=++tp;
									dst+=tp*phio;
								}
								else if (l<=(tcn+0.5))
								{
									tp=(((gdouble) l)-tcn-0.5)/twd;
									ct=(((gdouble) l)-tcn-1)/twd;
									pn+=++tp;
									dst+=tp*phio;
									cn+=++ct;
									phia+=phio;
									ddp+=ct*phia;
								}
								else if ((l-1)<=tcn)
								{
									tp=(tcn+0.5-((gdouble) l))/twd;
									ct=(((gdouble) l)-tcn-1)/twd;
									pn+=++tp;
									dst+=tp*phio;
									cn+=++ct;
									phia+=phio;
									ddp+=ct*phia;
								}
								else if (l<(tcn+twd+0.5))
								{
									tp=(tcn+0.5-((gdouble) l))/twd;
									ct=(tcn+1-((gdouble) l))/twd;
									pn+=++tp;
									dst+=tp*phio;
									cn+=++ct;
									phia+=phio;
									ddp+=ct*phia;
								}
								else if ((l-1)<(tcn+twd))
								{
									ct=(tcn+1-((gdouble) l))/twd;
									cn+=++ct;
									phia+=phio;
									ddp+=ct*phia;
								}
							}
							phia=-phio;
							phio=-phi;
							ivd*=ivd;
							ivdt*=ivdt;
							vt+=sqrt(ivd+ivdt);
						}
						pn*=NMY_2PI*g_array_index(delf, gdouble, j);
						if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
						else dst/=pn;
						cn*=NMY_2PI*g_array_index(delf, gdouble, j);
						cn*=G_PI*g_array_index(delf, gdouble, j);
						if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
						else ddp=cn/ddp;
						vt*=vzt/(sp-st+1);
					}
					else
					{
						str=g_strdup_printf(_("Insufficient windowing range in channel %d, %d."), j, k);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						{vt=0; dst=0; ddp=0;}
					}
					g_array_append_val(vis, vt);
					g_array_append_val(doms, dst);
					g_array_append_val(chp, ddp);
				}
			}
			label=gtk_label_new(_("Chirp"));
			gtk_table_attach(GTK_TABLE(rest), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_widget_show(label);
			if (jdim<=jdimxf)
			{
				vt=g_array_index(vis, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 7, "%f", vt);
				gtk_label_set_text(GTK_LABEL(visl), s);
				vt=g_array_index(doms, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 9, "%f", vt);
				gtk_label_set_text(GTK_LABEL(dsl), s);
				vt=g_array_index(chp, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 8, "%f", vt);
				chil=gtk_label_new(s);
			}
			else
			{
				str=g_strdup("");
				gtk_label_set_text(GTK_LABEL(visl), str);
				gtk_label_set_text(GTK_LABEL(dsl), str);
				chil=gtk_label_new(str);
				g_free(str);
			}
			flags|=20;
		}
		else
		{
			for (j=0; j<=jdimxf; j++)
			{
				vzt=g_array_index(stars, gdouble, 2*j*sz2);
				iv=g_array_index(delf, gdouble, j);
				if (iv<DZE) idelf=G_MAXDOUBLE;
				else idelf=1/iv;
				iv=g_array_index(zwa, gdouble, j)*idelf/2;
				for (l=1; l<iv; l++)
				{
					ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
					ivd*=ivd;
					ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
					ivdt*=ivdt;
					vzt+=sqrt(ivd+ivdt);
				}
				if (vzt<DZE) vzt=G_MAXDOUBLE;
				else vzt=l/vzt;
				for (k=0; k<=kdimx; k++)
				{
					st=ceil(g_array_index(isra, gdouble, j+(k*jdimxf))*idelf);
					sp=floor(g_array_index(ispa, gdouble, j+(k*jdimxf))*idelf);
					tcn=g_array_index(tca, gdouble, j+(k*jdimxf))*idelf;
					twd=g_array_index(twa, gdouble, j+(k*jdimxf))*idelf/2;
					if ((st<(sz2-1))&&(sp<sz2)&&((sp-st)>0))
					{
						vt=g_array_index(stars, gdouble, st+(2*j*sz2));
						ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-st);
						phio=-atan2(ivdt, vt);
						vt*=vt;
						ivdt*=ivdt;
						vt=sqrt(vt+ivdt);
						{dst=0; pn=0;}
						for (l=st+1; l<=sp; l++)
						{
							ivd=g_array_index(stars, gdouble, l+(2*j*sz2));
							ivdt=g_array_index(stars, gdouble, (2*(j+1)*sz2)-l);
							phi=atan2(ivdt, ivd);
							phio+=phi;
							if (phio>G_PI) phio-=(MY_2PI);
							else if (phio<=NMY_PI) phio+=(MY_2PI);
							if (l>(tcn-twd+0.5))
							{
								if (l<=(tcn+0.5))
								{
									tp=(((gdouble) l)-tcn-0.5)/twd;
									pn+=++tp;
									dst+=tp*phio;
								}
								else if (l<(tcn+twd+0.5))
								{
									tp=(tcn+0.5-((gdouble) l))/twd;
									pn+=++tp;
									dst+=tp*phio;
								}
							}
							phio=-phi;
							ivd*=ivd;
							ivdt*=ivdt;
							vt+=sqrt(ivd+ivdt);
						}
						pn*=NMY_2PI*g_array_index(delf, gdouble, j);
						if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
						else dst/=pn;
						vt*=vzt/(sp-st+1);
					}
					else
					{
						str=g_strdup_printf(_("Insufficient windowing range in channel %d, %d."), j, k);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						{vt=0; dst=0;}
					}
					g_array_append_val(vis, vt);
					g_array_append_val(doms, dst);
				}
			}
			if (jdim<=jdimxf)
			{
				vt=g_array_index(vis, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 7, "%f", vt);
				gtk_label_set_text(GTK_LABEL(visl), s);
				vt=g_array_index(doms, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 9, "%f", vt);
				gtk_label_set_text(GTK_LABEL(dsl), s);
			}
			else
			{
				str=g_strdup("");
				gtk_label_set_text(GTK_LABEL(visl), str);
				gtk_label_set_text(GTK_LABEL(dsl), str);
				g_free(str);
			}
			flags|=4;
		}
		kdimxf=kdimx;
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 2);
	}
	else
	{
		str=g_strdup(_("No transform for analysis exists yet."));
		gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
		g_free(str);
	}
}

void trs(GtkWidget *widget, gpointer data) /* need to incorporate case for inversion to 2pi/x */
{
	GtkAdjustment *adj;
	PlotLinear *plt;
	guint j, k, st, sp;
	gint n, zp, dx, dx2;
	gdouble iv, clc, ofs, xx, yx;
	gchar *str;
	double *y, *star;
	fftw_plan p;
	fftw_r2r_kind type=FFTW_R2HC;

	if ((flags&1)!=0)
	{
		zp=1<<(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(zpd)));
		n=zp*(jdimx+1);
		g_array_free(stars, TRUE);
		g_array_free(xsb, TRUE);
		g_array_free(ysb, TRUE);
		g_array_free(delf, TRUE);
		delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
		y=fftw_malloc(sizeof(double)*n);
		for (j=0; j<n; j++) y[j]=0;
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(twopionx)))/* interpolate */
		{
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(wll)))/* window based offset */
			{
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss- */
							{
								for (j=0; j<=jdimx; j++)
								{
									iv=g_array_index(bsra, gdouble, j);
									k=0;
									while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
									st=k;
									iv=g_array_index(bspa, gdouble, j);
									while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
									sp=k-st;
									if (sp>zp)
									{
										str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										sp=zp;
									}
									/* fill array */
								}
							}
							else /* +TdBss- */
							{
								for (j=0; j<=jdimx; j++)
								{
									iv=g_array_index(bsra, gdouble, j);
									k=0;
									while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
									st=k;
									iv=g_array_index(bspa, gdouble, j);
									while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
									sp=k-st;
									if (sp>zp)
									{
										str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										sp=zp;
									}
									/* fill array */
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss- */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								/* fill array */
							}
						}
						else /* +RdBss- */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								/* fill array */
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss- and +Tlss- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
					else /* -Rlss- +Rlss- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0- */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								/* fill array */
							}
						}
						else /* +TdB0- */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								/* fill array */
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
					else /* +RdB0- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0- and +Tl0- */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						/* fill array */
					}
				}
				else /* -Rl0- +Rl0- */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						/* fill array */
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(oft)))/* history based offset */
			{
				if ((flags&2)==0) ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));/* check if this is right */
				else ofs=ofs;/* change accordingly */
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss- */
							{
								for (j=0; j<=jdimx; j++)
								{
									iv=g_array_index(bsra, gdouble, j);
									k=0;
									while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
									st=k;
									iv=g_array_index(bspa, gdouble, j);
									while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
									sp=k-st;
									if (sp>zp)
									{
										str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										sp=zp;
									}
									/* fill array */
								}
							}
							else /* +TdBss- */
							{
								for (j=0; j<=jdimx; j++)
								{
									iv=g_array_index(bsra, gdouble, j);
									k=0;
									while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
									st=k;
									iv=g_array_index(bspa, gdouble, j);
									while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
									sp=k-st;
									if (sp>zp)
									{
										str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										sp=zp;
									}
									/* fill array */
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss- */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								/* fill array */
							}
						}
						else /* +RdBss- */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								/* fill array */
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss- and +Tlss- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
					else /* -Rlss- +Rlss- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0- */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								/* fill array */
							}
						}
						else /* +TdB0- */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								/* fill array */
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
					else /* +RdB0- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0- and +Tl0- */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						/* fill array */
					}
				}
				else /* -Rl0- +Rl0- */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						/* fill array */
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))/* user defined offset */
			{
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss- */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								/* fill array */
							}
						}
						else /* +TdBss- */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								/* fill array */
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
					else /* +RdBss- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss- +Tlss- */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						/* fill array */
					}
				}
				else /* -Rlss- +Rlss- */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						/* fill array */
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
			{
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
					else /* +TdB0- */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							/* fill array */
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0- */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						/* fill array */
					}
				}
				else /* +RdB0- */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						/* fill array */
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0- +Tl0- */
			{
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					/* fill array */
				}
			}
			else /* -Rl0- +Rl0- */
			{
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					/* fill array */
				}
			}
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(wll)))/* window based offset */
		{
			ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss+ */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
								g_array_append_val(delf, iv);
								for (k=0; k<sp; k++)
								{
									clc=ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl));
									if (clc<0)
									{
										clc=-exp(LNTOT*clc);
										y[k+(j*zp)]=log(++clc);
									}
									else y[k+(j*zp)]=-G_MAXDOUBLE;
								}
							}
						}
						else /* +TdBss+ */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
								g_array_append_val(delf, iv);
								for (k=0; k<sp; k++)
								{
									clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs;
									if (clc<0)
									{
										clc=-exp(LNTOT*clc);
										y[k+(j*zp)]=log(++clc);
									}
									else y[k+(j*zp)]=-G_MAXDOUBLE;
								}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss+ */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
							g_array_append_val(delf, iv);
							for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl)));
						}
					}
					else /* +RdBss+ */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
							g_array_append_val(delf, iv);
							for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs);
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss+ +Tlss+ */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=-g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
							clc++;
							if (clc>0) y[k+(j*zp)]=log(clc);
							else y[k+(j*zp)]=-G_MAXDOUBLE;
						}
					}
				}
				else /* -Rlss+ +Rlss+ */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
							if (clc>0) y[k+(j*zp)]=log(clc);
							else y[k+(j*zp)]=-G_MAXDOUBLE;
						}
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0+ */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
							g_array_append_val(delf, iv);
							for (k=0; k<sp; k++)
							{
								clc=ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl));
								clc=-exp(LNTOT*clc);
								y[k+(j*zp)]=++clc;
							}
						}
					}
					else /* +TdB0+ */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
							g_array_append_val(delf, iv);
							for (k=0; k<sp; k++)
							{
								clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs;
								clc=-exp(LNTOT*clc);
								y[k+(j*zp)]=++clc;
							}
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0+ */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl))));
					}
				}
				else /* +RdB0+ */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs));
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0+ +Tl0+ */
			{
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
					g_array_append_val(delf, iv);
					for (k=0; k<sp; k++)
					{
						clc=-g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
						y[k+(j*zp)]=++clc;
					}
				}
			}
			else /* -Rl0+ +Rl0+ */
			{
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
					g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) y[k+(j*zp)]=g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
				}
			}
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(oft)))/* history based offset */
		{
			if ((flags&2)==0) ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));/* check if this is right */
			else ofs=ofs;/* change accordingly */
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss+ */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
								g_array_append_val(delf, iv);
								for (k=0; k<sp; k++)
								{
									clc=ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl));
									if (clc<0)
									{
										clc=-exp(LNTOT*clc);
										y[k+(j*zp)]=log(++clc);
									}
									else y[k+(j*zp)]=-G_MAXDOUBLE;
								}
							}
						}
						else /* +TdBss+ */
						{
							for (j=0; j<=jdimx; j++)
							{
								iv=g_array_index(bsra, gdouble, j);
								k=0;
								while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
								st=k;
								iv=g_array_index(bspa, gdouble, j);
								while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
								sp=k-st;
								if (sp>zp)
								{
									str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									sp=zp;
								}
								iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
								g_array_append_val(delf, iv);
								for (k=0; k<sp; k++)
								{
									clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs;
									if (clc<0)
									{
										clc=-exp(LNTOT*clc);
										y[k+(j*zp)]=log(++clc);
									}
									else y[k+(j*zp)]=-G_MAXDOUBLE;
								}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss+ */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
							g_array_append_val(delf, iv);
							for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl)));
						}
					}
					else /* +RdBss+ */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
							g_array_append_val(delf, iv);
							for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs);
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss+ +Tlss+ */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=-g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
							clc++;
							if (clc>0) y[k+(j*zp)]=log(clc);
							else y[k+(j*zp)]=-G_MAXDOUBLE;
						}
					}
				}
				else /* -Rlss+ +Rlss+ */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
							if (clc>0) y[k+(j*zp)]=log(clc);
							else y[k+(j*zp)]=-G_MAXDOUBLE;
						}
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0+ */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
							g_array_append_val(delf, iv);
							for (k=0; k<sp; k++)
							{
								clc=ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl));
								clc=-exp(LNTOT*clc);
								y[k+(j*zp)]=++clc;
							}
						}
					}
					else /* +TdB0+ */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
							g_array_append_val(delf, iv);
							for (k=0; k<sp; k++)
							{
								clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs;
								clc=-exp(LNTOT*clc);
								y[k+(j*zp)]=++clc;
							}
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0+ */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl))));
					}
				}
				else /* +RdB0+ */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs));
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0+ +Tl0+ */
			{
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
					g_array_append_val(delf, iv);
					for (k=0; k<sp; k++)
					{
						clc=-g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
						y[k+(j*zp)]=++clc;
					}
				}
			}
			else /* -Rl0+ +Rl0+ */
			{
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
					g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) y[k+(j*zp)]=g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
				}
			}
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))/* user defined offset */
		{
			ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss+ */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
							g_array_append_val(delf, iv);
							for (k=0; k<sp; k++)
							{
								clc=ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl));
								if (clc<0)
								{
									clc=-exp(LNTOT*clc);
									y[k+(j*zp)]=log(++clc);
								}
								else y[k+(j*zp)]=-G_MAXDOUBLE;
							}
						}
					}
					else /* +TdBss+ */
					{
						for (j=0; j<=jdimx; j++)
						{
							iv=g_array_index(bsra, gdouble, j);
							k=0;
							while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
							st=k;
							iv=g_array_index(bspa, gdouble, j);
							while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
							sp=k-st;
							if (sp>zp)
							{
								str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								sp=zp;
							}
							iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
							g_array_append_val(delf, iv);
							for (k=0; k<sp; k++)
							{
								clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs;
								if (clc<0)
								{
									clc=-exp(LNTOT*clc);
									y[k+(j*zp)]=log(++clc);
								}
								else y[k+(j*zp)]=-G_MAXDOUBLE;
							}
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss+ */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl)));
					}
				}
				else /* +RdBss+ */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs);
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss+ +Tlss+ */
			{
				if ((ofs<DZE)&&(ofs>NZE))
				{
					str=g_strdup(_("Offset must be nonzero for linear measurements."));
					gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
					g_free(str);
				}
				else
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=-g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
							clc++;
							if (clc>0) y[k+(j*zp)]=log(clc);
							else y[k+(j*zp)]=-G_MAXDOUBLE;
						}
					}
				}
			}
			else /* -Rlss+ +Rlss+ */
			{
				if ((ofs<DZE)&&(ofs>NZE))
				{
					str=g_strdup(_("Offset must be nonzero for linear measurements."));
					gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
					g_free(str);
				}
				else
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
							if (clc>0) y[k+(j*zp)]=log(clc);
							else y[k+(j*zp)]=-G_MAXDOUBLE;
						}
					}
				}
			}
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
		{
			ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0+ */
				{
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl));
							clc=-exp(LNTOT*clc);
							y[k+(j*zp)]=++clc;
						}
					}
				}
				else /* +TdB0+ */
				{
					j=0;
					for (j=0; j<=jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
						k=0;
						while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
						st=k;
						iv=g_array_index(bspa, gdouble, j);
						while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
						sp=k-st;
						if (sp>zp)
						{
							str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							sp=zp;
						}
						iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
						g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs;
							clc=-exp(LNTOT*clc);
							y[k+(j*zp)]=++clc;
						}
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0+ */
			{
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
					g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl))));
				}
			}
			else /* +RdB0+ */
			{
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
					g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs));
				}
			}
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0+ and +Tl0+ */ 
		{
			ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
			if ((ofs<DZE)&&(ofs>NZE))
			{
				str=g_strdup(_("Offset must be nonzero for linear measurements."));
				gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
				g_free(str);
			}
			else
			{
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
					g_array_append_val(delf, iv);
					for (k=0; k<sp; k++)
					{
						clc=-g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
						y[k+(j*zp)]=++clc;
					}
				}
			}
		}
		else /* -Rl0+ and +Rl0+ */ 
		{
			ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
			if ((ofs<DZE)&&(ofs>NZE))
			{
				str=g_strdup(_("Offset must be nonzero for linear measurements."));
				gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
				g_free(str);
			}
			else
			{
				for (j=0; j<=jdimx; j++)
				{
					iv=g_array_index(bsra, gdouble, j);
					k=0;
					while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
					st=k;
					iv=g_array_index(bspa, gdouble, j);
					while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
					sp=k-st;
					if (sp>zp)
					{
						str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						sp=zp;
					}
					iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
					g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) y[k+(j*zp)]=g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
				}
			}
		}
		star=fftw_malloc(sizeof(double)*n);
		p=fftw_plan_many_r2r(1, &zp, (jdimx+1), y, NULL, 1, zp, star, NULL, 1, zp, &type, FFTW_ESTIMATE);
		fftw_execute(p);
		fftw_destroy_plan(p);
		fftw_free(y);
		stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
		for (j=0; j<n; j++)
		{
			iv=star[j];
			g_array_append_val(stars, iv);
		}
		xsb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), zp/2);
		sz=g_array_new(FALSE, FALSE, sizeof(gint));
		nx=g_array_new(FALSE, FALSE, sizeof(gint));
		dx=zp/2;
		dx2=0;
		if ((flagd&1)==0)
		{
			for (j=0; j<zp/2; j++)
			{
				xx=j*g_array_index(delf, gdouble, 0);
				g_array_append_val(xsb, xx);
			}
			ysb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), zp/2);
			yx=fabs(star[0]);
			g_array_append_val(ysb, yx);
			for (j=1; j<(zp/2); j++)
			{
				iv=star[j];
				iv*=iv;
				clc=star[zp-j];
				clc*=clc;
				iv+=clc;
				iv=sqrt(iv);
				if (yx<iv) yx=iv;
				g_array_append_val(ysb, iv);
			}
			g_array_append_val(sz, dx);
			g_array_append_val(nx, dx2);
		}
		else
		{
			ysb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n/2);
			xx=0;
			g_array_append_val(xsb, xx);
			yx=fabs(star[0]);
			g_array_append_val(ysb, yx);
			for (j=1; j<zp/2; j++)
			{
				xx=j*g_array_index(delf, gdouble, 0);
				g_array_append_val(xsb, xx);
				iv=star[j];
				iv*=iv;
				clc=star[zp-j];
				clc*=clc;
				iv+=clc;
				iv=sqrt(iv);
				if (yx<iv) yx=iv;
				g_array_append_val(ysb, iv);
			}
			g_array_append_val(sz, dx);
			g_array_append_val(nx, dx2);
			for (k=1; k<=jdimx; k++)
			{
				xx=0;
				g_array_append_val(xsb, xx);
				iv=fabs(star[k*zp]);
				g_array_append_val(ysb, iv);
				if (yx<iv) yx=iv;
				for (j=1; j<zp/2; j++)
				{
					xx=j*g_array_index(delf, gdouble, 0);
					g_array_append_val(xsb, xx);
					iv=star[j+(k*zp)];
					iv*=iv;
					clc=star[((k+1)*zp)-j];
					clc*=clc;
					iv+=clc;
					iv=sqrt(iv);
					if (yx<iv) yx=iv;
					g_array_append_val(ysb, iv);
				}
				g_array_append_val(sz, dx);
				dx2+=dx;
				g_array_append_val(nx, dx2);
			}
		}
		fftw_free(star);
		plt=PLOT_LINEAR(plot2);
		(plt->sizes)=sz;
		(plt->ind)=nx;
		(plt->xdata)=xsb;
		(plt->ydata)=ysb;
		plot_linear_update_scale_pretty(plot2, 0, xx, 0, yx);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 1);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind), 0);
		adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, jdimx, 1.0, 5.0, 0.0);
		gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(jind2), adj);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind2), 0);
		adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, G_MAXINT8, 1.0, 5.0, 0.0);
		gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(kind), adj);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(kind), 0);
		jdimxf=jdimx;
		flags|=2;
		pr_id=g_signal_connect(G_OBJECT(pr), "clicked", G_CALLBACK(prs), NULL);
	}
	else
	{
		str=g_strdup(_("Open a file for analysis first."));
		gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
		g_free(str);
	}
}
