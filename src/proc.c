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
	gchar *str;
	gchar s[10];
	gdouble cn, ct, ddp, delf, dst, idelf, iv, ivd, ivdt, phi, phio, phia, pn, tcn, tp, twd, vt, vzt;
	gint j, jdim, jdimx, k, kdim, l, sp, sr, sz4;
	GtkPlot *pt;
	GtkPlotLinear *plt;
	GtkWidget *label;

	if ((flags&PROC_TRS)!=0)
	{
		pt=GTK_PLOT(plot2);
		plt=GTK_PLOT_LINEAR(plot2);
		sz4=g_array_index(pt->sizes, gint, 0);/* check placing of this with what is desired for multiplots (within for loop?) */
		jdimx=(plt->ydata->len)/(2*sz4);
		if (jdimx==(bsra->len))
		{
			if ((flags&PROC_BAT)!=0)
			{
				gtk_notebook_remove_page(GTK_NOTEBOOK(notebook2), 2);
				rest=gtk_table_new(4, 2, FALSE);
				gtk_widget_show(rest);
				label=gtk_label_new(_("Visibility"));
				gtk_table_attach(GTK_TABLE(rest), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
				gtk_widget_show(label);
				visl=gtk_label_new("");
				gtk_table_attach(GTK_TABLE(rest), visl, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
				gtk_widget_show(visl);
				label=gtk_label_new(_("Domain Shift"));
				gtk_table_attach(GTK_TABLE(rest), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
				gtk_widget_show(label);
				dsl=gtk_label_new("");
				gtk_table_attach(GTK_TABLE(rest), dsl, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
				gtk_widget_show(dsl);
				label=gtk_label_new(_("Chirp"));
				gtk_table_attach(GTK_TABLE(rest), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
				gtk_widget_show(label);
				chil=gtk_label_new("");
				gtk_table_attach(GTK_TABLE(rest), chil, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
				gtk_widget_show(chil);
				label=gtk_label_new(_("Analysis Results"));
				gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), rest, label);
				flags^=PROC_BAT;
			}
			{g_array_free(vis, FALSE); g_array_free(doms, FALSE); g_array_free(chp, FALSE);}
			kdimxf=(isra->len)/jdimx;
			{vis=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx*kdimxf); doms=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx*kdimxf); chp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx*kdimxf);}
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(opttri)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(chi)))
				{
for (j=0; j<jdimx; j++)
{
	delf=g_array_index(plt->xdata, gdouble, (2*j*sz4)+1)-g_array_index(plt->xdata, gdouble, 2*j*sz4);
	if (iv<DZE) idelf=G_MAXDOUBLE;
	else idelf=1/delf;
	vzt=g_array_index(plt->ydata, gdouble, 2*j*sz4);
						/*
						 fit values to zwa
						 */
	iv=g_array_index(zwa, gdouble, j)*idelf/2;
	for (l=1; l<iv; l++) vzt+=g_array_index(plt->ydata, gdouble, 2*(l+(j*sz4)));
	if (vzt<DZE) vzt=G_MAXDOUBLE;
	else vzt=l/vzt;
	for (k=0; k<kdimxf; k++)
	{
		sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
		sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
							/*
							 fit values to twa and tca
							 */
		tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
		twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
		if ((sr<(sz4-2))&&(sp<sz4)&&((sp-sr)>1))
		{
			vt=g_array_index(plt->ydata, gdouble, 2*(sr+(j*sz4)));
			phia=g_array_index(plt->ydata, gdouble, 1+(2*(sr+(j*sz4))));
			vt+=g_array_index(plt->ydata, gdouble, 2*(1+sr+(j*sz4)));
			phio=-g_array_index(plt->ydata, gdouble, 1+(2*(1+sr+(j*sz4))));
			phia+=phio;
			{pn=0; cn=0; dst=0; ddp=0;}
			for (l=sr+2; l<=sp; l++)
			{
				vt+=g_array_index(plt->ydata, gdouble, 2*(l+(j*sz4)));
				phi=g_array_index(plt->ydata, gdouble, 1+(2*(l+(j*sz4))));
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
			}
			pn*=NMY_2PI*delf;
			if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
			else dst/=pn;
			cn*=G_PI*NMY_2PI*delf*delf;
			if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
			else ddp=cn/ddp;
			vt*=vzt/(sp-sr+1);
		}
		else
		{
			str=g_strdup_printf(_("Insufficient windowing range in channel %d, %d."), j, k);
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			{g_free(str); vt=0; dst=0; ddp=0;}
		}
		{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
	}
}
jdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(jind2));
kdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(kind));
vt=g_array_index(chp, gdouble, (kdim+(jdim*kdimxf)));
g_snprintf(s, 8, "%f", vt);
gtk_label_set_text(GTK_LABEL(chil), s);
flags|=(PROC_CHP|PROC_PRS);
				}
				else
				{
for (j=0; j<jdimx; j++)
{
	delf=g_array_index(plt->xdata, gdouble, (2*j*sz4)+1)-g_array_index(plt->xdata, gdouble, 2*j*sz4);
	if (iv<DZE) idelf=G_MAXDOUBLE;
	else idelf=1/delf;
	vzt=g_array_index(plt->ydata, gdouble, 2*j*sz4);
						/*
						 fit values to zwa
						 */
	iv=g_array_index(zwa, gdouble, j)*idelf/2;
	for (l=1; l<iv; l++) vzt+=g_array_index(plt->ydata, gdouble, 2*(l+(j*sz4)));
	if (vzt<DZE) vzt=G_MAXDOUBLE;
	else vzt=l/vzt;
	for (k=0; k<kdimxf; k++)
	{
		sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
		sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
							/*
							 fit values to twa and tca
							 */
		tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
		twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
		if ((sr<(sz4-1))&&(sp<sz4)&&((sp-sr)>0))
		{
			vt=g_array_index(plt->ydata, gdouble, 2*(sr+(j*sz4)));
			phio=-g_array_index(plt->ydata, gdouble, 1+(2*(sr+(j*sz4))));
			{dst=0; pn=0;}
			for (l=sr+1; l<=sp; l++)
			{
				vt+=g_array_index(plt->ydata, gdouble, 2*(l+(j*sz4)));
				phi=g_array_index(plt->ydata, gdouble, 1+(2*(l+(j*sz4))));
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
			}
			pn*=NMY_2PI*delf;
			dst=dst/pn;
			vt*=vzt/(sp-sr+1);
		}
		else
		{
			str=g_strdup_printf(_("Insufficient windowing range in channel %d, %d."), j, k);
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			{g_free(str); vt=0; dst=0;}
		}
		g_array_append_val(vis, vt);
		g_array_append_val(doms, dst);
	}
}
jdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(jind2));
kdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(kind));
gtk_label_set_text(GTK_LABEL(chil), "");
flags|=PROC_PRS;
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(chi)))
			{
for (j=0; j<jdimx; j++)
{
	delf=g_array_index(plt->xdata, gdouble, (2*j*sz4)+1)-g_array_index(plt->xdata, gdouble, 2*j*sz4);
	if (iv<DZE) idelf=G_MAXDOUBLE;
	else idelf=1/delf;
	vzt=g_array_index(plt->ydata, gdouble, 2*j*sz4);
	iv=g_array_index(zwa, gdouble, j)*idelf/2;
	for (l=1; l<iv; l++) vzt+=g_array_index(plt->ydata, gdouble, 2*(l+(j*sz4)));
	if (vzt<DZE) vzt=G_MAXDOUBLE;
	else vzt=l/vzt;
	for (k=0; k<kdimxf; k++)
	{
		sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
		sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
		tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
		twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
		if ((sr<(sz4-2))&&(sp<sz4)&&((sp-sr)>1))
		{
			vt=g_array_index(plt->ydata, gdouble, 2*(sr+(j*sz4)));
			phia=g_array_index(plt->ydata, gdouble, 1+(2*(sr+(j*sz4))));
			vt+=g_array_index(plt->ydata, gdouble, 2*(1+sr+(j*sz4)));
			phio=-g_array_index(plt->ydata, gdouble, 1+(2*(1+sr+(j*sz4))));
			phia+=phio;
			{pn=0; cn=0; dst=0; ddp=0;}
			for (l=sr+2; l<=sp; l++)
			{
				vt+=g_array_index(plt->ydata, gdouble, 2*(l+(j*sz4)));
				phi=g_array_index(plt->ydata, gdouble, 1+(2*(l+(j*sz4))));
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
			}
			pn*=NMY_2PI*delf;
			if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
			else dst/=pn;
			cn*=G_PI*NMY_2PI*delf*delf;
			if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
			else ddp=cn/ddp;
			vt*=vzt/(sp-sr+1);
		}
		else
		{
			str=g_strdup_printf(_("Insufficient windowing range in channel %d, %d."), j, k);
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			{g_free(str); vt=0; dst=0; ddp=0;}
		}
		{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
	}
}
jdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(jind2));
kdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(kind));
vt=g_array_index(chp, gdouble, (kdim+(jdim*kdimxf)));
g_snprintf(s, 8, "%f", vt);
gtk_label_set_text(GTK_LABEL(chil), s);
flags|=(PROC_CHP|PROC_PRS);
			}
			else
			{
for (j=0; j<jdimx; j++)
{
	delf=g_array_index(plt->xdata, gdouble, (2*j*sz4)+1)-g_array_index(plt->xdata, gdouble, 2*j*sz4);
	if (iv<DZE) idelf=G_MAXDOUBLE;
	else idelf=1/delf;
	vzt=g_array_index(plt->ydata, gdouble, 2*j*sz4);
	iv=g_array_index(zwa, gdouble, j)*idelf/2;
	for (l=1; l<iv; l++) vzt+=g_array_index(plt->ydata, gdouble, 2*(l+(j*sz4)));
	if (vzt<DZE) vzt=G_MAXDOUBLE;
	else vzt=l/vzt;
	for (k=0; k<kdimxf; k++)
	{
		sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
		sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
		tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
		twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
		if ((sr<(sz4-1))&&(sp<sz4)&&((sp-sr)>0))
		{
			vt=g_array_index(plt->ydata, gdouble, 2*(sr+(j*sz4)));
			phio=-g_array_index(plt->ydata, gdouble, 1+(2*(sr+(j*sz4))));
			{dst=0; pn=0;}
			for (l=sr+1; l<=sp; l++)
			{
				vt+=g_array_index(plt->ydata, gdouble, 2*(l+(j*sz4)));
				phi=g_array_index(plt->ydata, gdouble, 1+(2*(l+(j*sz4))));
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
			}
			pn*=NMY_2PI*delf;
			dst=dst/pn;
			vt*=vzt/(sp-sr+1);
		}
		else
		{
			str=g_strdup_printf(_("Insufficient windowing range in channel %d, %d."), j, k);
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			{g_free(str); vt=0; dst=0;}
		}
		g_array_append_val(vis, vt);
		g_array_append_val(doms, dst);
	}
}
jdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(jind2));
kdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(kind));
gtk_label_set_text(GTK_LABEL(chil), "");
flags|=PROC_PRS;
			}
			vt=g_array_index(vis, gdouble, (kdim+(jdim*kdimxf)));
			g_snprintf(s, 7, "%f", vt);
			gtk_label_set_text(GTK_LABEL(visl), s);
			vt=g_array_index(doms, gdouble, (kdim+(jdim*kdimxf)));
			g_snprintf(s, 9, "%f", vt);
			gtk_label_set_text(GTK_LABEL(dsl), s);
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 2);
		}
		else
		{
			str=g_strdup_printf(_("Parameter array dimensions (%d) do not match transformed data (%d)."), (bsra->len), jdimx);
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
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
	double *star, *yt;
	fftw_plan p;
	fftw_r2r_kind type=FFTW_R2HC;
	GArray *delf, *nx, *st, *sz, *x, *y;
	gchar *str;
	gdouble ce, clc, iv, iv2, ofe, ofs, xx, yx;
	gint dx, dx2, j, jdim, jdimx, k, n, satl, sp, sr, sz4, trc, zp;
	GtkPlot *pt;
	GtkPlotLinear *plt;
	GtkWidget *label;

	if ((flags&PROC_OPN)!=0)
	{
		if ((flags&PROC_BAT)!=0)
		{
			gtk_notebook_remove_page(GTK_NOTEBOOK(notebook2), 2);
			rest=gtk_table_new(4, 2, FALSE);
			gtk_widget_show(rest);
			label=gtk_label_new(_("Visibility"));
			gtk_table_attach(GTK_TABLE(rest), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_widget_show(label);
			visl=gtk_label_new("");
			gtk_table_attach(GTK_TABLE(rest), visl, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_widget_show(visl);
			label=gtk_label_new(_("Domain Shift"));
			gtk_table_attach(GTK_TABLE(rest), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_widget_show(label);
			dsl=gtk_label_new("");
			gtk_table_attach(GTK_TABLE(rest), dsl, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_widget_show(dsl);
			label=gtk_label_new(_("Analysis Results"));
			gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), rest, label);
			flags^=PROC_BAT;
		}
		jdimx=bsra->len;
		delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
		zp=1<<(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(zpd)));
		n=zp*jdimx;
		yt=fftw_malloc(sizeof(double)*n);
		plt=GTK_PLOT_LINEAR(plot1);
		pt=GTK_PLOT(plot1);
		{trc=g_array_index(pt->ind, gint, 0); satl=g_array_index(pt->stride, gint, 0); sz4=g_array_index(pt->sizes, gint, 0);}
		ce=0;
		for (j=1; j<=16; j++) ce+=g_array_index((plt->ydata), gdouble, trc+((sz4-j)*satl));
		ce/=16;
		for (j=0; j<n; j++) yt[j]=0;
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(twopionx)))/* interpolate */
		{
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(wll)))/* window based offset */
			{
				flags&=(PROC_OPN|PROC_TRS|PROC_PRS|PROC_BAT|PROC_CHP|PROC_POL|PROC_COM|PROC_RI);
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss- */
							{
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
									/* fill array */
								}
							}
							else /* +TdBss- */
							{
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
									/* fill array */
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss- */
						{
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
								/* fill array */
							}
						}
						else /* +RdBss- */
						{
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
								/* fill array */
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss- and +Tlss- */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
							/* fill array */
						}
					}
					else /* -Rlss- +Rlss- */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
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
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
								/* fill array */
							}
						}
						else /* +TdB0- */
						{
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
								/* fill array */
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0- */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
							/* fill array */
						}
					}
					else /* +RdB0- */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
							/* fill array */
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0- and +Tl0- */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
						/* fill array */
					}
				}
				else /* -Rl0- +Rl0- */
				{
					for (j=0; j<jdimx; j++)
					{
						iv=g_array_index(bsra, gdouble, j);
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
						/* fill array */
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(oft)))/* history based offset */
			{
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
				if ((flags&PROC_OFT)!=0)
				{
					ofs+=ce-oe;
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(fst), ofs);
				}
				else flags|=PROC_OFT;
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss- */
							{
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
									/* fill array */
								}
							}
							else /* +TdBss- */
							{
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
									/* fill array */
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss- */
						{
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
								/* fill array */
							}
						}
						else /* +RdBss- */
						{
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
								/* fill array */
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss- and +Tlss- */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
							/* fill array */
						}
					}
					else /* -Rlss- +Rlss- */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
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
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
								/* fill array */
							}
						}
						else /* +TdB0- */
						{
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
								/* fill array */
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0- */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
							/* fill array */
						}
					}
					else /* +RdB0- */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
							/* fill array */
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0- and +Tl0- */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
						/* fill array */
					}
				}
				else /* -Rl0- +Rl0- */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
						/* fill array */
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))/* user defined offset */
			{
				flags&=(PROC_OPN|PROC_TRS|PROC_PRS|PROC_BAT|PROC_CHP|PROC_POL|PROC_COM|PROC_RI);
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss- */
						{
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
								/* fill array */
							}
						}
						else /* +TdBss- */
						{
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
								/* fill array */
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss- */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
							/* fill array */
						}
					}
					else /* +RdBss- */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
							/* fill array */
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss- +Tlss- */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
						/* fill array */
					}
				}
				else /* -Rlss- +Rlss- */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
						/* fill array */
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
			{
				flags&=(PROC_OPN|PROC_TRS|PROC_PRS|PROC_BAT|PROC_CHP|PROC_POL|PROC_COM|PROC_RI);
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0- */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
							/* fill array */
						}
					}
					else /* +TdB0- */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
							/* fill array */
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0- */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
						/* fill array */
					}
				}
				else /* +RdB0- */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
						/* fill array */
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0- +Tl0- */
			{
				flags&=(PROC_OPN|PROC_TRS|PROC_PRS|PROC_BAT|PROC_CHP|PROC_POL|PROC_COM|PROC_RI);
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
				for (j=0; j<jdimx; j++)
				{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
					/* fill array */
				}
			}
			else /* -Rl0- +Rl0- */
			{
				flags&=(PROC_OPN|PROC_TRS|PROC_PRS|PROC_BAT|PROC_CHP|PROC_POL|PROC_COM|PROC_RI);
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
				for (j=0; j<jdimx; j++)
				{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
					/* fill array */
				}
			}
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(wll)))/* window based offset */
		{
			flags&=(PROC_OPN|PROC_TRS|PROC_PRS|PROC_BAT|PROC_CHP|PROC_POL|PROC_COM|PROC_RI);
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssw+ */
						{
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
if (sp<16) {ofs=g_array_index((plt->ydata), gdouble, trc+(sr*satl)); ofe=g_array_index((plt->ydata), gdouble, trc+((sp+sr-1)*satl));}
else
{
	{ofs=g_array_index((plt->ydata), gdouble, trc+(sr*satl)); ofe=g_array_index((plt->ydata), gdouble, trc+((sp+sr-1)*satl)); k=1;}
	while (k<8)
	{
		ofs+=g_array_index((plt->ydata), gdouble, trc+((sr+k)*satl));
		ofe+=g_array_index((plt->ydata), gdouble, trc+((sp+sr-(++k))*satl));
	}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
								for (k=0; k<sp; k++)
								{
									clc=ofs+(k*(ofe-ofs)/(sp-1))-g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl));
									if (clc<NZE) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
									else yt[k+(j*zp)]=-G_MAXDOUBLE;
								}
							}
							gtk_spin_button_set_value(GTK_SPIN_BUTTON(fst), (ofs+ofe)/2);
						}
						else /* +TdBssw+ */
						{
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
if (sp<16) {ofs=g_array_index((plt->ydata), gdouble, trc+(sr*satl)); ofe=g_array_index((plt->ydata), gdouble, trc+((sp+sr-1)*satl));}
else
{
	{ofs=g_array_index((plt->ydata), gdouble, trc+(sr*satl)); ofe=g_array_index((plt->ydata), gdouble, trc+((sp+sr-1)*satl)); k=1;}
	while (k<8)
	{
		ofs+=g_array_index((plt->ydata), gdouble, trc+((sr+k)*satl));
		ofe+=g_array_index((plt->ydata), gdouble, trc+((sp+sr-(++k))*satl));
	}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
								for (k=0; k<sp; k++)
								{
									clc=g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))-ofs-(k*(ofe-ofs)/(sp-1));
									if (clc<NZE) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
									else yt[k+(j*zp)]=-G_MAXDOUBLE;
								}
							}
							gtk_spin_button_set_value(GTK_SPIN_BUTTON(fst), (ofs+ofe)/2);
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssw+ */
					{
						ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
							for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(ofs-g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl)));
						}
					}
					else /* +RdBssw+ */
					{
						ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
							for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))-ofs);
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssw+ +Tlssw+ */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
if (sp<16) {ofs=g_array_index((plt->ydata), gdouble, trc+(sr*satl)); ofe=g_array_index((plt->ydata), gdouble, trc+((sp+sr-1)*satl));}
else
{
	{ofs=g_array_index((plt->ydata), gdouble, trc+(sr*satl)); ofe=g_array_index((plt->ydata), gdouble, trc+((sp+sr-1)*satl)); k=1;}
	while (k<8)
	{
		ofs+=g_array_index((plt->ydata), gdouble, trc+((sr+k)*satl));
		ofe+=g_array_index((plt->ydata), gdouble, trc+((sp+sr-(++k))*satl));
	}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=ofs+(k*(ofe-ofs)/(sp-1));
							if (clc<DZE && clc>NZE) yt[k+(j*zp)]=-G_MAXDOUBLE;
							else
							{
								clc=1-(g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))/clc);
								if (clc>DZE) yt[k+(j*zp)]=log(clc);
								else yt[k+(j*zp)]=-G_MAXDOUBLE;
							}
						}
					}
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(fst), (ofs+ofe)/2);
				}
				else /* -Rlssw+ +Rlssw+ */
				{
					ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))/ofs;
							if (clc>0) yt[k+(j*zp)]=log(clc);
							else yt[k+(j*zp)]=-G_MAXDOUBLE;
						}
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0w+ */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
if (sp<16) {ofs=g_array_index((plt->ydata), gdouble, trc+(sr*satl)); ofe=g_array_index((plt->ydata), gdouble, trc+((sp+sr-1)*satl));}
else
{
	{ofs=g_array_index((plt->ydata), gdouble, trc+(sr*satl)); ofe=g_array_index((plt->ydata), gdouble, trc+((sp+sr-1)*satl)); k=1;}
	while (k<8)
	{
		ofs+=g_array_index((plt->ydata), gdouble, trc+((sr+k)*satl));
		ofe+=g_array_index((plt->ydata), gdouble, trc+((sp+sr-(++k))*satl));
	}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
							for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(ofs+(k*(ofe-ofs)/(sp-1))-g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))));
						}
						gtk_spin_button_set_value(GTK_SPIN_BUTTON(fst), (ofs+ofe)/2);
					}
					else /* +TdB0w+ */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
