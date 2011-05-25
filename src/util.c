/***************************************************************************
 *            util.c
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

#include "util.h"

void help(GtkWidget *widget, gpointer data)
{
	gchar *uri, *str;
	GError *Err=NULL;

	if (data) uri=g_strdup_printf("ghelp:Harmonic-help?%s", (gchar *) data);
	else uri=g_strdup("ghelp:Harmonic-help");
	gtk_show_uri(NULL, uri, gtk_get_current_event_time(), &Err);
	g_free(uri);
	if (Err)
	{
		str=g_strdup(_("Could not load help files."));
		gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
		g_free(str);
		g_error_free(Err);
	}
}

void about(GtkWidget *widget, gpointer data)
{
	GtkWidget *helpwin;

	helpwin=gtk_about_dialog_new();
	g_signal_connect_swapped(G_OBJECT(helpwin), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(helpwin));
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(helpwin), "0.1.0");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(helpwin), "(c) Paul Childs, 2011");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(helpwin), _("Harmonic is a program for performing harmonic analysis (e.g. Fourier analysis) and obtaining visibility of fringes, domain shift and chirp measurements."));
	gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(helpwin), _("translator-credits"));
	gtk_widget_show(helpwin);
	g_signal_connect_swapped(G_OBJECT(helpwin), "response", G_CALLBACK(gtk_widget_destroy), G_OBJECT(helpwin));
}

void upg(GtkWidget *widget, gpointer data)
{
	PlotLinear *plt;
	gdouble dt, xi, xf, mny, mxy;
	gint j, sz4;
	GSList *list;

	trc=satl;
	list=group2;
	while (list)
	{
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data))) break;
		list=list->next;
		trc--;
	}
	plt=PLOT_LINEAR(plot1);
	sz4=g_array_index((plt->sizes), gint, 0);/* adjust accordingly with multitrace considerations*/
	g_array_free(yb, TRUE);
	yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
	dt=g_array_index(specs, gdouble, trc-1);
	mny=dt;
	mxy=dt;
	g_array_append_val(yb, dt);
	for (j=1; j<sz4; j++)
	{
		dt=g_array_index(specs, gdouble, trc-1+(j*satl));
		g_array_append_val(yb, dt);
		if (dt<mny) mny=dt;
		else if (dt>mxy) mxy=dt;
	}
	(plt->ydata)=yb;
	xi=g_array_index(x, gdouble, 0);
	xf=g_array_index(x, gdouble, (lc-1));
	plot_linear_update_scale(plot1, xi, xf, mny, mxy);
}

void pltmv(PlotLinear *plot, gpointer data)
{
	gchar *str;

	str=g_strdup_printf(_("x: %f, y: %f"), (plot->xps), (plot->yps));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	g_free(str);
}

void pltmvp(PlotPolar *plot, gpointer data)
{
	gchar *str;

	str=g_strdup_printf(_("x: %f, y: %f"), (plot->rps), (plot->thps));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	g_free(str);
}

