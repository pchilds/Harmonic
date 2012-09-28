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
	GArray *nx;
	gdouble dt, mny, mxy, xi, xf;
	gint j, satl, sz4, trc;
	GSList *list;
	GtkPlot *pt;
	GtkPlotLinear *plt;

	plt=GTK_PLOT_LINEAR(plot1);
	pt=GTK_PLOT(plot1);
	satl=g_array_index(pt->stride, gint, 0);
	trc=satl-1;
	list=group2;
	while (list)
	{
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data))) break;
		{list=(list->next); trc--;}
	}
	sz4=g_array_index((pt->sizes), gint, 0);/* adjust accordingly with multitrace considerations*/
	nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1);
	g_array_append_val(nx, trc);
	{xi=g_array_index(plt->xdata, gdouble, trc); dt=g_array_index(plt->ydata, gdouble, trc);}
	{mny=dt; mxy=dt;}
	for (j=1; j<sz4; j++)
	{
		dt=g_array_index(plt->ydata, gdouble, trc+(j*satl));
		if (dt<mny) mny=dt;
		else if (dt>mxy) mxy=dt;
	}
	xf=g_array_index(plt->xdata, gdouble, trc+((sz4-1)*satl));
	gtk_plot_set_index(pt, nx);
	gtk_plot_linear_update_scale(plot1, xi, xf, mny, mxy);
	g_array_unref(nx);
}

void pltmv(GtkPlotLinear *plot, gpointer data)
{
	gchar *str;

	str=g_strdup_printf(_("x: %f, y: %f"), (plot->xps), (plot->yps));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	g_free(str);
}

