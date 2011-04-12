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
	gtk_widget_show(helpwin);
	g_signal_connect_swapped(G_OBJECT(helpwin), "response", G_CALLBACK(gtk_widget_destroy), G_OBJECT(helpwin));
}

void upg(GtkWidget *widget, gpointer data)
{
	PlotLinear *plt;
	gdouble dt, xi, xf, mny, mxy;
	gint j, sz;
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
	sz=g_array_index((plt->sizes), gint, 0);/* adjust accordingly with multitrace considerations*/
	g_array_free(yb, TRUE);
	yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
	dt=g_array_index(specs, gdouble, trc-1);
	mny=dt;
	mxy=dt;
	g_array_append_val(yb, dt);
	for (j=1; j<sz; j++)
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
	guint j, k, sz2;
	gdouble num, num2, num3, num4, num5, num6, num7, xi, xf, mny, mxy;
	gdouble *ptr;
	gchar s[10];
	gchar *str;

	jdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	if (jdim>jdimx)
	{
		num=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isr));
		num2=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isp));
		num3=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tc));
		num4=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tw));
		num5=gtk_spin_button_get_value(GTK_SPIN_BUTTON(zw));
		num6=gtk_spin_button_get_value(GTK_SPIN_BUTTON(bsr));
		num7=gtk_spin_button_get_value(GTK_SPIN_BUTTON(bsp));
		k=0;
		while (k<kdimx)
		{
			for (j=(jdimx+1); j<=jdim; j++)
			{
				g_array_insert_val(isra, j+(k*jdim), num);
				g_array_insert_val(ispa, j+(k*jdim), num2);
				g_array_insert_val(tca, j+(k*jdim), num3);
				g_array_insert_val(twa, j+(k*jdim), num4);
			}
			k++;
		}
		for (j=(jdimx+1); j<=jdim; j++)
		{
			g_array_append_val(isra, num);
			g_array_append_val(ispa, num2);
			g_array_append_val(tca, num3);
			g_array_append_val(twa, num4);
			g_array_append_val(zwa, num5);
			g_array_append_val(bsra, num6);
			g_array_append_val(bspa, num7);
		}
		jdimx=jdim;
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
	if (jdim<=jdimxf)
	{
		if (((flags&2)!=0)&&((flagd&1)==0))
		{
			plt2=PLOT_LINEAR(plot2);
			sz2=g_array_index((plt2->sizes), gint, 0);
			g_array_free(xsb, TRUE);
			g_array_free(ysb, TRUE);
			xsb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), sz2);
			ysb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), sz2);
			for (j=0; j<sz2; j++)
			{
				num=j*g_array_index(delf, gdouble, jdim);
				g_array_append_val(xsb, num);
			}
			j*=(2*jdim);
			num2=fabs(g_array_index(stars, gdouble, j));
			g_array_append_val(ysb, num2);
			while (j<(((2*jdim)+1)*sz2))
			{
				num6=g_array_index(stars, gdouble, j);
				num6*=num6;
				num7=g_array_index(stars, gdouble, (2*sz2)-j);
				num7*=num7;
				num6+=num7;
				num6=sqrt(num6);
				if (num2<num6) num2=num6;
				g_array_append_val(ysb, num6);
				j++;
			}
			(plt2->xdata)=xsb;
			(plt2->ydata)=ysb;
			plot_linear_update_scale_pretty(plot2, 0, num, 0, num2);
		}
		if ((flags&4)!=0)
		{
			if ((flags&8)!=0)
			{
				if ((flagd&2)==0)
				{
					g_array_free(byr, TRUE);
					if ((flagd&4)==0)
					{
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
						if ((flagd&16)==0)
						{
							if ((flagd&8)==0)
							{
								num3=g_array_index(vis, gdouble, kdim+(jdim*kdimxf));
								g_array_append_val(byr, num3);
								num4=num3;
								for (j=1; j<mx; j++)
								{
									num=g_array_index(vis, gdouble, kdim+((jdim+(j*jdimxf))*kdimxf));
									g_array_append_val(byr, num);
									if (num>num4) num4=num;
									else if (num<num3) num3=num;
								}
							}
							else
							{
								num3=g_array_index(doms, gdouble, kdim+(jdim*kdimxf));
								g_array_append_val(byr, num3);
								num4=num3;
								for (j=1; j<mx; j++)
								{
									num=g_array_index(doms, gdouble, kdim+((jdim+(j*jdimxf))*kdimxf));
									g_array_append_val(byr, num);
									if (num>num4) num4=num;
									else if (num<num3) num3=num;
								}
							}
						}
						else
						{
							num3=g_array_index(chp, gdouble, kdim+(jdim*kdimxf));
							g_array_append_val(byr, num3);
							num4=num3;
							for (j=1; j<mx; j++)
							{
								num=g_array_index(chp, gdouble, kdim+((jdim+(j*jdimxf))*kdimxf));
								g_array_append_val(byr, num);
								if (num>num4) num4=num;
								else if (num<num3) num3=num;
							}
						}
					}
					else/* multiplot over k */
					{
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf);
						if ((flagd&16)==0)
						{
							if ((flagd&8)==0)
							{
								num3=g_array_index(vis, gdouble, (jdim*kdimxf));
								g_array_append_val(byr, num3);
								num4=num3;
								for (j=1; j<mx; j++)
								{
									num=g_array_index(vis, gdouble, ((jdim+(j*jdimxf))*kdimxf));
									g_array_append_val(byr, num);
									if (num>num4) num4=num;
									else if (num<num3) num3=num;
								}
								for (k=1; k<kdimxf; k++)
								{
									for (j=0; j<mx; j++)
									{
										num=g_array_index(vis, gdouble, k+((jdim+(j*jdimxf))*kdimxf));
										g_array_append_val(byr, num);
										if (num>num4) num4=num;
										else if (num<num3) num3=num;
									}
								}
							}
							else
							{
								num3=g_array_index(doms, gdouble, (jdim*kdimxf));
								g_array_append_val(byr, num3);
								num4=num3;
								for (j=1; j<mx; j++)
								{
									num=g_array_index(doms, gdouble, ((jdim+(j*jdimxf))*kdimxf));
									g_array_append_val(byr, num);
									if (num>num4) num4=num;
									else if (num<num3) num3=num;
								}
								for (k=1; k<kdimxf; k++)
								{
									for (j=0; j<mx; j++)
									{
										num=g_array_index(doms, gdouble, k+((jdim+(j*jdimxf))*kdimxf));
										g_array_append_val(byr, num);
										if (num>num4) num4=num;
										else if (num<num3) num3=num;
									}
								}
							}
						}
						else
						{
							num3=g_array_index(chp, gdouble, (jdim*kdimxf));
							g_array_append_val(byr, num3);
							num4=num3;
							for (j=1; j<mx; j++)
							{
								num=g_array_index(chp, gdouble, ((jdim+(j*jdimxf))*kdimxf));
								g_array_append_val(byr, num);
								if (num>num4) num4=num;
								else if (num<num3) num3=num;
							}
							for (k=1; k<kdimxf; k++)
							{
								for (j=0; j<mx; j++)
								{
									num=g_array_index(chp, gdouble, k+((jdim+(j*jdimxf))*kdimxf));
									g_array_append_val(byr, num);
									if (num>num4) num4=num;
									else if (num<num3) num3=num;
								}
							}
						}
					}
					if ((flags&32)==0)
					{
						plt3=PLOT_LINEAR(plot3);
						(plt3->ydata)=byr;
						g_object_get(G_OBJECT(plot3), "xmin", &num, "xmax", &num2, NULL);
						plot_linear_update_scale_pretty(plot3, num, num2, num3, num4);
					}
					else
					{
						plt4=PLOT_POLAR(plot3);
						(plt4->rdata)=byr;
						g_object_get(G_OBJECT(plot3), "thmin", &num, "thmax", &num2, NULL);
						plot_polar_update_scale_pretty(plot3, num3, num4, num, num2);
					}
				}
			}
			else if (kdim<=kdimxf)
			{
				num=g_array_index(vis, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 7, "%f", num);
				gtk_label_set_text(GTK_LABEL(visl), s);
				num=g_array_index(doms, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 9, "%f", num);
				gtk_label_set_text(GTK_LABEL(dsl), s);
				if ((flags&16)!=0)
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
				if ((flags&16)!=0) gtk_label_set_text(GTK_LABEL(chil), str);
				g_free(str);
			}
		}
	}
	else if (((flags&4)!=0)&&((flags&8)==0))
	{
		str=g_strdup("");
		gtk_label_set_text(GTK_LABEL(visl), str);
		gtk_label_set_text(GTK_LABEL(dsl), str);
		if ((flags&16)!=0) gtk_label_set_text(GTK_LABEL(chil), str);
		g_free(str);
	}
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
	guint j, k;
	gdouble num, num2, num3, num4;
	gchar s[10];
	gchar *str;

	kdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	if (kdim>kdimx)
	{
		num=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isr));
		num2=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isp));
		num3=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tc));
		num4=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tw));
		for (j=0; j<jdimx*(kdim-kdimx); j++)
		{
			g_array_append_val(isra, num);
			g_array_append_val(ispa, num2);
			g_array_append_val(tca, num3);
			g_array_append_val(twa, num4);
		}
		kdimx=kdim;
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
	if ((flags&4)!=0)
	{
		if (kdim<=kdimxf)
		{
			if ((flags&8)!=0)
			{
				if ((flagd&4)==0)
				{
					g_array_free(byr, TRUE);
					if ((flagd&2)==0)
					{
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
						if ((flagd&16)==0)
						{
							if ((flagd&8)==0)
							{
								num3=g_array_index(vis, gdouble, kdim+(jdim*kdimxf));
								g_array_append_val(byr, num3);
								num4=num3;
								for (j=1; j<mx; j++)
								{
									num=g_array_index(vis, gdouble, kdim+((jdim+(j*jdimxf))*kdimxf));
									g_array_append_val(byr, num);
									if (num>num4) num4=num;
									else if (num<num3) num3=num;
								}
							}
							else
							{
								num3=g_array_index(doms, gdouble, kdim+(jdim*kdimxf));
								g_array_append_val(byr, num3);
								num4=num3;
								for (j=1; j<mx; j++)
								{
									num=g_array_index(doms, gdouble, kdim+((jdim+(j*jdimxf))*kdimxf));
									g_array_append_val(byr, num);
									if (num>num4) num4=num;
									else if (num<num3) num3=num;
								}
							}
						}
						else
						{
							num3=g_array_index(chp, gdouble, kdim+(jdim*kdimxf));
							g_array_append_val(byr, num3);
							num4=num3;
							for (j=1; j<mx; j++)
							{
								num=g_array_index(chp, gdouble, kdim+((jdim+(j*jdimxf))*kdimxf));
								g_array_append_val(byr, num);
								if (num>num4) num4=num;
								else if (num<num3) num3=num;
							}
						}
					}
					else/* multiplot over j */
					{
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimxf);
						if ((flagd&16)==0)
						{
							if ((flagd&8)==0)
							{
								num3=g_array_index(vis, gdouble, kdim);
								g_array_append_val(byr, num3);
								num4=num3;
								for (j=1; j<mx; j++)
								{
									num=g_array_index(vis, gdouble, kdim+(j*jdimxf*kdimxf));
									g_array_append_val(byr, num);
									if (num>num4) num4=num;
									else if (num<num3) num3=num;
								}
								for (k=1; k<jdimxf; k++)
								{
									for (j=0; j<mx; j++)
									{
										num=g_array_index(vis, gdouble, kdim+((k+(j*jdimxf))*kdimxf));
										g_array_append_val(byr, num);
										if (num>num4) num4=num;
										else if (num<num3) num3=num;
									}
								}
							}
							else
							{
								num3=g_array_index(doms, gdouble, kdim);
								g_array_append_val(byr, num3);
								num4=num3;
								for (j=1; j<mx; j++)
								{
									num=g_array_index(doms, gdouble, kdim+(j*jdimxf*kdimxf));
									g_array_append_val(byr, num);
									if (num>num4) num4=num;
									else if (num<num3) num3=num;
								}
								for (k=1; k<jdimxf; k++)
								{
									for (j=0; j<mx; j++)
									{
										num=g_array_index(doms, gdouble, kdim+((k+(j*jdimxf))*kdimxf));
										g_array_append_val(byr, num);
										if (num>num4) num4=num;
										else if (num<num3) num3=num;
									}
								}
							}
						}
						else
						{
							num3=g_array_index(chp, gdouble, kdim);
							g_array_append_val(byr, num3);
							num4=num3;
							for (j=1; j<mx; j++)
							{
								num=g_array_index(chp, gdouble, kdim+(j*jdimxf*kdimxf));
								g_array_append_val(byr, num);
								if (num>num4) num4=num;
								else if (num<num3) num3=num;
							}
							for (k=1; k<jdimxf; k++)
							{
								for (j=0; j<mx; j++)
								{
									num=g_array_index(chp, gdouble, kdim+((k+(j*jdimxf))*kdimxf));
									g_array_append_val(byr, num);
									if (num>num4) num4=num;
									else if (num<num3) num3=num;
								}
							}
						}
					}
					if ((flags&32)==0)
					{
						plt3=PLOT_LINEAR(plot3);
						(plt3->ydata)=byr;
						g_object_get(G_OBJECT(plot3), "xmin", &num, "xmax", &num2, NULL);
						plot_linear_update_scale_pretty(plot3, num, num2, num3, num4);
					}
					else
					{
						plt4=PLOT_POLAR(plot3);
						(plt4->rdata)=byr;
						g_object_get(G_OBJECT(plot3), "thmin", &num, "thmax", &num2, NULL);
						plot_polar_update_scale_pretty(plot3, num3, num4, num, num2);
					}
				}
			}
			else if (jdim<=jdimxf)
			{
				num=g_array_index(vis, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 7, "%f", num);
				gtk_label_set_text(GTK_LABEL(visl), s);
				num=g_array_index(doms, gdouble, (jdim+(kdim*jdimxf)));
				g_snprintf(s, 9, "%f", num);
				gtk_label_set_text(GTK_LABEL(dsl), s);
				if ((flags&16)!=0)
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
				if ((flags&16)!=0) gtk_label_set_text(GTK_LABEL(chil), str);
				g_free(str);
			}
		}
		else if ((flags&8)==0)
		{
			str=g_strdup("");
			gtk_label_set_text(GTK_LABEL(visl), str);
			gtk_label_set_text(GTK_LABEL(dsl), str);
			if ((flags&16)!=0) gtk_label_set_text(GTK_LABEL(chil), str);
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
	{jdim=0; jdimx=0; kdim=0; kdimx=0;}
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind), 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(kind), 0);
	{(bsra->len)=1; (bspa->len)=1;}
	{(isra->len)=1; (ispa->len)=1; (tca->len)=1; (twa->len)=1; (zwa->len)=1;}
}

void reset2(GtkWidget *widget, gpointer data)
{
	{jdim=0; kdim=0; kdimx=0;}
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind2), 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(kind), 0);
	{(isra->len)=jdimx+1; (ispa->len)=jdimx+1; (tca->len)=jdimx+1; (twa->len)=jdimx+1;}
}