void upj(GtkWidget *widget, gpointer data)
{
	/*
	 * Checks if j index spinner has increased to a new value and fills parameter array values accordingly
	 * updates front panel to display parameters for new j,k values
	 * If transform has been performed and in single plot mode, changes the graph in plot 2
	 * If processing has been performed, updates the displayed value/plot
	 */
	PlotLinear *plt2, *plt3;
	PlotPolar *plt4;
	gint j, k, l, sz4;
	gdouble num, num2, num3, num4, num5, num6, num7, xi, xf, mny, mxy;
	gdouble *ptr;
	gchar s[10];
	gchar *str;

	jdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	if (jdim>=jdimx)
	{
		num=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isr));
		num2=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isp));
		num3=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tc));
		num4=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tw));
		num5=gtk_spin_button_get_value(GTK_SPIN_BUTTON(zw));
		num6=gtk_spin_button_get_value(GTK_SPIN_BUTTON(bsr));
		num7=gtk_spin_button_get_value(GTK_SPIN_BUTTON(bsp));
		k=0;
		while (k<(kdimx-1))
		{
			for (j=jdimx; j<=jdim; j++)
			{
				g_array_insert_val(isra, j+(k*jdim), num);
				g_array_insert_val(ispa, j+(k*jdim), num2);
				g_array_insert_val(tca, j+(k*jdim), num3);
				g_array_insert_val(twa, j+(k*jdim), num4);
			}
			k++;
		}
		for (j=jdimx; j<=jdim; j++)
		{
			g_array_append_val(isra, num);
			g_array_append_val(ispa, num2);
			g_array_append_val(tca, num3);
			g_array_append_val(twa, num4);
			g_array_append_val(zwa, num5);
			g_array_append_val(bsra, num6);
			g_array_append_val(bspa, num7);
		}
		jdimx=(jdim+1);
	}
	else
	{
		num=g_array_index(isra, gdouble, (jdim+(kdim*jdimx)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(isr), num);
		num=g_array_index(ispa, gdouble, (jdim+(kdim*jdimx)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(isp), num);
		num=g_array_index(tca, gdouble, (jdim+(kdim*jdimx)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(tc), num);
		num=g_array_index(twa, gdouble, (jdim+(kdim*jdimx)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(tw), num);
		num=g_array_index(zwa, gdouble, jdim);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(zw), num);
		num6=g_array_index(bsra, gdouble, jdim);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(bsr), num6);
		num6=g_array_index(bspa, gdouble, jdim);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(bsp), num6);
	}
	if (jdim<jdimxf)
	{
		if (((flags&PROC_TRS)!=0)&&((flagd&DISP_MIJ)==0))
		{
			g_object_get(G_OBJECT(plot2), "xmin", &num, "xmax", &num2, NULL);
			plt2=PLOT_LINEAR(plot2);
			sz4=g_array_index((plt2->sizes), gint, 0);
			g_array_free(nx2, TRUE);
			nx2=g_array_new(FALSE, FALSE, sizeof(gint));
			k=jdim*sz4;
			g_array_append_val(nx2, k);
			num3=g_array_index(ysb, gdouble, k);
			for (j=k+1; j<k+sz4; j++)
			{
				num4=g_array_index(ysb, gdouble, j);
				if (num4>num3) num3=num4;
			}
			(plt2->ind)=nx2;
			plot_linear_update_scale_pretty(plot2, num, num2, 0, num3);
		}
		if ((flags&PROC_PRS)!=0)
		{
			if ((flags&PROC_BAT)!=0)
			{
				if ((flagd&DISP_MRJ)==0)
				{
					sz4=g_array_index(bsz, gint, 0);
					g_array_free(bnx, TRUE);
					if ((flagd&DISP_MRK)==0)/* single plot */
					{
						l=sz4*(kdim+(jdim*kdimxf));
						bnx=g_array_new(FALSE, FALSE, sizeof(gint));
						g_array_append_val(bnx, l);
						num=g_array_index(bxr, gdouble, 0);
						num3=g_array_index(byr, gdouble, l);
						{num2=num; num4=num3;}
						for (k=1; k<sz4; k++)
						{
							num5=g_array_index(bxr, gdouble, k);
							if (num5>num2) num2=num5;
							else if (num5<num) num=num5;
							num5=g_array_index(byr, gdouble, l+k);
							if (num5>num4) num4=num5;
							else if (num5<num3) num3=num5;
						}
					}
					else/* multiplot over k */
					{
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
						num=g_array_index(bxr, gdouble, 0);
						l=sz4*jdim*kdimxf;
						g_array_append_val(bnx, l);
						num3=g_array_index(byr, gdouble, l);
						{num2=num; num4=num3;}
						for (j=1; j<sz4; j++)
						{
							num5=g_array_index(bxr, gdouble, j);
							if (num5>num2) num2=num5;
							else if (num5<num) num=num5;
							num5=g_array_index(byr, gdouble, j+l);
							if (num5>num4) num4=num5;
							else if (num5<num3) num3=num5;
						}
						k=1;
						while (k<kdimxf)
						{
							{l+=sz4; k++;}
							g_array_append_val(bnx, l);
							for (j=0; j<sz4; j++)
							{
								num5=g_array_index(byr, gdouble, j+l);
								if (num5>num4) num4=num5;
								else if (num5<num3) num3=num5;
							}
						}
					}
					if ((flags&PROC_POL)==0)
					{
						plt3=PLOT_LINEAR(plot3);
						(plt3->sizes)=bsz;
						(plt3->ind)=bnx;
						plot_linear_update_scale_pretty(plot3, num, num2, num3, num4);
					}
					else
					{
						plt4=PLOT_POLAR(plot3);
						(plt4->sizes)=bsz;
						(plt4->ind)=bnx;
						plot_polar_update_scale_pretty(plot3, num3, num4, num, num2);
					}
				}
			}
			else if (kdim<kdimxf)
			{
				num=g_array_index(vis, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 7, "%f", num);
				gtk_label_set_text(GTK_LABEL(visl), s);
				num=g_array_index(doms, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 9, "%f", num);
				gtk_label_set_text(GTK_LABEL(dsl), s);
				if ((flags&PROC_CHP)!=0)
				{
					num=g_array_index(chp, gdouble, (jdim+(kdim*jdimxf)));
					g_snprintf(s, 8, "%f", num);
					gtk_label_set_text(GTK_LABEL(chil), s);
				}
			}
			else
			{
				str=g_strdup("");
				gtk_label_set_text(GTK_LABEL(visl), str);
				gtk_label_set_text(GTK_LABEL(dsl), str);
				if ((flags&PROC_CHP)!=0) gtk_label_set_text(GTK_LABEL(chil), str);
				g_free(str);
			}
		}
	}
	else if (((flags&PROC_PRS)!=0)&&((flags&PROC_BAT)==0))
	{
		str=g_strdup("");
		gtk_label_set_text(GTK_LABEL(visl), str);
		gtk_label_set_text(GTK_LABEL(dsl), str);
		if ((flags&PROC_CHP)!=0) gtk_label_set_text(GTK_LABEL(chil), str);
		g_free(str);
	}
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind2), jdim);
}