void pltmvp(GtkPlotPolar *plot, gpointer data)
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
	GArray *nx, *sz, *x, *y;
	gchar s[10];
	gchar *str;
	gdouble mny, mxy, num, num2, num3, num4, num5, num6, num7, xf, xi;
	gdouble *ptr;
	gint j, jdim, jdimx, k, kdim, kdimx, l, sz4;
	GtkPlot *pt;
	GtkPlotLinear *plt;
	GtkPlotPolar *plt2;

	jdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	kdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(kind));
	jdimx=bsra->len;
	if (jdim>=jdimx)
	{
		{num=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isr)); num2=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isp)); num3=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tc)); num4=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tw)); num5=gtk_spin_button_get_value(GTK_SPIN_BUTTON(zw)); num6=gtk_spin_button_get_value(GTK_SPIN_BUTTON(bsr)); num7=gtk_spin_button_get_value(GTK_SPIN_BUTTON(bsp));}
		k=0;
		kdimx=(isra->len)/jdimx;
		while (k<(kdimx-1))
		{
			for (j=jdimx; j<=jdim; j++) {g_array_insert_val(isra, j+(k*jdim), num); g_array_insert_val(ispa, j+(k*jdim), num2); g_array_insert_val(tca, j+(k*jdim), num3); g_array_insert_val(twa, j+(k*jdim), num4);}
			k++;
		}
		for (j=jdimx; j<=jdim; j++) {g_array_append_val(isra, num); g_array_append_val(ispa, num2); g_array_append_val(tca, num3); g_array_append_val(twa, num4); g_array_append_val(zwa, num5); g_array_append_val(bsra, num6); g_array_append_val(bspa, num7);}
	}
	else
	{
		{g_signal_handler_block(G_OBJECT(isr), isr_id); g_signal_handler_block(G_OBJECT(isp), isp_id); g_signal_handler_block(G_OBJECT(tc), tc_id); g_signal_handler_block(G_OBJECT(tw), tw_id); g_signal_handler_block(G_OBJECT(zw), zw_id); g_signal_handler_block(G_OBJECT(bsr), bsr_id); g_signal_handler_block(G_OBJECT(bsp), bsp_id);}
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
		{g_signal_handler_unblock(G_OBJECT(isr), isr_id); g_signal_handler_unblock(G_OBJECT(isp), isp_id); g_signal_handler_unblock(G_OBJECT(tc), tc_id); g_signal_handler_unblock(G_OBJECT(tw), tw_id); g_signal_handler_unblock(G_OBJECT(zw), zw_id); g_signal_handler_unblock(G_OBJECT(bsr), bsr_id); g_signal_handler_unblock(G_OBJECT(bsp), bsp_id);}
	}
	if (((flags&PROC_TRS)!=0)&&((flagd&DISP_MIJ)==0))
	{
		pt=GTK_PLOT(plot2);
		sz4=g_array_index(pt->sizes, gint, 0);
		plt=GTK_PLOT_LINEAR(plot2);
		jdimx=(plt->ydata->len)/(2*sz4);
		if (jdim<jdimx)
		{
			g_object_get(G_OBJECT(plot2), "xmin", &num, "xmax", &num2, NULL);
			nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1);
			k=2*jdim*sz4;
			g_array_append_val(nx, k);
			num3=g_array_index(plt->ydata, gdouble, k);
			for (j=k+2; j<k+(2*sz4); j+=2)
			{
				num4=g_array_index(plt->ydata, gdouble, j);
				if (num4>num3) num3=num4;
			}
			gtk_plot_set_index(pt, nx);
			gtk_plot_linear_update_scale_pretty(plot2, num, num2, 0, num3);
		}
	}
	if ((flags&PROC_PRS)!=0)
	{
		if (((flags&PROC_BAT)!=0)&&((flagd&DISP_MRJ)==0))
		{
			pt=GTK_PLOT(plot3);
			jdimx=g_array_index(pt->stride, gint, 0)/kdimxf;/*resync jdimx in terms of plot rather than parameter arrays*/
			if (jdim<jdimx)
			{
				sz4=g_array_index(pt->sizes, gint, 0);
				if ((flagd&DISP_MRK)==0)/* single plot */
				{
					l=kdim+(jdim*kdimxf);
					nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1);
					g_array_append_val(nx, l);
					gtk_plot_set_index(pt, nx);
					g_array_unref(nx);
					if ((flags&PROC_POL)==0)
					{
						num=g_array_index(plt->xdata, gdouble, l);
						num3=g_array_index(plt->ydata, gdouble, l);
						{num2=num; num4=num3;}
						for (k=1; k<sz4; k++)
						{
							num5=g_array_index(plt->xdata, gdouble, k);
							if (num5>num2) num2=num5;
							else if (num5<num) num=num5;
							num5=g_array_index(plt->ydata, gdouble, l+k);
							if (num5>num4) num4=num5;
							else if (num5<num3) num3=num5;
						}
						gtk_plot_linear_update_scale_pretty(plot3, num, num2, num3, num4);
					}
					else
					{
						plt2=GTK_PLOT_POLAR(plot3);
						num=g_array_index(plt2->thdata, gdouble, l);
						num3=g_array_index(plt2->rdata, gdouble, l);
						{num2=num; num4=num3;}
						for (k=1; k<sz4; k++)
						{
							num5=g_array_index(plt2->thdata, gdouble, k);
							if (num5>num2) num2=num5;
							else if (num5<num) num=num5;
							num5=g_array_index(plt2->rdata, gdouble, l+k);
							if (num5>num4) num4=num5;
							else if (num5<num3) num3=num5;
						}
						gtk_plot_polar_update_scale_pretty(plot3, num3, num4, num, num2);
					}
				}
				else/* multiplot over k */
				{
					nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
					for (l=jdim*kdimxf; l<(jdim+1)*kdimxf; l++) g_array_append_val(nx, l);
					gtk_plot_set_index(pt, nx);
					g_array_unref(nx);
					if ((flags&PROC_POL)==0)
					{
						plt=GTK_PLOT_LINEAR(plot3);
						num=g_array_index(plt->xdata, gdouble, l);
						num3=g_array_index(plt->ydata, gdouble, l);
						{num2=num; num4=num3;}
						for (j=1; j<sz4; j++)
						{
							num5=g_array_index(plt->xdata, gdouble, j);
							if (num5>num2) num2=num5;
							else if (num5<num) num=num5;
							num5=g_array_index(plt->ydata, gdouble, j+l);
							if (num5>num4) num4=num5;
							else if (num5<num3) num3=num5;
						}
						k=1;
						while (k<kdimxf)
						{
							{l+=sz4; k++;}
							for (j=0; j<sz4; j++)
							{
								num5=g_array_index(plt->ydata, gdouble, j+l);
								if (num5>num4) num4=num5;
								else if (num5<num3) num3=num5;
							}
						}
						gtk_plot_linear_update_scale_pretty(plot3, num, num2, num3, num4);
					}
					else
					{
						plt2=GTK_PLOT_POLAR(plot3);
						num=g_array_index(plt2->thdata, gdouble, l);
						num3=g_array_index(plt2->rdata, gdouble, l);
						{num2=num; num4=num3;}
						for (j=1; j<sz4; j++)
						{
							num5=g_array_index(plt2->thdata, gdouble, j);
							if (num5>num2) num2=num5;
							else if (num5<num) num=num5;
							num5=g_array_index(plt2->rdata, gdouble, j+l);
							if (num5>num4) num4=num5;
							else if (num5<num3) num3=num5;
						}
						k=1;
						while (k<kdimxf)
						{
							{l+=sz4; k++;}
							for (j=0; j<sz4; j++)
							{
								num5=g_array_index(plt2->rdata, gdouble, j+l);
								if (num5>num4) num4=num5;
								else if (num5<num3) num3=num5;
							}
						}
						gtk_plot_polar_update_scale_pretty(plot3, num3, num4, num, num2);
					}
				}
			}
		}
		else
		{
			jdimx=(vis->len)/kdimxf;/*resync jdimx in terms of vis rather than parameter arrays*/
			if ((kdim<kdimxf)&&(jdim<jdimx))
			{
				num=g_array_index(vis, gdouble, (jdim+(kdim*jdimx)));
				g_snprintf(s, 7, "%f", num);
				gtk_label_set_text(GTK_LABEL(visl), s);
				num=g_array_index(doms, gdouble, (jdim+(kdim*jdimx)));
				g_snprintf(s, 9, "%f", num);
				gtk_label_set_text(GTK_LABEL(dsl), s);
				if ((flags&PROC_CHP)!=0)
				{
					num=g_array_index(chp, gdouble, (jdim+(kdim*jdimx)));
					g_snprintf(s, 8, "%f", num);
					gtk_label_set_text(GTK_LABEL(chil), s);
				}
			}
			else
			{
				gtk_label_set_text(GTK_LABEL(visl), "");
				gtk_label_set_text(GTK_LABEL(dsl), "");
				if ((flags&PROC_CHP)!=0) gtk_label_set_text(GTK_LABEL(chil), "");
			}
		}
	}
	if (widget==jind)
	{
		g_signal_handler_block(G_OBJECT(jind2), j2_id);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind2), jdim);
		g_signal_handler_unblock(G_OBJECT(jind2), j2_id);
	}
	else if (widget=jind2)
	{
		g_signal_handler_block(G_OBJECT(jind), j1_id);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind), jdim);
		g_signal_handler_unblock(G_OBJECT(jind), j1_id);
	}
}