if (sp<16) {ofs=g_array_index((plt->ydata), gdouble, trc+(sr*satl)); ofe=g_array_index((plt->ydata), gdouble, trc+((sp+sr-1)*satl));}
else
{
	{ofs=g_array_index((plt->ydata), gdouble, trc+(sr*satl)); ofe=g_array_index((plt->ydata), gdouble, trc+((sp+sr-1)*satl)); k=1;}
	while (k<8)
	{
		ofs+=g_array_index((plt->ydata), gdouble, trc+((sr+k)*satl));
		ofe+=g_array_index((plt->ydata), gdouble, trc+((sp+sr-(++k))*satl));
	}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
							for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))-ofs-(k*(ofe-ofs)/(sp-1))));
						}
						gtk_spin_button_set_value(GTK_SPIN_BUTTON(fst), (ofs+ofe)/2);
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0w+ */
				{
					ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
						for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(ofs-g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))));
					}
				}
				else /* +RdB0w+ */
				{
					ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
						for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))-ofs));
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0w+ +Tl0w+ */
			{
				for (j=0; j<jdimx; j++)
				{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
if (sp<16) {ofs=g_array_index((plt->ydata), gdouble, trc+(sr*satl)); ofe=g_array_index((plt->ydata), gdouble, trc+((sp+sr-1)*satl));}
else
{
	{ofs=g_array_index((plt->ydata), gdouble, trc+(sr*satl)); ofe=g_array_index((plt->ydata), gdouble, trc+((sp+sr-1)*satl)); k=1;}
	while (k<8)
	{
		ofs+=g_array_index((plt->ydata), gdouble, trc+((sr+k)*satl));
		ofe+=g_array_index((plt->ydata), gdouble, trc+((sp+sr-(++k))*satl));
	}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
					for (k=0; k<sp; k++)
					{
						clc=ofs+(k*(ofe-ofs)/(sp-1));
						if (clc<DZE && clc>NZE) yt[k+(j*zp)]=-G_MAXDOUBLE;
						else yt[k+(j*zp)]=1-(g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))/clc);
					}
				}
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(fst), (ofs+ofe)/2);
			}
			else /* -Rl0w+ +Rl0w+ */
			{
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
				for (j=0; j<jdimx; j++)
				{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) yt[k+(j*zp)]=g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))/ofs;
				}
			}
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(oft)))/* history based offset */
		{
			ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
			if ((flags&PROC_OFT)!=0)
			{
				ofs+=ce-oe;
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(fst), ofs);
			}
			else flags|=PROC_OFT;
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssh+ */
						{
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
								for (k=0; k<sp; k++)
								{
									clc=ofs-g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl));
									if (clc<0) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
									else yt[k+(j*zp)]=-G_MAXDOUBLE;
								}
							}
						}
						else /* +TdBssh+ */
						{
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
								for (k=0; k<sp; k++)
								{
									clc=g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))-ofs;
									if (clc<0) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
									else yt[k+(j*zp)]=-G_MAXDOUBLE;
								}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssh+ */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
							for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(ofs-g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl)));
						}
					}
					else /* +RdBssh+ */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
							for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))-ofs);
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssh+ +Tlssh+ */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=1-(g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))/ofs);
							if (clc>0) yt[k+(j*zp)]=log(clc);
							else yt[k+(j*zp)]=-G_MAXDOUBLE;
						}
					}
				}
				else /* -Rlssh+ +Rlssh+ */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))/ofs;
							if (clc>0) yt[k+(j*zp)]=log(clc);
							else yt[k+(j*zp)]=-G_MAXDOUBLE;
						}
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0h+ */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
							for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(ofs-g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))));
						}
					}
					else /* +TdB0h+ */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
							for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))-ofs));
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0h+ */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
						for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(ofs-g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))));
					}
				}
				else /* +RdB0h+ */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
						for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))-ofs));
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0h+ +Tl0h+ */
			{
				for (j=0; j<jdimx; j++)
				{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) yt[k+(j*zp)]=1-(g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))/ofs);
				}
			}
			else /* -Rl0h+ +Rl0h+ */
			{
				for (j=0; j<jdimx; j++)
				{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) yt[k+(j*zp)]=g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))/ofs;
				}
			}
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))/* user defined offset */
		{
			flags&=(PROC_OPN|PROC_TRS|PROC_PRS|PROC_BAT|PROC_CHP|PROC_POL|PROC_COM|PROC_RI);
			ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBsso+ */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
							for (k=0; k<sp; k++)
							{
								clc=ofs-g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl));
								if (clc<0) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
								else yt[k+(j*zp)]=-G_MAXDOUBLE;
							}
						}
					}
					else /* +TdBsso+ */
					{
						for (j=0; j<jdimx; j++)
						{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
							for (k=0; k<sp; k++)
							{
								clc=g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))-ofs;
								if (clc<0) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
								else yt[k+(j*zp)]=-G_MAXDOUBLE;
							}
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBsso+ */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
						for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(ofs-g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl)));
					}
				}
				else /* +RdBsso+ */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
						for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))-ofs);
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlsso+ +Tlsso+ */
			{
if ((ofs<DZE)&&(ofs>NZE))
{
	str=g_strdup(_("Offset must be nonzero for linear measurements."));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	g_free(str);
}
				else
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=1-(g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))/ofs);
							if (clc>0) yt[k+(j*zp)]=log(clc);
							else yt[k+(j*zp)]=-G_MAXDOUBLE;
						}
					}
				}
			}
			else /* -Rlsso+ +Rlsso+ */
			{
if ((ofs<DZE)&&(ofs>NZE))
{
	str=g_strdup(_("Offset must be nonzero for linear measurements."));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	g_free(str);
}
				else
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
						for (k=0; k<sp; k++)
						{
							clc=g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))/ofs;
							if (clc>0) yt[k+(j*zp)]=log(clc);
							else yt[k+(j*zp)]=-G_MAXDOUBLE;
						}
					}
				}
			}
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
		{
			flags&=(PROC_OPN|PROC_TRS|PROC_PRS|PROC_BAT|PROC_CHP|PROC_POL|PROC_COM|PROC_RI);
			ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0o+ */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
						for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(ofs-g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))));
					}
				}
				else /* +TdB0o+ */
				{
					for (j=0; j<jdimx; j++)
					{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
						for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))-ofs));
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0o+ */
			{
				for (j=0; j<jdimx; j++)
				{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(ofs-g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))));
				}
			}
			else /* +RdB0o+ */
			{
				for (j=0; j<jdimx; j++)
				{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))-ofs));
				}
			}
		}
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0o+ and +Tl0o+ */
		{
			flags&=(PROC_OPN|PROC_TRS|PROC_PRS|PROC_BAT|PROC_CHP|PROC_POL|PROC_COM|PROC_RI);
			ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
if ((ofs<DZE)&&(ofs>NZE))
{
	str=g_strdup(_("Offset must be nonzero for linear measurements."));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	g_free(str);
}
			else
			{
				for (j=0; j<jdimx; j++)
				{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) yt[k+(j*zp)]=1-(g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))/ofs);
				}
			}
		}
		else /* -Rl0o+ and +Rl0o+ */
		{
			flags&=(PROC_OPN|PROC_TRS|PROC_PRS|PROC_BAT|PROC_CHP|PROC_POL|PROC_COM|PROC_RI);
			ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
if ((ofs<DZE)&&(ofs>NZE))
{
	str=g_strdup(_("Offset must be nonzero for linear measurements."));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	g_free(str);
}
			else
			{
				for (j=0; j<jdimx; j++)
				{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<sz4)&&(iv>g_array_index((plt->xdata), gdouble, trc+(k*satl)))) k++;
sp=k-sr;
if (sp>zp)
{
	str=g_strdup_printf(_("Some clipping occured in channel %d. Increase zero padding."), j);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{sp=zp; g_free(str);}
}
iv=(sp-1)/(zp*(g_array_index((plt->xdata), gdouble, trc+(satl*(sp+sr-1)))-g_array_index((plt->xdata), gdouble, trc+(satl*sr))));
g_array_append_val(delf, iv);
					for (k=0; k<sp; k++) yt[k+(j*zp)]=g_array_index((plt->ydata), gdouble, trc+((k+sr)*satl))/ofs;
				}
			}
		}
		star=fftw_malloc(sizeof(double)*n);
		p=fftw_plan_many_r2r(1, &zp, jdimx, yt, NULL, 1, zp, star, NULL, 1, zp, &type, FFTW_ESTIMATE);
		fftw_execute(p);
		{fftw_destroy_plan(p); fftw_free(yt);}
		{n>>=1; dx=zp>>1;}
		{x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n); y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);}
		xx=0;
		g_array_append_val(x, xx);
		g_array_append_val(x, xx);
		iv=fabs(star[0]);
		g_array_append_val(y, iv);
		g_array_append_val(y, xx);
		for (j=1; j<dx; j++)
		{
			xx=j*g_array_index(delf, gdouble, 0);
			g_array_append_val(x, xx);
			g_array_append_val(x, xx);
			{iv=star[j]; clc=star[zp-j];}
			iv2=atan2(iv, clc);
			iv=sqrt((iv*iv)+(clc*clc));
			g_array_append_val(y, iv);
			g_array_append_val(y, iv2);
		}
		k=1;
		while (k<jdimx)
		{
			xx=0;
			g_array_append_val(x, xx);
			g_array_append_val(x, xx);
			iv=fabs(star[k*zp]);
			g_array_append_val(y, iv);
			g_array_append_val(y, xx);
			for (j=1; j<dx; j++)
			{
				xx=j*g_array_index(delf, gdouble, 0);
				g_array_append_val(x, xx);
				{iv=star[j+(k*zp)]; clc=star[((k+1)*zp)-j];}
				iv2=atan2(iv, clc);
				iv=sqrt((iv*iv)+(clc*clc));
				g_array_append_val(y, iv);
				g_array_append_val(y, iv2);
			}
			k++;
		}
		{fftw_free(star); g_array_free(delf, FALSE);}
		dx2=2;
		if ((flagd&DISP_MIJ)==0)
		{
			jdim=zp*gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(jind));
			{nx=g_array_new(FALSE, FALSE, sizeof(gint)); sz=g_array_new(FALSE, FALSE, sizeof(gint)); st=g_array_new(FALSE, FALSE, sizeof(gint));}
			{g_array_append_val(nx, jdim); g_array_append_val(sz, dx); g_array_append_val(st, dx2);}
			yx=g_array_index(y, gdouble, jdim);
			for (j=2; j<zp; j+=2)
			{
				iv=g_array_index(y, gdouble, j+jdim);
				if (yx<iv) yx=iv;
			}
		}
		else
		{
			{nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx); st=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx);}
			for (j=0;j<(2*n);j+=zp) {g_array_append_val(nx, j); g_array_append_val(sz, dx); g_array_append_val(st, dx2);}
			yx=g_array_index(y, gdouble, 0);
			for (j=2; j<n; j+=2)
			{
				iv=g_array_index(y, gdouble, j);
				if (yx<iv) yx=iv;
			}
		}
		{oe=ce; flags|=PROC_TRS;}
		plt=GTK_PLOT_LINEAR(plot2);
		gtk_plot_linear_set_data(plt, x, y, nx, sz, st);
		{g_array_unref(x); g_array_unref(y); g_array_unref(nx); g_array_unref(sz); g_array_unref(st);}
		gtk_plot_linear_update_scale_pretty(plot2, 0, xx, 0, yx);
		{gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1); gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 1);}
		g_signal_connect(G_OBJECT(pr), "clicked", G_CALLBACK(prs), NULL);
	}
	else
	{
		str=g_strdup(_("Open a file for analysis first."));
		gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
		g_free(str);
	}
}