void upj2(GtkWidget *widget, gpointer data)
{
	gint j;

	j=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	if (j!=jdim) gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind), j);
}

void upk(GtkWidget *widget, gpointer data)
{
	/*
	 * Checks if k index spinner has increased to a new value and fills parameter array values accordingly
	 * updates front panel to display parameters for new j,k values
	 * If processing has been performed, updates the displayed value/plot
	 */
	PlotLinear *plt3;
	PlotPolar *plt4;
	gint j, k, l, sz4;
	gdouble num, num2, num3, num4, num5;
	gchar s[10];
	gchar *str;

	kdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	if (kdim>=kdimx)
	{
		num=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isr));
		num2=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isp));
		num3=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tc));
		num4=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tw));
		for (j=0; j<jdimx*(kdim-kdimx+1); j++)
		{
			g_array_append_val(isra, num);
			g_array_append_val(ispa, num2);
			g_array_append_val(tca, num3);
			g_array_append_val(twa, num4);
		}
		kdimx=(kdim+1);
	}
	else
	{
		num=g_array_index(isra, gdouble, (jdim+(kdim*jdimx)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(isr), num);
		num=g_array_index(ispa, gdouble, (jdim+(kdim*jdimx)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(isp), num);
		num=g_array_index(tca, gdouble, (jdim+(kdim*jdimx)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(tc), num);
		num=g_array_index(twa, gdouble, (jdim+(kdim*jdimx)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(tw), num);
	}
	if ((flags&PROC_PRS)!=0)
	{
		if (kdim<kdimxf)
		{
			if ((flags&PROC_BAT)!=0)
			{
				if ((flagd&DISP_MRK)==0)
				{
					sz4=g_array_index(bsz, gint, 0);
					g_array_free(bnx, TRUE);
					if ((flagd&DISP_MRJ)==0)/* single plot*/
					{
						l=sz4*(kdim+(jdim*kdimxf));
						bnx=g_array_new(FALSE, FALSE, sizeof(gint));
						g_array_append_val(bnx, l);
						num=g_array_index(bxr, gdouble, 0);
						num3=g_array_index(byr, gdouble, l);
						{num2=num; num4=num3;}
						for (k=1; k<sz4; k++)
						{
							num5=g_array_index(bxr, gdouble, k);
							if (num5>num2) num2=num5;
							else if (num5<num) num=num5;
							num5=g_array_index(byr, gdouble, l+k);
							if (num5>num4) num4=num5;
							else if (num5<num3) num3=num5;
						}
					}
					else/* multiplot over j */
					{
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
						l=sz4*kdim;
						g_array_append_val(bnx, l);
						num=g_array_index(bxr, gdouble, 0);
						num3=g_array_index(byr, gdouble, l);
						{num2=num; num4=num3;}
						for (k=1; k<sz4; k++)
						{
							num5=g_array_index(bxr, gdouble, k);
							if (num5>num2) num2=num5;
							else if (num5<num) num=num5;
							num5=g_array_index(byr, gdouble, k+l);
							if (num5>num4) num4=num5;
							else if (num5<num3) num3=num5;
						}
						j=1;
						while (j<jdimxf)
						{
							{l+=sz4*kdimxf; j++;}
							g_array_append_val(bnx, l);
							for (k=0; k<sz4; k++)
							{
								num5=g_array_index(byr, gdouble, k+l);
								if (num5>num4) num4=num5;
								else if (num5<num3) num3=num5;
							}
						}
					}
					if ((flags&PROC_POL)==0)
					{
						plt3=PLOT_LINEAR(plot3);
						(plt3->sizes)=bsz;
						(plt3->ind)=bnx;
						plot_linear_update_scale_pretty(plot3, num, num2, num3, num4);
					}
					else
					{
						plt4=PLOT_POLAR(plot3);
						(plt4->sizes)=bsz;
						(plt4->ind)=bnx;
						plot_polar_update_scale_pretty(plot3, num3, num4, num, num2);
					}
				}
			}
			else if (jdim<jdimxf)
			{
				num=g_array_index(vis, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 7, "%f", num);
				gtk_label_set_text(GTK_LABEL(visl), s);
				num=g_array_index(doms, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 9, "%f", num);
				gtk_label_set_text(GTK_LABEL(dsl), s);
				if ((flags&PROC_CHP)!=0)
				{
					num=g_array_index(chp, gdouble, (jdim+(kdim*jdimxf)));
					g_snprintf(s, 8, "%f", num);
					gtk_label_set_text(GTK_LABEL(chil), s);
				}
			}
			else
			{
				str=g_strdup("");
				gtk_label_set_text(GTK_LABEL(visl), str);
				gtk_label_set_text(GTK_LABEL(dsl), str);
				if ((flags&PROC_CHP)!=0) gtk_label_set_text(GTK_LABEL(chil), str);
				g_free(str);
			}
		}
		else if ((flags&PROC_BAT)==0)
		{
			str=g_strdup("");
			gtk_label_set_text(GTK_LABEL(visl), str);
			gtk_label_set_text(GTK_LABEL(dsl), str);
			if ((flags&PROC_CHP)!=0) gtk_label_set_text(GTK_LABEL(chil), str);
			g_free(str);
		}
	}
}

void upa2(GtkWidget *widget, gpointer data)
{
	gdouble *ptr;

	ptr=&g_array_index((GArray*)data, gdouble, jdim+(kdim*jdimx));
	*ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
}

void upa1(GtkWidget *widget, gpointer data)
{
	gdouble *ptr;

	ptr=&g_array_index((GArray*)data, gdouble, jdim);
	*ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
}

void reset(GtkWidget *widget, gpointer data)
{
	gdouble *ptr;
	
	{jdim=0; jdimx=1; kdim=0; kdimx=1;}
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind2), 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(kind), 0);
	{(bsra->len)=1; (bspa->len)=1;}
	{(isra->len)=1; (ispa->len)=1; (tca->len)=1; (twa->len)=1; (zwa->len)=1;}
	ptr=&g_array_index(bsra, gdouble, 0);
	*ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(bsr));
	ptr=&g_array_index(bspa, gdouble, 0);
	*ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(bsp));
	ptr=&g_array_index(isra, gdouble, 0);
	*ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isr));
	ptr=&g_array_index(ispa, gdouble, 0);
	*ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isp));
	ptr=&g_array_index(tca, gdouble, 0);
	*ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tc));
	ptr=&g_array_index(twa, gdouble, 0);
	*ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tw));
	ptr=&g_array_index(zwa, gdouble, 0);
	*ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(zw));
}

void reset2(GtkWidget *widget, gpointer data)
{/*update arrays to current values*/
	gdouble *ptr;
	gdouble num;
	
	{kdim=0; kdimx=1;}
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(kind), 0);
	{(isra->len)=jdimx; (ispa->len)=jdimx; (tca->len)=jdimx; (twa->len)=jdimx;}
	num=g_array_index(isra, gdouble, jdim);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(isr), num);
	num=g_array_index(ispa, gdouble, jdim);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(isp), num);
	num=g_array_index(tca, gdouble, jdim);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(tc), num);
	num=g_array_index(twa, gdouble, jdim);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(tw), num);
}