void upk(GtkWidget *widget, gpointer data)
{
	/*
	 * Checks if k index spinner has increased to a new value and fills parameter array values accordingly
	 * updates front panel to display parameters for new j,k values
	 * If processing has been performed, updates the displayed value/plot
	 */
	GArray *nx;
	gchar s[10];
	gchar *str;
	gdouble num, num2, num3, num4, num5;
	gint j, jdim, jdimx, k, kdim, kdimx, l, st4, sz4;
	GtkPlot *pt;
	GtkPlotLinear *plt;
	GtkPlotPolar *plt2;

	kdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	jdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(jind2));
	jdimx=(bsra->len);
	if (jdimx==0) kdimx=0;
	kdimx=(isra->len)/jdimx;
	if (kdim>=kdimx)
	{
		{num=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isr)); num2=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isp)); num3=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tc)); num4=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tw));}
		for (j=0; j<jdimx*(kdim-kdimx+1); j++) {g_array_append_val(isra, num); g_array_append_val(ispa, num2); g_array_append_val(tca, num3); g_array_append_val(twa, num4);}
	}
	else
	{
		{g_signal_handler_block(G_OBJECT(isr), isr_id); g_signal_handler_block(G_OBJECT(isp), isp_id); g_signal_handler_block(G_OBJECT(tc), tc_id); g_signal_handler_block(G_OBJECT(tw), tw_id);}
		num=g_array_index(isra, gdouble, (jdim+(kdim*jdimx)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(isr), num);
		num=g_array_index(ispa, gdouble, (jdim+(kdim*jdimx)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(isp), num);
		num=g_array_index(tca, gdouble, (jdim+(kdim*jdimx)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(tc), num);
		num=g_array_index(twa, gdouble, (jdim+(kdim*jdimx)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(tw), num);
		{g_signal_handler_unblock(G_OBJECT(isr), isr_id); g_signal_handler_unblock(G_OBJECT(isp), isp_id); g_signal_handler_unblock(G_OBJECT(tc), tc_id); g_signal_handler_unblock(G_OBJECT(tw), tw_id);}
	}
	if ((flags&PROC_PRS)!=0)
	{
		if (kdim<kdimxf)
		{
			if ((flags&PROC_BAT)!=0)
			{
				if ((flagd&DISP_MRK)==0)
				{
					pt=GTK_PLOT(plot3);
					st4=g_array_index(pt->stride, gint, 0);
					sz4=g_array_index(pt->sizes, gint, 0);
					if ((flagd&DISP_MRJ)==0)/* single plot*/
					{
						nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1);
						l=kdim+(jdim*kdimxf);
						g_array_append_val(nx, l);
						gtk_plot_set_index(pt, nx);
						g_array_unref(nx);
						if ((flags&PROC_POL)==0)
						{
							plt=GTK_PLOT_LINEAR(plot3);
							num=g_array_index((plt->xdata), gdouble, l);
							num3=g_array_index((plt->ydata), gdouble, l);
							{num2=num; num4=num3;}
							for (k=1; k<sz4; k++)
							{
								l+=st4;
								num5=g_array_index((plt->xdata), gdouble, l);
								if (num5>num2) num2=num5;
								else if (num5<num) num=num5;
								num5=g_array_index((plt->ydata), gdouble, l);
								if (num5>num4) num4=num5;
								else if (num5<num3) num3=num5;
							}
							gtk_plot_linear_update_scale_pretty(plot3, num, num2, num3, num4);
						}
						else
						{
							plt2=GTK_PLOT_POLAR(plot3);
							num=g_array_index((plt2->thdata), gdouble, l);
							num3=g_array_index((plt2->rdata), gdouble, l);
							{num2=num; num4=num3;}
							for (k=1; k<sz4; k++)
							{
								l+=st4;
								num5=g_array_index((plt2->thdata), gdouble, l);
								if (num5>num2) num2=num5;
								else if (num5<num) num=num5;
								num5=g_array_index((plt2->rdata), gdouble, l);
								if (num5>num4) num4=num5;
								else if (num5<num3) num3=num5;
							}
							gtk_plot_polar_update_scale_pretty(plot3, num3, num4, num, num2);
						}
					}
					else/* multiplot over j */
					{
						jdimx=st4/kdimxf;/*resync jdimx in terms of the plot rather than parameter arrays*/
						nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx);
						for (l=kdim; l<st4; l+=kdimxf) g_array_append_val(nx, l);
						gtk_plot_set_index(pt, nx);
						g_array_unref(nx);
						l=kdim+st4;
						if ((flags&PROC_POL)==0)
						{
							plt=GTK_PLOT_LINEAR(plot3);
							l=kdim;
							num=g_array_index((plt->xdata), gdouble, kdim);
							num3=g_array_index((plt->ydata), gdouble, kdim);
							{num2=num; num4=num3;}
							for (k=1; k<sz4; k++)
							{
								num5=g_array_index((plt->xdata), gdouble, l);
								if (num5>num2) num2=num5;
								else if (num5<num) num=num5;
								num5=g_array_index((plt->ydata), gdouble, l);
								if (num5>num4) num4=num5;
								else if (num5<num3) num3=num5;
								{j=1; l+=kdimxf;}
								while (j<jdimx)
								{
									num5=g_array_index((plt->ydata), gdouble, l);
									if (num5>num4) num4=num5;
									else if (num5<num3) num3=num5;
									{l+=kdimxf; j++;}
								}
							}
							gtk_plot_linear_update_scale_pretty(plot3, num, num2, num3, num4);
						}
						else
						{
							plt2=GTK_PLOT_POLAR(plot3);
							num=g_array_index((plt2->thdata), gdouble, kdim);
							num3=g_array_index((plt2->rdata), gdouble, kdim);
							{num2=num; num4=num3;}
							for (k=1; k<sz4; k++)
							{
								num5=g_array_index((plt2->thdata), gdouble, l);
								if (num5>num2) num2=num5;
								else if (num5<num) num=num5;
								num5=g_array_index((plt2->rdata), gdouble, l);
								if (num5>num4) num4=num5;
								else if (num5<num3) num3=num5;
								{j=1; l+=kdimxf;}
								while (j<jdimx)
								{
									num5=g_array_index((plt2->rdata), gdouble, l);
									if (num5>num4) num4=num5;
									else if (num5<num3) num3=num5;
									{l+=kdimxf; j++;}
								}
							}
							gtk_plot_polar_update_scale_pretty(plot3, num3, num4, num, num2);
						}
					}
				}
			}
			else if ((jdim*kdimxf)<(vis->len))
			{
				num=g_array_index(vis, gdouble, (kdim+(jdim*kdimxf)));
				g_snprintf(s, 7, "%f", num);
				gtk_label_set_text(GTK_LABEL(visl), s);
				num=g_array_index(doms, gdouble, (kdim+(jdim*kdimxf)));
				g_snprintf(s, 9, "%f", num);
				gtk_label_set_text(GTK_LABEL(dsl), s);
				if ((flags&PROC_CHP)!=0)
				{
					num=g_array_index(chp, gdouble, (kdim+(jdim*kdimxf)));
					g_snprintf(s, 8, "%f", num);
					gtk_label_set_text(GTK_LABEL(chil), s);
				}
			}
			else
			{
				gtk_label_set_text(GTK_LABEL(visl), "");
				gtk_label_set_text(GTK_LABEL(dsl), "");
				if ((flags&PROC_CHP)!=0) gtk_label_set_text(GTK_LABEL(chil), "");
			}
		}
		else if ((flags&PROC_BAT)==0)
		{
			gtk_label_set_text(GTK_LABEL(visl), "");
			gtk_label_set_text(GTK_LABEL(dsl), "");
			if ((flags&PROC_CHP)!=0) gtk_label_set_text(GTK_LABEL(chil), "");
		}
	}
}

void upa2(GtkWidget *widget, gpointer data)
{
	gdouble *ptr;
	gint jdim, jdimx, kdim;

	kdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(kind));
	jdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(jind2));
	jdimx=(bsra->len);
	ptr=&g_array_index((GArray*)data, gdouble, jdim+(kdim*jdimx));
	*ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
}

void upa1(GtkWidget *widget, gpointer data)
{
	gdouble *ptr;
	gint jdim;

	jdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(jind));
	ptr=&g_array_index((GArray*)data, gdouble, jdim);
	*ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
}

void reset(GtkWidget *widget, gpointer data)
{
	gdouble *ptr;
	
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
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind2), 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(kind), 0);
	{(bsra->len)=1; (bspa->len)=1;}
	{(isra->len)=1; (ispa->len)=1; (tca->len)=1; (twa->len)=1; (zwa->len)=1;}
}

void reset2(GtkWidget *widget, gpointer data)
{/*update arrays to current values*/
	gdouble *ptr;
	gdouble num;
	gint jdim, jdimx;
	
	jdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(jind2));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(kind), 0);
	jdimx=bsra->len;
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

