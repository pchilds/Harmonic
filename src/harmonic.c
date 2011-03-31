/***************************************************************************
 *            harmonic.c
 *
 *  A GTK+ program for analysing data in the Fourier domain
 *  version 0.1.0
 *  Features:
 *            Multiple data format input
 *            Graphical display of data and transformed result
 *            Batch Processing
 *            Analysis for visibility, domain shifts and chirp
 *
 *  Sat Dec  4 17:18:14 2010
 *  Copyright  2011  Paul Childs
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

/*
 * TO DO:
 *
 * DPR: change to be nonmodal
 * DPR: handling of plot3 and vis/doms/chp selector
 * OPD: config writer (+overwrite confirm)
 * OPD: unix/windows/mac delimiter support
 * FFT: implement invert to 2pi/x routine
 * PRC: triangle optimisation
 * SAVE: case for batch processed data
 */

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <fftw3.h>
#include <math.h>
#include "plotlinear0-1-0.h"
#include "plotpolar0-1-0.h"

#define DZE 0.00001 /* divide by zero threshold */
#define NZE -0.00001 /* negative of this */
#define MXDM 15 /* one less than maximum number of windows */
#define MY_2PI 6.2831853071795864769252867665590057683943387987502
#define NMY_2PI -6.2831853071795864769252867665590057683943387987502
#define NMY_PI -3.1415926535897932384626433832795028841971693993751
#define LNTOT 0.23025850929940456840179914546843642076011014886288 /* (ln10)/10 */
#define DLMT "\n"

GtkWidget *window, *tr, *zpd, *pr, *tracmenu, *trac, *fst, *notebook, *notebook2, *plot1, *plot2, *plot3, *statusbar, *rest, *visl, *dsl, *chil;
GtkWidget *agosa, *agtl, *anosa, *sws, *dlm, *ncmp, *lcmp, *bat, *chi, *twopionx, *opttri, *trans, *dBs, *neg;
GtkWidget *bsr, *bsp, *isr, *isp, *tc, *tw, *zw, *jind, *jind2, *kind; /* widgets for windowing */
GArray *bsra, *bspa, *isra, *ispa, *tca, *twa, *zwa, *x, *specs, *yb, *stars, *xsb, *ysb, *sz, *nx, *delf, *vis, *doms, *chp, *msr, *bxr, *byr, *bsz, *bnx; /* arrays for windowing and data */
GSList *group2=NULL; /* list for various traces available */
gint lc, mx; /* number of data points and number of files in batch routine */
guint jdim=0, kdim=0, jdimx=0, kdimx=0, jdimxf=0, kdimxf=0, satl=0, trc=1, flags=0, flagd=0; /* array indices, #of traces, trace number, and current processing state and display flags */
gulong pr_id; /* id for disabling/enabling post-transform processing */

void static help(GtkWidget *widget, gpointer data)
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

void static about(GtkWidget *widget, gpointer data)
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

void static dpr(GtkWidget *widget, gpointer data)
{
	GtkWidget *helpwin, *content, *table, *vbox, *entry1, *entry2, *label, *spin1, *spin2, *hsp, *ck, *ck2, *ck3, *ck4;
	GtkAdjustment *adj;
	PlotLinear *plt;
	PlotPolar *plt2;
	gdouble xi, xf, mny, mxy, iv, clc, rcn, thc;
	gint dx, dx2, j, k;
	gchar *str;
	
	helpwin=gtk_dialog_new_with_buttons(_("Display Properties"), GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL, GTK_STOCK_APPLY, GTK_RESPONSE_APPLY, NULL);
	g_signal_connect_swapped(G_OBJECT(helpwin), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(helpwin));
	gtk_widget_show(helpwin);
	content=gtk_dialog_get_content_area(GTK_DIALOG(helpwin));
	vbox=gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	if ((flags&32)!=0)
	{
		table=gtk_table_new(4, 2, FALSE);
		gtk_widget_show(table);
		label=gtk_label_new(_("Text size:"));
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		label=gtk_label_new(_("Tick label size:"));
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		entry1=gtk_entry_new();
		entry2=gtk_entry_new();
		hsp=gtk_hseparator_new();
		gtk_widget_show(hsp);
		label=gtk_label_new(_("Radial axis text:"));
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		label=gtk_label_new(_("Azimuthal axis text:"));
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		plt2=PLOT_POLAR(plot3);
		str=g_strdup(plt2->rlab);
		gtk_entry_set_text(GTK_ENTRY(entry1), str);
		g_free(str);
		str=g_strdup(plt2->thlab);
		gtk_entry_set_text(GTK_ENTRY(entry2), str);
		g_free(str);
		adj=(GtkAdjustment *) gtk_adjustment_new((plt2->lfsize), 8, 64, 1.0, 5.0, 0.0);
		spin1=gtk_spin_button_new(adj, 0, 0);
		adj=(GtkAdjustment *) gtk_adjustment_new((plt2->afsize), 8, 64, 1.0, 5.0, 0.0);
		spin2=gtk_spin_button_new(adj, 0, 0);
		gtk_widget_show(entry1);
		gtk_widget_show(entry2);
		gtk_widget_show(spin1);
		gtk_widget_show(spin2);
		gtk_table_attach(GTK_TABLE(table), entry1, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_table_attach(GTK_TABLE(table), entry2, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_table_attach(GTK_TABLE(table), spin1, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_table_attach(GTK_TABLE(table), spin2, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), hsp, FALSE, FALSE, 2);
		ck4=gtk_combo_box_new_text();
		gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Visibility"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Domain Shift"));
		gtk_combo_box_set_active(GTK_COMBO_BOX(ck4), 1);
		if ((flags&16)!=0) gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Chirp"));
		gtk_widget_show(ck4);
		gtk_box_pack_start(GTK_BOX(vbox), ck4, FALSE, FALSE, 2);
		ck2=gtk_check_button_new_with_label(_("Multiple plots for Results over index j"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck2), (flagd&2));
		gtk_widget_show(ck2);
		gtk_box_pack_start(GTK_BOX(vbox), ck2, FALSE, FALSE, 2);
		ck3=gtk_check_button_new_with_label(_("Multiple plots for Results over index k"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck3), (flagd&4));
		gtk_widget_show(ck3);
		gtk_box_pack_start(GTK_BOX(vbox), ck3, FALSE, FALSE, 2);
		gtk_container_add(GTK_CONTAINER(content), vbox);
		if (gtk_dialog_run(GTK_DIALOG(helpwin))==GTK_RESPONSE_APPLY)
		{
			g_free(plt2->rlab);
			g_free(plt2->thlab);
			(plt2->rlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry1)));
			(plt2->thlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry2)));
			(plt2->lfsize)=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin1));
			(plt2->afsize)=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin2));
			g_object_get(G_OBJECT(plot3), "thmin", &xi, "thmax", &xf, "rmin", &mny, "rmax", &mxy, "rcnt", &rcn, "thcnt", &thc, NULL);
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2)))
			{
				if ((flagd&2)==0)/* expand arrays over j*/
				{
					flagd^=2;
					if ((flagd&4)==0)
					{
						if ((flagd&16)==0)
						{
							if ((flagd&8)==0)
							{
							}
							else
							{
							}
						}
						else
						{
						}
					}
					else if ((flagd&16)==0)
					{
						if ((flagd&8)==0)
						{
						}
						else
						{
						}
					}
					else
					{
					}
				}
			}
			else if ((flagd&2)!=0)/* shrink j variable of arrays */
			{
				flagd^=2;
				if ((flagd&4)==0)
				{
					if ((flagd&16)==0)
					{
						if ((flagd&8)==0)
						{
						}
						else
						{
						}
					}
					else
					{
					}
				}
				else if ((flagd&16)==0)
				{
					if ((flagd&8)==0)
					{
					}
					else
					{
					}
				}
				else
				{
				}
			}
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3)))
			{
				if ((flagd&4)==0)/* expand arrays over k*/
				{
					flagd^=4;
					if ((flagd&2)==0)
					{
						if ((flagd&16)==0)
						{
							if ((flagd&8)==0)
							{
							}
							else
							{
							}
						}
						else
						{
						}
					}
					else if ((flagd&16)==0)
					{
						if ((flagd&8)==0)
						{
						}
						else
						{
						}
					}
					else
					{
					}
				}
			}
			else if ((flagd&4)!=0)/* shrink k variable of arrays */
			{
				flagd^=4;
				if ((flagd&2)==0)
				{
					if ((flagd&16)==0)
					{
						if ((flagd&8)==0)
						{
						}
						else
						{
						}
					}
					else
					{
					}
				}
				else if ((flagd&16)==0)
				{
					if ((flagd&8)==0)
					{
					}
					else
					{
					}
				}
				else
				{
				}
			}
			plot_polar_update_scale(plot3, mny, mxy, xi, xf, rcn, thc);
		}
	}
	else if ((flags&8)!=0)
	{
		table=gtk_table_new(4, 2, FALSE);
		gtk_widget_show(table);
		label=gtk_label_new(_("Text size:"));
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		label=gtk_label_new(_("Tick label size:"));
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		entry1=gtk_entry_new();
		entry2=gtk_entry_new();
		hsp=gtk_hseparator_new();
		gtk_widget_show(hsp);
		label=gtk_label_new(_("X axis text:"));
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		label=gtk_label_new(_("Y axis text:"));
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		plt=PLOT_LINEAR(plot3);
		str=g_strdup(plt->xlab);
		gtk_entry_set_text(GTK_ENTRY(entry1), str);
		g_free(str);
		str=g_strdup(plt->ylab);
		gtk_entry_set_text(GTK_ENTRY(entry2), str);
		g_free(str);
		adj=(GtkAdjustment *) gtk_adjustment_new((plt->lfsize), 8, 64, 1.0, 5.0, 0.0);
		spin1=gtk_spin_button_new(adj, 0, 0);
		adj=(GtkAdjustment *) gtk_adjustment_new((plt->afsize), 8, 64, 1.0, 5.0, 0.0);
		spin2=gtk_spin_button_new(adj, 0, 0);
		gtk_widget_show(entry1);
		gtk_widget_show(entry2);
		gtk_widget_show(spin1);
		gtk_widget_show(spin2);
		gtk_table_attach(GTK_TABLE(table), entry1, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_table_attach(GTK_TABLE(table), entry2, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_table_attach(GTK_TABLE(table), spin1, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_table_attach(GTK_TABLE(table), spin2, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), hsp, FALSE, FALSE, 2);
		ck4=gtk_combo_box_new_text();
		gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Visibility"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Domain Shift"));
		gtk_combo_box_set_active(GTK_COMBO_BOX(ck4), 1);
		if ((flags&16)!=0) gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Chirp"));
		gtk_widget_show(ck4);
		gtk_box_pack_start(GTK_BOX(vbox), ck4, FALSE, FALSE, 2);
		ck2=gtk_check_button_new_with_label(_("Multiple plots for Results over index j"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck2), (flagd&2));
		gtk_widget_show(ck2);
		gtk_box_pack_start(GTK_BOX(vbox), ck2, FALSE, FALSE, 2);
		ck3=gtk_check_button_new_with_label(_("Multiple plots for Results over index k"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck3), (flagd&4));
		gtk_widget_show(ck3);
		gtk_box_pack_start(GTK_BOX(vbox), ck3, FALSE, FALSE, 2);
		gtk_container_add(GTK_CONTAINER(content), vbox);
		if (gtk_dialog_run(GTK_DIALOG(helpwin))==GTK_RESPONSE_APPLY)
		{
			g_free(plt->xlab);
			g_free(plt->ylab);
			(plt->xlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry1)));
			(plt->ylab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry2)));
			(plt->lfsize)=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin1));
			(plt->afsize)=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin2));
			g_object_get(G_OBJECT(plot3), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2)))
			{
				if ((flagd&2)==0)/* expand arrays over j*/
				{
					flagd^=2;
					if ((flagd&4)==0)
					{
						if ((flagd&16)==0)
						{
							if ((flagd&8)==0)
							{
							}
							else
							{
							}
						}
						else
						{
						}
					}
					else if ((flagd&16)==0)
					{
						if ((flagd&8)==0)
						{
						}
						else
						{
						}
					}
					else
					{
					}
				}
			}
			else if ((flagd&2)!=0)/* shrink j variable of arrays */
			{
				flagd^=2;
				if ((flagd&4)==0)
				{
					if ((flagd&16)==0)
					{
						if ((flagd&8)==0)
						{
						}
						else
						{
						}
					}
					else
					{
					}
				}
				else if ((flagd&16)==0)
				{
					if ((flagd&8)==0)
					{
					}
					else
					{
					}
				}
				else
				{
				}
			}
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3)))
			{
				if ((flagd&4)==0)/* expand arrays over k*/
				{
					flagd^=4;
					if ((flagd&2)==0)
					{
						if ((flagd&16)==0)
						{
							if ((flagd&8)==0)
							{
							}
							else
							{
							}
						}
						else
						{
						}
					}
					else if ((flagd&16)==0)
					{
						if ((flagd&8)==0)
						{
						}
						else
						{
						}
					}
					else
					{
					}
				}
			}
			else if ((flagd&4)!=0)/* shrink k variable of arrays */
			{
				flagd^=4;
				if ((flagd&2)==0)
				{
					if ((flagd&16)==0)
					{
						if ((flagd&8)==0)
						{
						}
						else
						{
						}
					}
					else
					{
					}
				}
				else if ((flagd&16)==0)
				{
					if ((flagd&8)==0)
					{
					}
					else
					{
					}
				}
				else
				{
				}
			}
			plot_linear_update_scale(plot3, xi, xf, mny, mxy);
		}
	}
	else
	{
		switch (gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook2)))
		{
			case 2:
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bat)))
			{
				ck4=gtk_combo_box_new_text();
				gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Visibility"));
				gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Domain Shift"));
				gtk_combo_box_set_active(GTK_COMBO_BOX(ck4), 1);
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(chi))) gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Chirp"));
				gtk_widget_show(ck4);
				gtk_box_pack_start(GTK_BOX(vbox), ck4, FALSE, FALSE, 2);
				ck2=gtk_check_button_new_with_label(_("Multiple plots for Results over index j"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck2), (flagd&2));
				gtk_widget_show(ck2);
				gtk_box_pack_start(GTK_BOX(vbox), ck2, FALSE, FALSE, 2);
				ck3=gtk_check_button_new_with_label(_("Multiple plots for Results over index k"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck3), (flagd&4));
				gtk_widget_show(ck3);
				gtk_box_pack_start(GTK_BOX(vbox), ck3, FALSE, FALSE, 2);
				gtk_container_add(GTK_CONTAINER(content), vbox);
				if (gtk_dialog_run(GTK_DIALOG(helpwin))==GTK_RESPONSE_APPLY)
				{
					flagd&=1;
					j=gtk_combo_box_get_active(GTK_COMBO_BOX(ck4));
					flagd|=j*8;
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2))) flagd|=2;
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3))) flagd|=4;
				}
			}
			gtk_widget_destroy(helpwin);
			break;
			case 1:
			table=gtk_table_new(4, 2, FALSE);
			gtk_widget_show(table);
			label=gtk_label_new(_("Text size:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			label=gtk_label_new(_("Tick label size:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			entry1=gtk_entry_new();
			entry2=gtk_entry_new();
			hsp=gtk_hseparator_new();
			gtk_widget_show(hsp);
			label=gtk_label_new(_("X axis text:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			label=gtk_label_new(_("Y axis text:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			plt=PLOT_LINEAR(plot2);
			str=g_strdup(plt->xlab);
			gtk_entry_set_text(GTK_ENTRY(entry1), str);
			g_free(str);
			str=g_strdup(plt->ylab);
			gtk_entry_set_text(GTK_ENTRY(entry2), str);
			g_free(str);
			adj=(GtkAdjustment *) gtk_adjustment_new((plt->lfsize), 8, 64, 1.0, 5.0, 0.0);
			spin1=gtk_spin_button_new(adj, 0, 0);
			adj=(GtkAdjustment *) gtk_adjustment_new((plt->afsize), 8, 64, 1.0, 5.0, 0.0);
			spin2=gtk_spin_button_new(adj, 0, 0);
			gtk_widget_show(entry1);
			gtk_widget_show(entry2);
			gtk_widget_show(spin1);
			gtk_widget_show(spin2);
			gtk_table_attach(GTK_TABLE(table), entry1, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_table_attach(GTK_TABLE(table), entry2, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_table_attach(GTK_TABLE(table), spin1, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_table_attach(GTK_TABLE(table), spin2, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(vbox), hsp, FALSE, FALSE, 2);
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bat)))
			{
				ck=gtk_check_button_new_with_label(_("Multiple plots for Inverse Domain"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck), (flagd&1));
				gtk_widget_show(ck);
				ck2=gtk_check_button_new_with_label(_("Multiple plots for Results over index j"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck2), (flagd&2));
				gtk_widget_show(ck2);
				gtk_box_pack_start(GTK_BOX(vbox), ck2, FALSE, FALSE, 2);
				ck3=gtk_check_button_new_with_label(_("Multiple plots for Results over index k"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck3), (flagd&4));
				gtk_widget_show(ck3);
				gtk_box_pack_start(GTK_BOX(vbox), ck3, FALSE, FALSE, 2);
				gtk_container_add(GTK_CONTAINER(content), vbox);
				if (gtk_dialog_run(GTK_DIALOG(helpwin))==GTK_RESPONSE_APPLY)
				{
					g_free(plt->xlab);
					g_free(plt->ylab);
					(plt->xlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry1)));
					(plt->ylab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry2)));
					(plt->lfsize)=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin1));
					(plt->afsize)=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin2));
					g_object_get(G_OBJECT(plot2), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck)))
					{
						if ((flagd&1)==0)
						{
							flagd^=1;
							if ((flags&2)!=0)/* merge into the one array */
							{
								dx=((plt->ydata)->len);
								dx2=0;
								g_array_free(ysb, TRUE);
								g_array_free(nx, TRUE);
								ysb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), dx*(jdimxf+1));
								nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), dx*(jdimxf+1));
								mxy=fabs(g_array_index(stars, gdouble, 0));
								g_array_append_val(ysb, mxy);
								for (j=1; j<dx; j++)
								{
									{iv=g_array_index(stars, gdouble, j); clc=g_array_index(stars, gdouble, (2*dx)-j);}
									{iv*=iv; clc*=clc;}
									iv=sqrt(iv+clc);
									if (mxy<iv) mxy=iv;
									g_array_append_val(ysb, iv);
								}
								g_array_append_val(nx, dx2);
								for (k=1; k<=jdimxf; k++)
								{
									xf=0;
									g_array_append_val(xsb, xf);
									iv=fabs(g_array_index(stars, gdouble, 2*k*dx));
									g_array_append_val(ysb, iv);
									if (mxy<iv) mxy=iv;
									for (j=1; j<dx; j++)
									{
										xf=j*g_array_index(delf, gdouble, 0);
										g_array_append_val(xsb, xf);
										{iv=g_array_index(stars, gdouble, j+(2*k*dx)); clc=g_array_index(stars, gdouble, (2*(k+1)*dx)-j);}
										{iv*=iv; clc*=clc;}
										iv=sqrt(iv+clc);
										if (mxy<iv) mxy=iv;
										g_array_append_val(ysb, iv);
									}
									g_array_append_val(sz, dx);
									dx2+=dx;
									g_array_append_val(nx, dx2);
								}
								(plt->sizes)=sz;
								(plt->ind)=nx;
								(plt->xdata)=xsb;
								(plt->ydata)=ysb;
							}
						}
					}
					else if ((flagd&1)!=0)
					{
						flagd^=1;
						if ((flags&2)!=0)/* narrow array to the one plot */
						{
							dx=g_array_index((plt->sizes), gint, 0);
							g_array_free(xsb, TRUE);
							g_array_free(ysb, TRUE);
							g_array_free(sz, TRUE);
							g_array_free(nx, TRUE);
							xsb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), dx);
							ysb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), dx);
							sz=g_array_new(FALSE, FALSE, sizeof(gint));
							nx=g_array_new(FALSE, FALSE, sizeof(gint));
							xf=0;
							g_array_append_val(xsb, xf);
							mxy=fabs(g_array_index(stars, gdouble, 2*jdim*dx));
							g_array_append_val(ysb, mxy);
							for (j=1; j<dx; j++)
							{
								xf=j*g_array_index(delf, gdouble, 0);
								g_array_append_val(xsb, xf);
								{iv=g_array_index(stars, gdouble, j+(2*jdim*dx)); clc=g_array_index(stars, gdouble, (2*(jdim+1)*dx)-j);}
								{iv*=iv; clc*=clc;}
								iv=sqrt(iv+clc);
								if (mxy<iv) mxy=iv;
								g_array_append_val(ysb, iv);
							}
							g_array_append_val(sz, dx);
							dx=0;
							g_array_append_val(nx, dx);
							(plt->sizes)=sz;
							(plt->ind)=nx;
							(plt->xdata)=xsb;
							(plt->ydata)=ysb;
						}
					}
					plot_linear_update_scale(plot2, xi, xf, mny, mxy);
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2))) flagd|=2;
					else if ((flagd&2)!=0) flagd^=2;
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3))) flagd|=4;
					else if ((flagd&4)!=0) flagd^=4;
				}
			}
			else
			{
				ck=gtk_check_button_new_with_label(_("Multiple plots for Inverse Domain"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck), (flagd&1));
				gtk_widget_show(ck);
				gtk_box_pack_start(GTK_BOX(vbox), ck, FALSE, FALSE, 2);
				gtk_container_add(GTK_CONTAINER(content), vbox);
				if (gtk_dialog_run(GTK_DIALOG(helpwin))==GTK_RESPONSE_APPLY)
				{
					g_free(plt->xlab);
					g_free(plt->ylab);
					(plt->xlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry1)));
					(plt->ylab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry2)));
					(plt->lfsize)=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin1));
					(plt->afsize)=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin2));
					g_object_get(G_OBJECT(plot2), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck)))
					{
						if ((flagd&1)==0)
						{
							flagd^=1;
							if ((flags&2)!=0)/* merge into the one array */
							{
								dx=((plt->ydata)->len);
								dx2=0;
								g_array_free(ysb, TRUE);
								g_array_free(nx, TRUE);
								ysb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), dx*(jdimxf+1));
								nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), dx*(jdimxf+1));
								mxy=fabs(g_array_index(stars, gdouble, 0));
								g_array_append_val(ysb, mxy);
								for (j=1; j<dx; j++)
								{
									{iv=g_array_index(stars, gdouble, j); clc=g_array_index(stars, gdouble, (2*dx)-j);}
									{iv*=iv; clc*=clc;}
									iv=sqrt(iv+clc);
									if (mxy<iv) mxy=iv;
									g_array_append_val(ysb, iv);
								}
								g_array_append_val(nx, dx2);
								for (k=1; k<=jdimxf; k++)
								{
									xf=0;
									g_array_append_val(xsb, xf);
									iv=fabs(g_array_index(stars, gdouble, 2*k*dx));
									g_array_append_val(ysb, iv);
									if (mxy<iv) mxy=iv;
									for (j=1; j<dx; j++)
									{
										xf=j*g_array_index(delf, gdouble, 0);
										g_array_append_val(xsb, xf);
										{iv=g_array_index(stars, gdouble, j+(2*k*dx)); clc=g_array_index(stars, gdouble, (2*(k+1)*dx)-j);}
										{iv*=iv; clc*=clc;}
										iv=sqrt(iv+clc);
										if (mxy<iv) mxy=iv;
										g_array_append_val(ysb, iv);
									}
									g_array_append_val(sz, dx);
									dx2+=dx;
									g_array_append_val(nx, dx2);
								}
								(plt->sizes)=sz;
								(plt->ind)=nx;
								(plt->xdata)=xsb;
								(plt->ydata)=ysb;
							}
						}
					}
					else if ((flagd&1)!=0)
					{
						flagd^=1;
						if ((flags&2)!=0)/* narrow array to the one plot */
						{
							dx=g_array_index((plt->sizes), gint, 0);
							g_array_free(xsb, TRUE);
							g_array_free(ysb, TRUE);
							g_array_free(sz, TRUE);
							g_array_free(nx, TRUE);
							xsb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), dx);
							ysb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), dx);
							sz=g_array_new(FALSE, FALSE, sizeof(gint));
							nx=g_array_new(FALSE, FALSE, sizeof(gint));
							xf=0;
							g_array_append_val(xsb, xf);
							mxy=fabs(g_array_index(stars, gdouble, 2*jdim*dx));
							g_array_append_val(ysb, mxy);
							for (j=1; j<dx; j++)
							{
								xf=j*g_array_index(delf, gdouble, 0);
								g_array_append_val(xsb, xf);
								{iv=g_array_index(stars, gdouble, j+(2*jdim*dx)); clc=g_array_index(stars, gdouble, (2*(jdim+1)*dx)-j);}
								{iv*=iv; clc*=clc;}
								iv=sqrt(iv+clc);
								if (mxy<iv) mxy=iv;
								g_array_append_val(ysb, iv);
							}
							g_array_append_val(sz, dx);
							dx=0;
							g_array_append_val(nx, dx);
							(plt->sizes)=sz;
							(plt->ind)=nx;
							(plt->xdata)=xsb;
							(plt->ydata)=ysb;
						}
					}
					plot_linear_update_scale(plot2, xi, xf, mny, mxy);
				}
			}
			gtk_widget_destroy(helpwin);
			break;
			default:
			table=gtk_table_new(4, 2, FALSE);
			gtk_widget_show(table);
			label=gtk_label_new(_("Text size:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			label=gtk_label_new(_("Tick label size:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			entry1=gtk_entry_new();
			entry2=gtk_entry_new();
			hsp=gtk_hseparator_new();
			gtk_widget_show(hsp);
			label=gtk_label_new(_("X axis text:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			label=gtk_label_new(_("Y axis text:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			plt=PLOT_LINEAR(plot1);
			str=g_strdup(plt->xlab);
			gtk_entry_set_text(GTK_ENTRY(entry1), str);
			g_free(str);
			str=g_strdup(plt->ylab);
			gtk_entry_set_text(GTK_ENTRY(entry2), str);
			g_free(str);
			adj=(GtkAdjustment *) gtk_adjustment_new((plt->lfsize), 8, 64, 1.0, 5.0, 0.0);
			spin1=gtk_spin_button_new(adj, 0, 0);
			adj=(GtkAdjustment *) gtk_adjustment_new((plt->afsize), 8, 64, 1.0, 5.0, 0.0);
			spin2=gtk_spin_button_new(adj, 0, 0);
			gtk_widget_show(entry1);
			gtk_widget_show(entry2);
			gtk_widget_show(spin1);
			gtk_widget_show(spin2);
			gtk_table_attach(GTK_TABLE(table), entry1, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_table_attach(GTK_TABLE(table), entry2, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_table_attach(GTK_TABLE(table), spin1, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_table_attach(GTK_TABLE(table), spin2, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(vbox), hsp, FALSE, FALSE, 2);
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bat)))
			{
				ck=gtk_check_button_new_with_label(_("Multiple plots for Inverse Domain"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck), (flagd&1));
				gtk_widget_show(ck);
				gtk_box_pack_start(GTK_BOX(vbox), ck, FALSE, FALSE, 2);
				ck2=gtk_check_button_new_with_label(_("Multiple plots for Results over index j"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck2), (flagd&2));
				gtk_widget_show(ck2);
				gtk_box_pack_start(GTK_BOX(vbox), ck2, FALSE, FALSE, 2);
				ck3=gtk_check_button_new_with_label(_("Multiple plots for Results over index k"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck3), (flagd&4));
				gtk_widget_show(ck3);
				gtk_box_pack_start(GTK_BOX(vbox), ck3, FALSE, FALSE, 2);
				gtk_container_add(GTK_CONTAINER(content), vbox);
				if (gtk_dialog_run(GTK_DIALOG(helpwin))==GTK_RESPONSE_APPLY)
				{
					g_free(plt->xlab);
					g_free(plt->ylab);
					(plt->xlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry1)));
					(plt->ylab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry2)));
					(plt->lfsize)=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin1));
					(plt->afsize)=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin2));
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck)))
					{
						if ((flagd&1)==0)
						{
							flagd^=1;
							if ((flags&2)!=0)/* merge into the one array */
							{
								plt=PLOT_LINEAR(plot2);
								dx=((plt->ydata)->len);
								dx2=0;
								g_array_free(ysb, TRUE);
								g_array_free(nx, TRUE);
								ysb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), dx*(jdimxf+1));
								nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), dx*(jdimxf+1));
								mxy=fabs(g_array_index(stars, gdouble, 0));
								g_array_append_val(ysb, mxy);
								for (j=1; j<dx; j++)
								{
									{iv=g_array_index(stars, gdouble, j); clc=g_array_index(stars, gdouble, (2*dx)-j);}
									{iv*=iv; clc*=clc;}
									iv=sqrt(iv+clc);
									if (mxy<iv) mxy=iv;
									g_array_append_val(ysb, iv);
								}
								g_array_append_val(nx, dx2);
								for (k=1; k<=jdimxf; k++)
								{
									xf=0;
									g_array_append_val(xsb, xf);
									iv=fabs(g_array_index(stars, gdouble, 2*k*dx));
									g_array_append_val(ysb, iv);
									if (mxy<iv) mxy=iv;
									for (j=1; j<dx; j++)
									{
										xf=j*g_array_index(delf, gdouble, 0);
										g_array_append_val(xsb, xf);
										{iv=g_array_index(stars, gdouble, j+(2*k*dx)); clc=g_array_index(stars, gdouble, (2*(k+1)*dx)-j);}
										{iv*=iv; clc*=clc;}
										iv=sqrt(iv+clc);
										if (mxy<iv) mxy=iv;
										g_array_append_val(ysb, iv);
									}
									g_array_append_val(sz, dx);
									dx2+=dx;
									g_array_append_val(nx, dx2);
								}
								(plt->sizes)=sz;
								(plt->ind)=nx;
								(plt->xdata)=xsb;
								(plt->ydata)=ysb;
								plot_linear_update_scale_pretty(plot2, 0, xf, 0, mxy);
							}
						}
					}
					else if ((flagd&1)!=0)/* narrow array to one plot */
					{
						flagd^=1;
						if ((flags&2)!=0)
						{
							plt=PLOT_LINEAR(plot2);
							dx=g_array_index((plt->sizes), gint, 0);
							g_array_free(xsb, TRUE);
							g_array_free(ysb, TRUE);
							g_array_free(sz, TRUE);
							g_array_free(nx, TRUE);
							xsb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), dx);
							ysb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), dx);
							sz=g_array_new(FALSE, FALSE, sizeof(gint));
							nx=g_array_new(FALSE, FALSE, sizeof(gint));
							xf=0;
							g_array_append_val(xsb, xf);
							mxy=fabs(g_array_index(stars, gdouble, 2*jdim*dx));
							g_array_append_val(ysb, mxy);
							for (j=1; j<dx; j++)
							{
								xf=j*g_array_index(delf, gdouble, 0);
								g_array_append_val(xsb, xf);
								{iv=g_array_index(stars, gdouble, j+(2*jdim*dx)); clc=g_array_index(stars, gdouble, (2*(jdim+1)*dx)-j);}
								{iv*=iv; clc*=clc;}
								iv=sqrt(iv+clc);
								if (mxy<iv) mxy=iv;
								g_array_append_val(ysb, iv);
							}
							g_array_append_val(sz, dx);
							dx=0;
							g_array_append_val(nx, dx);
							(plt->sizes)=sz;
							(plt->ind)=nx;
							(plt->xdata)=xsb;
							(plt->ydata)=ysb;
							plot_linear_update_scale_pretty(plot2, 0, xf, 0, mxy);
						}
					}
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2))) flagd|=2;
					else if ((flagd&2)!=0) flagd^=2;
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3))) flagd|=4;
					else if ((flagd&4)!=0) flagd^=4; 
					g_object_get(G_OBJECT(plot1), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
					plot_linear_update_scale(plot1, xi, xf, mny, mxy);
				}
			}
			else
			{
				ck=gtk_check_button_new_with_label(_("Multiple plots for Inverse Domain"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck), (flagd&1));
				gtk_widget_show(ck);
				gtk_box_pack_start(GTK_BOX(vbox), ck, FALSE, FALSE, 2);
				gtk_container_add(GTK_CONTAINER(content), vbox);
				if (gtk_dialog_run(GTK_DIALOG(helpwin))==GTK_RESPONSE_APPLY)
				{
					g_free(plt->xlab);
					g_free(plt->ylab);
					(plt->xlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry1)));
					(plt->ylab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry2)));
					(plt->lfsize)=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin1));
					(plt->afsize)=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin2));
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck)))
					{
						if ((flagd&1)==0)
						{
							flagd^=1;
							if ((flags&2)!=0)/* merge into the one array */
							{
								plt=PLOT_LINEAR(plot2);
								dx=((plt->ydata)->len);
								dx2=0;
								g_array_free(ysb, TRUE);
								g_array_free(nx, TRUE);
								ysb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), dx*(jdimxf+1));
								nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), dx*(jdimxf+1));
								mxy=fabs(g_array_index(stars, gdouble, 0));
								g_array_append_val(ysb, mxy);
								for (j=1; j<dx; j++)
								{
									{iv=g_array_index(stars, gdouble, j); clc=g_array_index(stars, gdouble, (2*dx)-j);}
									{iv*=iv; clc*=clc;}
									iv=sqrt(iv+clc);
									if (mxy<iv) mxy=iv;
									g_array_append_val(ysb, iv);
								}
								g_array_append_val(nx, dx2);
								for (k=1; k<=jdimxf; k++)
								{
									xf=0;
									g_array_append_val(xsb, xf);
									iv=fabs(g_array_index(stars, gdouble, 2*k*dx));
									g_array_append_val(ysb, iv);
									if (mxy<iv) mxy=iv;
									for (j=1; j<dx; j++)
									{
										xf=j*g_array_index(delf, gdouble, 0);
										g_array_append_val(xsb, xf);
										{iv=g_array_index(stars, gdouble, j+(2*k*dx)); clc=g_array_index(stars, gdouble, (2*(k+1)*dx)-j);}
										{iv*=iv; clc*=clc;}
										iv=sqrt(iv+clc);
										if (mxy<iv) mxy=iv;
										g_array_append_val(ysb, iv);
									}
									g_array_append_val(sz, dx);
									dx2+=dx;
									g_array_append_val(nx, dx2);
								}
								(plt->sizes)=sz;
								(plt->ind)=nx;
								(plt->xdata)=xsb;
								(plt->ydata)=ysb;
								plot_linear_update_scale_pretty(plot2, 0, xf, 0, mxy);
							}
						}
					}
					else if ((flagd&1)!=0)
					{
						flagd^=1;
						if ((flags&2)!=0)/* narrow array to just one plot */
						{
							plt=PLOT_LINEAR(plot2);
							dx=g_array_index((plt->sizes), gint, 0);
							g_array_free(xsb, TRUE);
							g_array_free(ysb, TRUE);
							g_array_free(sz, TRUE);
							g_array_free(nx, TRUE);
							xsb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), dx);
							ysb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), dx);
							sz=g_array_new(FALSE, FALSE, sizeof(gint));
							nx=g_array_new(FALSE, FALSE, sizeof(gint));
							xf=0;
							g_array_append_val(xsb, xf);
							mxy=fabs(g_array_index(stars, gdouble, 2*jdim*dx));
							g_array_append_val(ysb, mxy);
							for (j=1; j<dx; j++)
							{
								xf=j*g_array_index(delf, gdouble, 0);
								g_array_append_val(xsb, xf);
								{iv=g_array_index(stars, gdouble, j+(2*jdim*dx)); clc=g_array_index(stars, gdouble, (2*(jdim+1)*dx)-j);}
								{iv*=iv; clc*=clc;}
								iv=sqrt(iv+clc);
								if (mxy<iv) mxy=iv;
								g_array_append_val(ysb, iv);
							}
							g_array_append_val(sz, dx);
							dx=0;
							g_array_append_val(nx, dx);
							(plt->sizes)=sz;
							(plt->ind)=nx;
							(plt->xdata)=xsb;
							(plt->ydata)=ysb;
							plot_linear_update_scale_pretty(plot2, 0, xf, 0, mxy);
						}
					}
					g_object_get(G_OBJECT(plot1), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
					plot_linear_update_scale(plot1, xi, xf, mny, mxy);
				}
			}
			gtk_widget_destroy(helpwin);
			break;
		}
	}
}

void static prt(GtkWidget *widget, gpointer data)
{
	GtkWidget *wfile;
	GtkFileFilter *filter;
	gchar *str, *fout=NULL;

	switch (gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook2)))
	{
		case 2:
		if ((flags&12)==12)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			filter=gtk_file_filter_new();
			gtk_file_filter_set_name(filter, "Encapsulated Postscript (EPS)");
			gtk_file_filter_add_pattern(filter, "*.eps");
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), filter);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				if ((flags&32)!=0) plot_polar_print_eps(plot3, fout);
				else plot_linear_print_eps(plot3, fout);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&2)!=0)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			filter=gtk_file_filter_new();
			gtk_file_filter_set_name(filter, "Encapsulated Postscript (EPS)");
			gtk_file_filter_add_pattern(filter, "*.eps");
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), filter);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				plot_linear_print_eps(plot2, fout);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&1)!=0)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			filter=gtk_file_filter_new();
			gtk_file_filter_set_name(filter, "Encapsulated Postscript (EPS)");
			gtk_file_filter_add_pattern(filter, "*.eps");
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), filter);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				plot_linear_print_eps(plot1, fout);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 0);
			str=g_strdup(_("No available image."));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
		break;
		case 1:
		if ((flags&2)!=0)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			filter=gtk_file_filter_new();
			gtk_file_filter_set_name(filter, "Encapsulated Postscript (EPS)");
			gtk_file_filter_add_pattern(filter, "*.eps");
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), filter);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				plot_linear_print_eps(plot2, fout);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&1)!=0)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			filter=gtk_file_filter_new();
			gtk_file_filter_set_name(filter, "Encapsulated Postscript (EPS)");
			gtk_file_filter_add_pattern(filter, "*.eps");
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), filter);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				plot_linear_print_eps(plot1, fout);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 0);
			str=g_strdup(_("No available image."));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
		break;
		default:
		if ((flags&1)!=0)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			filter=gtk_file_filter_new();
			gtk_file_filter_set_name(filter, "Encapsulated Postscript (EPS)");
			gtk_file_filter_add_pattern(filter, "*.eps");
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), filter);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				plot_linear_print_eps(plot1, fout);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else
		{
			str=g_strdup(_("No available image."));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
		break;
	}
}

void static sav(GtkWidget *widget, gpointer data)
{
	GtkWidget *wfile, *dialog, *cont, *label;
	PlotLinear *plt;
	gchar *contents, *str, *str2, *fout=NULL;
	gchar s1[10], s2[10], s3[10];
	gint j, k, sz2;
	gdouble num, num2;
	GError *Err=NULL;

	switch (gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook2)))
	{
		case 2:
		if ((flags&28)==28)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Visibility"), 1, _("Domain Shift"), 2, _("Chirp"), 3, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 1:
					/*
					fill contents
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s"), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					 */
					break;
					case 2:
					/*
					fill contents
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s"), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					 */
					break;
					case 3:
					/*
					fill contents
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s"), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					*/
					break;
					default:
					break;
				}
				gtk_widget_destroy(dialog);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&12)==12)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Visibility"), 1, _("Domain Shift"), 2, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 1:
					/*
					fill contents
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s"), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					 */
					break;
					case 2:
					/*
					fill contents
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s"), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					 */
					break;
					default:
					break;
				}
				gtk_widget_destroy(dialog);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&2)!=0)
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 1);
			wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, "Real/Imaginary", 1, "Magnitude/Phase", 2, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 1:
					str2=g_strdup(_("INVERSE_D\tREAL_VAL \tIMAG_VAL "));
					contents=g_strdup(str2);
					for (k=1; k<=jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz2=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<=jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz2));
						str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
						g_free(str2);
						str2=g_strdup(str);
						g_free(str);
					}
					str=g_strdup(contents);
					g_free(contents);
					contents=g_strjoin(DLMT, str, str2, NULL);
					g_free(str);
					g_free(str2);
					for (j=1; j<sz2; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, j));
						g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*sz2)-j));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<=jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, (2*k*sz2)+j));
							g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*(++k)*sz2)-j));
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						g_free(str);
						g_free(str2);
					}
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s"), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					g_free(fout);
					break;
					case 2:
					str2=g_strdup(_("INVERSE_D\tMAGNITUDE\tPHASE    "));
					contents=g_strdup(str2);
					for (k=1; k<=jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz2=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<=jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz2));
						str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
						g_free(str2);
						str2=g_strdup(str);
						g_free(str);
					}
					str=g_strdup(contents);
					g_free(contents);
					contents=g_strjoin(DLMT, str, str2, NULL);
					g_free(str);
					g_free(str2);
					for (j=1; j<sz2; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						num=g_array_index(stars, gdouble, j);
						num2=g_array_index(stars, gdouble, (2*sz2)-j);
						g_snprintf(s3, 10, "%f", atan2(num2, num));
						num*=num;
						num2*=num2;
						num+=num2;
						num=sqrt(num);
						g_snprintf(s2, 10, "%f", num);
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<=jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							num=g_array_index(stars, gdouble, (2*k*sz2)+j);
							num2=g_array_index(stars, gdouble, (2*(++k)*sz2)-j);
							g_snprintf(s3, 10, "%f", atan2(num2, num));
							num*=num;
							num2*=num2;
							num+=num2;
							num=sqrt(num);
							g_snprintf(s2, 10, "%f", num);
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						g_free(str);
						g_free(str2);
					}
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s"), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					g_free(fout);
					break;
					default:
					break;
				}
			}
			gtk_widget_destroy(wfile);
		}
		else
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 0);
			str=g_strdup(_("No available processed data."));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
		break;
		case 1:
		if ((flags&2)!=0)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, "Real/Imaginary", 1, "Magnitude/Phase", 2, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 1:
					str2=g_strdup(_("INVERSE_D\tREAL_VAL \tIMAG_VAL "));
					contents=g_strdup(str2);
					for (k=1; k<=jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz2=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<=jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz2));
						str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
						g_free(str2);
						str2=g_strdup(str);
						g_free(str);
					}
					str=g_strdup(contents);
					g_free(contents);
					contents=g_strjoin(DLMT, str, str2, NULL);
					g_free(str);
					g_free(str2);
					for (j=1; j<sz2; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, j));
						g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*sz2)-j));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<=jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, (2*k*sz2)+j));
							g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*(++k)*sz2)-j));
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						g_free(str);
						g_free(str2);
					}
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s"), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					g_free(fout);
					break;
					case 2:
					str2=g_strdup(_("INVERSE_D\tMAGNITUDE\tPHASE    "));
					contents=g_strdup(str2);
					for (k=1; k<=jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz2=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<=jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz2));
						str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
						g_free(str2);
						str2=g_strdup(str);
						g_free(str);
					}
					str=g_strdup(contents);
					g_free(contents);
					contents=g_strjoin(DLMT, str, str2, NULL);
					g_free(str);
					g_free(str2);
					for (j=1; j<sz2; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						num=g_array_index(stars, gdouble, j);
						num2=g_array_index(stars, gdouble, (2*sz2)-j);
						g_snprintf(s3, 10, "%f", atan2(num2, num));
						num*=num;
						num2*=num2;
						num+=num2;
						num=sqrt(num);
						g_snprintf(s2, 10, "%f", num);
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<=jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							num=g_array_index(stars, gdouble, (2*k*sz2)+j);
							num2=g_array_index(stars, gdouble, (2*(++k)*sz2)-j);
							g_snprintf(s3, 10, "%f", atan2(num2, num));
							num*=num;
							num2*=num2;
							num+=num2;
							num=sqrt(num);
							g_snprintf(s2, 10, "%f", num);
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						g_free(str);
						g_free(str2);
					}
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s"), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					g_free(fout);
					break;
					default:
					break;
				}
			}
			gtk_widget_destroy(wfile);
		}
		else
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 0);
			str=g_strdup(_("No available processed data."));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
		break;
		default:
		if ((flags&2)!=0)
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 1);
			wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, "Real/Imaginary", 1, "Magnitude/Phase", 2, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 1:
					str2=g_strdup(_("INVERSE_D\tREAL_VAL \tIMAG_VAL "));
					contents=g_strdup(str2);
					for (k=1; k<=jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz2=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<=jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz2));
						str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
						g_free(str2);
						str2=g_strdup(str);
						g_free(str);
					}
					str=g_strdup(contents);
					g_free(contents);
					contents=g_strjoin(DLMT, str, str2, NULL);
					g_free(str);
					g_free(str2);
					for (j=1; j<sz2; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, j));
						g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*sz2)-j));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<=jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, (2*k*sz2)+j));
							g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*(++k)*sz2)-j));
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						g_free(str);
						g_free(str2);
					}
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s"), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					g_free(fout);
					break;
					case 2:
					str2=g_strdup(_("INVERSE_D\tMAGNITUDE\tPHASE    "));
					contents=g_strdup(str2);
					for (k=1; k<=jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz2=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<=jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz2));
						str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
						g_free(str2);
						str2=g_strdup(str);
						g_free(str);
					}
					str=g_strdup(contents);
					g_free(contents);
					contents=g_strjoin(DLMT, str, str2, NULL);
					g_free(str);
					g_free(str2);
					for (j=1; j<sz2; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						num=g_array_index(stars, gdouble, j);
						num2=g_array_index(stars, gdouble, (2*sz2)-j);
						g_snprintf(s3, 10, "%f", atan2(num2, num));
						num*=num;
						num2*=num2;
						num+=num2;
						num=sqrt(num);
						g_snprintf(s2, 10, "%f", num);
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<=jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							num=g_array_index(stars, gdouble, (2*k*sz2)+j);
							num2=g_array_index(stars, gdouble, (2*(++k)*sz2)-j);
							g_snprintf(s3, 10, "%f", atan2(num2, num));
							num*=num;
							num2*=num2;
							num+=num2;
							num=sqrt(num);
							g_snprintf(s2, 10, "%f", num);
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						g_free(str);
						g_free(str2);
					}
					g_file_set_contents(fout, contents, -1, &Err);
					g_free(contents);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s"), (gchar *) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					g_free(fout);
					break;
					default:
					break;
				}
			}
			gtk_widget_destroy(wfile);
		}
		else
		{
			str=g_strdup(_("No available processed data."));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
		break;
	}
}

void static prs(GtkWidget *widget, gpointer data)
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

void static trs(GtkWidget *widget, gpointer data) /* need to incorporate case for inversion to 2pi/x */
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
		ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
		zp=1<<(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(zpd)));
		n=zp*(jdimx+1);
		g_array_free(stars, TRUE);
		g_array_free(xsb, TRUE);
		g_array_free(ysb, TRUE);
		g_array_free(delf, TRUE);
		delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
		y=fftw_malloc(sizeof(double)*n);
		for (j=0; j<n; j++) y[j]=0;
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(twopionx)))
		{
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
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -Rlss- */
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
				else /* +Rlss- */
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
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -Rl0- */
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
			else /* +Rl0- */
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
		else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
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
		adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, MXDM, 1.0, 5.0, 0.0);
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

void static upg(GtkWidget *widget, gpointer data)
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

void static pltmv(PlotLinear *plot, gpointer data)
{
	gchar *str;

	str=g_strdup_printf(_("x: %f, y: %f"), (plot->xps), (plot->yps));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	g_free(str);
}

void static pltmvp(PlotPolar *plot, gpointer data)
{
	gchar *str;

	str=g_strdup_printf(_("x: %f, y: %f"), (plot->rps), (plot->thps));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	g_free(str);
}

void static opd(GtkWidget *widget, gpointer data)
{
	PlotLinear *plt;
	PlotPolar *plt2;
	GtkWidget *wfile, *dialog, *cont, *trace, *table, *label;
	gdouble xi, xf, lcl, mny, mxy, idelf, iv, vzt, vt, ivd, ivdt, tcn, twd, phi, phio, phia, dst, ddp, pn, cn, tp, ct, ofs, clc, dx2, xx;
	guint j, k, l, m, sal, st, sp, kib;
	gint n, zp, lcib, dr;
	gchar s[5];
	gchar *contents, *contents2, *str, *fin=NULL;
	gchar **strary, **strary2, **strat, **strat2;
	GSList *list;
	GError *Err=NULL;
	double *y, *star;
	fftw_plan p;
	fftw_r2r_kind type=FFTW_R2HC;

	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bat)))
	{
		if ((flags&4)==0)
		{
			str=g_strdup(_("Perform an initial test of the parameters first."));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
		else if ((jdimx!=jdimxf)||(kdimx!=kdimxf))
		{
			str=g_strdup(_("Parameters have changed since the last test. Retest with the current values first."));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
		else /* batch job processing mode */
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Config File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_NEW, GTK_RESPONSE_APPLY, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			dr=gtk_dialog_run(GTK_DIALOG(wfile));
			if (dr==GTK_RESPONSE_APPLY)
			{
				fin=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile)); /* overwrite confirmation? */
				/*
				
				 */
				dr=GTK_RESPONSE_ACCEPT;
			}
			if (dr!=GTK_RESPONSE_ACCEPT) gtk_widget_destroy(wfile);
			else
			{
				fin=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				gtk_widget_destroy(wfile);
				if (g_file_get_contents(fin, &contents2, NULL, &Err))
				{
					strary2=g_strsplit_set(contents2, "\r\n", 0);/*combine delimiters?*/
					mx=g_strv_length(strary2);
					zp=1<<(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(zpd)));
					n=zp*(jdimx+1);
					ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
					y=fftw_malloc(sizeof(double)*n);
					star=fftw_malloc(sizeof(double)*n);
					p=fftw_plan_many_r2r(1, &zp, (jdimx+1), y, NULL, 1, zp, star, NULL, 1, zp, &type, FFTW_ESTIMATE);
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(anosa))) {kib=2; lcib=-1;}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(sws))) {kib=0; lcib=-1;}
					else {kib=0; lcib=0;}
					g_array_free(vis, TRUE);
					g_array_free(doms, TRUE);
					g_array_free(msr, TRUE);
					g_array_free(bnx, TRUE);
					g_array_free(bsz, TRUE);
					vis=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx*kdimx*mx);
					doms=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx*kdimx*mx);
					msr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
					strat2=g_strsplit_set(strary2[0], "\t,", 0);
					xi=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
					xf=xi;
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(chi)))
					{
						flags|=16;
						g_array_free(chp, TRUE);
						chp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx*kdimx*mx);
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(twopionx)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
								{
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
									{
										if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss- */
										{
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													g_array_free(x, TRUE);
													g_array_free(yb, TRUE);
													g_array_free(delf, TRUE);
													g_array_free(stars, TRUE);
													x=g_array_new(FALSE, FALSE, sizeof(gdouble));
													yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
													lc=lcib;
													for (k=kib; k<sal; k++)
													{
														if (!strary[k]) continue;
														g_strchug(strary[k]);
														if (!g_strcmp0("", strary[k])) continue;
														if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
														if (lc<0) {lc++; continue;}
														strat=g_strsplit_set(strary[k], "\t,", 0);
														lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
														g_array_append_val(x, lcl);
														if (!strat[trc]) lcl=0;
														else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
														g_array_append_val(yb, lcl);
														g_strfreev(strat);
														lc++;
													}
													g_strfreev(strary);
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
													for (j=0; j<n; j++) y[j]=0;
													for (j=0; j<=jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<=jdimx; j++)
													{
														vzt=g_array_index(stars, gdouble, j*zp);
														iv=g_array_index(delf, gdouble, j);
														if (iv<DZE) idelf=G_MAXDOUBLE;
														else idelf=1/iv;
														iv=g_array_index(zwa, gdouble, j)*idelf/2;
														for (l=1; l<iv; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivd*=ivd;
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
															ivdt*=ivdt;
															vzt+=sqrt(ivd+ivdt);
														}
														if (vzt<DZE) vzt=G_MAXDOUBLE;
														else vzt=l/vzt;
														for (k=0; k<=kdimx; k++)
														{
															st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
															sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
															tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
															twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
															if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
															{
																vt=g_array_index(stars, gdouble, st+(j*zp));
																ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
																phia=atan2(ivdt, vt);
																vt*=vt;
																ivdt*=ivdt;
																vt=sqrt(vt+ivdt);
																ivd=g_array_index(stars, gdouble, st+1+(j*zp));
																ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
																phio=-atan2(ivdt, ivd);
																phia+=phio;
																ivd*=ivd;
																ivdt*=ivdt;
																vt+=sqrt(ivd+ivdt);
																{pn=0; cn=0; dst=0; ddp=0;}
																for (l=st+2; l<=sp; l++)
																{
																	ivd=g_array_index(stars, gdouble, l+(j*zp));
																	ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
															else {vt=0; dst=0; ddp=0;}
															g_array_append_val(vis, vt);
															g_array_append_val(doms, dst);
															g_array_append_val(chp, ddp);
														}
													}
												}
												else
												{
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
										else /* +TdBss- */
										{
											for (m=0; m<g_strv_length(strary2); m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													g_array_free(x, TRUE);
													g_array_free(yb, TRUE);
													g_array_free(delf, TRUE);
													g_array_free(stars, TRUE);
													x=g_array_new(FALSE, FALSE, sizeof(gdouble));
													yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
													lc=lcib;
													for (k=kib; k<sal; k++)
													{
														if (!strary[k]) continue;
														g_strchug(strary[k]);
														if (!g_strcmp0("", strary[k])) continue;
														if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
														if (lc<0) {lc++; continue;}
														strat=g_strsplit_set(strary[k], "\t,", 0);
														lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
														g_array_append_val(x, lcl);
														if (!strat[trc]) lcl=0;
														else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
														g_array_append_val(yb, lcl);
														g_strfreev(strat);
														lc++;
													}
													g_strfreev(strary);
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
													for (j=0; j<n; j++) y[j]=0;
													for (j=0; j<=jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<=jdimx; j++)
													{
														vzt=g_array_index(stars, gdouble, j*zp);
														iv=g_array_index(delf, gdouble, j);
														if (iv<DZE) idelf=G_MAXDOUBLE;
														else idelf=1/iv;
														iv=g_array_index(zwa, gdouble, j)*idelf/2;
														for (l=1; l<iv; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivd*=ivd;
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
															ivdt*=ivdt;
															ivd+=ivdt;
															ivd=sqrt(ivd);
															vzt+=ivd;
														}
														if (vzt<DZE) vzt=G_MAXDOUBLE;
														else vzt=l/vzt;
														for (k=0; k<=kdimx; k++)
														{
															st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
															sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
															tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
															twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
															if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
															{
																vt=g_array_index(stars, gdouble, st+(j*zp));
																ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
																phia=atan2(ivdt, vt);
																vt*=vt;
																ivdt*=ivdt;
																vt=sqrt(vt+ivdt);
																ivd=g_array_index(stars, gdouble, st+1+(j*zp));
																ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
																phio=-atan2(ivdt, ivd);
																phia+=phio;
																ivd*=ivd;
																ivdt*=ivdt;
																vt+=sqrt(ivd+ivdt);
																{pn=0; cn=0; dst=0; ddp=0;}
																for (l=st+2; l<=sp; l++)
																{
																	ivd=g_array_index(stars, gdouble, l+(j*zp));
																	ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
															else {vt=0; dst=0; ddp=0;}
															g_array_append_val(vis, vt);
															g_array_append_val(doms, dst);
															g_array_append_val(chp, ddp);
														}
													}
												}
												else
												{
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
									}
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss- */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												g_array_free(x, TRUE);
												g_array_free(yb, TRUE);
												g_array_free(delf, TRUE);
												g_array_free(stars, TRUE);
												x=g_array_new(FALSE, FALSE, sizeof(gdouble));
												yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
												lc=lcib;
												for (k=kib; k<sal; k++)
												{
													if (!strary[k]) continue;
													g_strchug(strary[k]);
													if (!g_strcmp0("", strary[k])) continue;
													if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
													if (lc<0) {lc++; continue;}
													strat=g_strsplit_set(strary[k], "\t,", 0);
													lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
													g_array_append_val(x, lcl);
													if (!strat[trc]) lcl=0;
													else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
													g_array_append_val(yb, lcl);
													g_strfreev(strat);
													lc++;
												}
												g_strfreev(strary);
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<=jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<=jdimx; j++)
												{
													vzt=g_array_index(stars, gdouble, j*zp);
													iv=g_array_index(delf, gdouble, j);
													if (iv<DZE) idelf=G_MAXDOUBLE;
													else idelf=1/iv;
													iv=g_array_index(zwa, gdouble, j)*idelf/2;
													for (l=1; l<iv; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivd*=ivd;
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
														ivdt*=ivdt;
														vzt+=sqrt(ivd+ivdt);
													}
													if (vzt<DZE) vzt=G_MAXDOUBLE;
													else vzt=l/vzt;
													for (k=0; k<=kdimx; k++)
													{
														st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
														sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
														tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
														twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
														if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
														{
															vt=g_array_index(stars, gdouble, st+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
															phia=atan2(ivdt, vt);
															vt*=vt;
															ivdt*=ivdt;
															vt=sqrt(vt+ivdt);
															ivd=g_array_index(stars, gdouble, st+1+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
															phio=-atan2(ivdt, ivd);
															phia+=phio;
															ivd*=ivd;
															ivdt*=ivdt;
															vt+=sqrt(ivd+ivdt);
															{pn=0; cn=0; dst=0; ddp=0;}
															for (l=st+2; l<=sp; l++)
															{
																ivd=g_array_index(stars, gdouble, l+(j*zp));
																ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
														else {vt=0; dst=0; ddp=0;}
														g_array_append_val(vis, vt);
														g_array_append_val(doms, dst);
														g_array_append_val(chp, ddp);
													}
												}
											}
											else
											{
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +RdBss- */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												g_array_free(x, TRUE);
												g_array_free(yb, TRUE);
												g_array_free(delf, TRUE);
												g_array_free(stars, TRUE);
												x=g_array_new(FALSE, FALSE, sizeof(gdouble));
												yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
												lc=lcib;
												for (k=kib; k<sal; k++)
												{
													if (!strary[k]) continue;
													g_strchug(strary[k]);
													if (!g_strcmp0("", strary[k])) continue;
													if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
													if (lc<0) {lc++; continue;}
													strat=g_strsplit_set(strary[k], "\t,", 0);
													lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
													g_array_append_val(x, lcl);
													if (!strat[trc]) lcl=0;
													else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
													g_array_append_val(yb, lcl);
													g_strfreev(strat);
													lc++;
												}
												g_strfreev(strary);
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<=jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<=jdimx; j++)
												{
													vzt=g_array_index(stars, gdouble, j*zp);
													iv=g_array_index(delf, gdouble, j);
													if (iv<DZE) idelf=G_MAXDOUBLE;
													else idelf=1/iv;
													iv=g_array_index(zwa, gdouble, j)*idelf/2;
													for (l=1; l<iv; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivd*=ivd;
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
														ivdt*=ivdt;
														vzt+=sqrt(ivd+ivdt);
													}
													if (vzt<DZE) vzt=G_MAXDOUBLE;
													else vzt=l/vzt;
													for (k=0; k<=kdimx; k++)
													{
														st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
														sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
														tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
														twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
														if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
														{
															vt=g_array_index(stars, gdouble, st+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
															phia=atan2(ivdt, vt);
															vt*=vt;
															ivdt*=ivdt;
															vt=sqrt(vt+ivdt);
															ivd=g_array_index(stars, gdouble, st+1+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
															phio=-atan2(ivdt, ivd);
															phia+=phio;
															ivd*=ivd;
															ivdt*=ivdt;
															vt+=sqrt(ivd+ivdt);
															{pn=0; cn=0; dst=0; ddp=0;}
															for (l=st+2; l<=sp; l++)
															{
																ivd=g_array_index(stars, gdouble, l+(j*zp));
																ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
														else {vt=0; dst=0; ddp=0;}
														g_array_append_val(vis, vt);
														g_array_append_val(doms, dst);
														g_array_append_val(chp, ddp);
													}
												}
											}
											else
											{
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss- +Tlss- */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phia=atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														ivd=g_array_index(stars, gdouble, st+1+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
														phio=-atan2(ivdt, ivd);
														phia+=phio;
														ivd*=ivd;
														ivdt*=ivdt;
														vt+=sqrt(ivd+ivdt);
														{pn=0; cn=0; dst=0; ddp=0;}
														for (l=st+2; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
													else {vt=0; dst=0; ddp=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
													g_array_append_val(chp, ddp);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* -Rlss- +Rlss- */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phia=atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														ivd=g_array_index(stars, gdouble, st+1+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
														phio=-atan2(ivdt, ivd);
														phia+=phio;
														ivd*=ivd;
														ivdt*=ivdt;
														vt+=sqrt(ivd+ivdt);
														{pn=0; cn=0; dst=0; ddp=0;}
														for (l=st+2; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
													else {vt=0; dst=0; ddp=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
													g_array_append_val(chp, ddp);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
								{
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0- */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												g_array_free(x, TRUE);
												g_array_free(yb, TRUE);
												g_array_free(delf, TRUE);
												g_array_free(stars, TRUE);
												x=g_array_new(FALSE, FALSE, sizeof(gdouble));
												yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
												lc=lcib;
												for (k=kib; k<sal; k++)
												{
													if (!strary[k]) continue;
													g_strchug(strary[k]);
													if (!g_strcmp0("", strary[k])) continue;
													if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
													if (lc<0) {lc++; continue;}
													strat=g_strsplit_set(strary[k], "\t,", 0);
													lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
													g_array_append_val(x, lcl);
													if (!strat[trc]) lcl=0;
													else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
													g_array_append_val(yb, lcl);
													g_strfreev(strat);
													lc++;
												}
												g_strfreev(strary);
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<=jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<=jdimx; j++)
												{
													vzt=g_array_index(stars, gdouble, j*zp);
													iv=g_array_index(delf, gdouble, j);
													if (iv<DZE) idelf=G_MAXDOUBLE;
													else idelf=1/iv;
													iv=g_array_index(zwa, gdouble, j)*idelf/2;
													for (l=1; l<iv; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivd*=ivd;
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
														ivdt*=ivdt;
														vzt+=sqrt(ivd+ivdt);
													}
													if (vzt<DZE) vzt=G_MAXDOUBLE;
													else vzt=l/vzt;
													for (k=0; k<=kdimx; k++)
													{
														st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
														sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
														tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
														twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
														if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
														{
															vt=g_array_index(stars, gdouble, st+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
															phia=atan2(ivdt, vt);
															vt*=vt;
															ivdt*=ivdt;
															vt=sqrt(vt+ivdt);
															ivd=g_array_index(stars, gdouble, st+1+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
															phio=-atan2(ivdt, ivd);
															phia+=phio;
															ivd*=ivd;
															ivdt*=ivdt;
															vt+=sqrt(ivd+ivdt);
															{pn=0; cn=0; dst=0; ddp=0;}
															for (l=st+2; l<=sp; l++)
															{
																ivd=g_array_index(stars, gdouble, l+(j*zp));
																ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
														else {vt=0; dst=0; ddp=0;}
														g_array_append_val(vis, vt);
														g_array_append_val(doms, dst);
														g_array_append_val(chp, ddp);
													}
												}
											}
											else
											{
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +TdB0- */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												g_array_free(x, TRUE);
												g_array_free(yb, TRUE);
												g_array_free(delf, TRUE);
												g_array_free(stars, TRUE);
												x=g_array_new(FALSE, FALSE, sizeof(gdouble));
												yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
												lc=lcib;
												for (k=kib; k<sal; k++)
												{
													if (!strary[k]) continue;
													g_strchug(strary[k]);
													if (!g_strcmp0("", strary[k])) continue;
													if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
													if (lc<0) {lc++; continue;}
													strat=g_strsplit_set(strary[k], "\t,", 0);
													lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
													g_array_append_val(x, lcl);
													if (!strat[1]) lcl=0;
													else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
													g_array_append_val(yb, lcl);
													g_strfreev(strat);
													lc++;
												}
												g_strfreev(strary);
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<=jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<=jdimx; j++)
												{
													vzt=g_array_index(stars, gdouble, j*zp);
													iv=g_array_index(delf, gdouble, j);
													if (iv<DZE) idelf=G_MAXDOUBLE;
													else idelf=1/iv;
													iv=g_array_index(zwa, gdouble, j)*idelf/2;
													for (l=1; l<iv; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivd*=ivd;
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
														ivdt*=ivdt;
														vzt+=sqrt(ivd+ivdt);
													}
													if (vzt<DZE) vzt=G_MAXDOUBLE;
													else vzt=l/vzt;
													for (k=0; k<=kdimx; k++)
													{
														st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
														sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
														tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
														twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
														if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
															{
															vt=g_array_index(stars, gdouble, st+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
															phia=atan2(ivdt, vt);
															vt*=vt;
															ivdt*=ivdt;
															vt=sqrt(vt+ivdt);
															ivd=g_array_index(stars, gdouble, st+1+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
															phio=-atan2(ivdt, ivd);
															phia+=phio;
															ivd*=ivd;
															ivdt*=ivdt;
															vt+=sqrt(ivd+ivdt);
															{pn=0; cn=0; dst=0; ddp=0;}
															for (l=st+2; l<=sp; l++)
															{
																ivd=g_array_index(stars, gdouble, l+(j*zp));
																ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
														else {vt=0; dst=0; ddp=0;}
														g_array_append_val(vis, vt);
														g_array_append_val(doms, dst);
														g_array_append_val(chp, ddp);
													}
												}
											}
											else
											{
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0- */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phia=atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														ivd=g_array_index(stars, gdouble, st+1+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
														phio=-atan2(ivdt, ivd);
														phia+=phio;
														ivd*=ivd;
														ivdt*=ivdt;
														vt+=sqrt(ivd+ivdt);
														{pn=0; cn=0; dst=0; ddp=0;}
														for (l=st+2; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
													else {vt=0; dst=0; ddp=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
													g_array_append_val(chp, ddp);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +RdB0- */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phia=atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														ivd=g_array_index(stars, gdouble, st+1+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
														phio=-atan2(ivdt, ivd);
														phia+=phio;
														ivd*=ivd;
														ivdt*=ivdt;
														vt+=sqrt(ivd+ivdt);
														{pn=0; cn=0; dst=0; ddp=0;}
														for (l=st+2; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
															ivd=sqrt(ivd+ivdt);
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
													else {vt=0; dst=0; ddp=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
													g_array_append_val(chp, ddp);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0- +Tl0- */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
										}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phia=atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													ivd=g_array_index(stars, gdouble, st+1+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
													phio=-atan2(ivdt, ivd);
													phia+=phio;
													ivd*=ivd;
													ivdt*=ivdt;
													ivd=sqrt(ivd+ivdt);
													{pn=0; cn=0; dst=0; ddp=0;}
													for (l=st+2; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0; ddp=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
												g_array_append_val(chp, ddp);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* -Rl0- +Rl0- */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
										}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phia=atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													ivd=g_array_index(stars, gdouble, st+1+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
													phio=-atan2(ivdt, ivd);
													phia+=phio;
													ivd*=ivd;
													ivdt*=ivdt;
													vt+=sqrt(ivd+ivdt);
													{pn=0; cn=0; dst=0; ddp=0;}
													for (l=st+2; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0; ddp=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
												g_array_append_val(chp, ddp);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
								{
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss+ */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												g_array_free(x, TRUE);
												g_array_free(yb, TRUE);
												g_array_free(delf, TRUE);
												g_array_free(stars, TRUE);
												x=g_array_new(FALSE, FALSE, sizeof(gdouble));
												yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
												lc=lcib;
												for (k=kib; k<sal; k++)
												{
													if (!strary[k]) continue;
													g_strchug(strary[k]);
													if (!g_strcmp0("", strary[k])) continue;
													if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
													if (lc<0) {lc++; continue;}
													strat=g_strsplit_set(strary[k], "\t,", 0);
													lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
													g_array_append_val(x, lcl);
													if (!strat[trc]) lcl=0;
													else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
													g_array_append_val(yb, lcl);
													g_strfreev(strat);
													lc++;
												}
												g_strfreev(strary);
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<=jdimx; j++)
												{
													iv=g_array_index(bsra, gdouble, j);
													k=0;
													while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
													st=k;
													iv=g_array_index(bspa, gdouble, j);
													while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
													sp=k-st;
													if (sp>zp) sp=zp;
													iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
													g_array_append_val(delf, iv);
													for (k=0; k<sp; k++)
													{
														clc=ofs-g_array_index(yb, gdouble, k+st);
														if (clc<0)
														{
															clc=-exp(LNTOT*clc);
															y[k+(j*zp)]=log(++clc);
														}
														else y[k+(j*zp)]=-G_MAXDOUBLE;
													}
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<=jdimx; j++)
												{
													vzt=g_array_index(stars, gdouble, j*zp);
													iv=g_array_index(delf, gdouble, j);
													if (iv<DZE) idelf=G_MAXDOUBLE;
													else idelf=1/iv;
													iv=g_array_index(zwa, gdouble, j)*idelf/2;
													for (l=1; l<iv; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivd*=ivd;
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
														ivdt*=ivdt;
														vzt+=sqrt(ivd+ivdt);
													}
													if (vzt<DZE) vzt=G_MAXDOUBLE;
													else vzt=l/vzt;
													for (k=0; k<=kdimx; k++)
													{
														st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
														sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
														tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
														twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
														if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
														{
															vt=g_array_index(stars, gdouble, st+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
															phia=atan2(ivdt, vt);
															vt*=vt;
															ivdt*=ivdt;
															vt=sqrt(vt+ivdt);
															ivd=g_array_index(stars, gdouble, st+1+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
															phio=-atan2(ivdt, ivd);
															phia+=phio;
															ivd*=ivd;
															ivdt*=ivdt;
															vt+=sqrt(ivd+ivdt);
															{pn=0; cn=0; dst=0; ddp=0;}
															for (l=st+2; l<=sp; l++)
															{
																ivd=g_array_index(stars, gdouble, l+(j*zp));
																ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
														else {vt=0; dst=0; ddp=0;}
														g_array_append_val(vis, vt);
														g_array_append_val(doms, dst);
														g_array_append_val(chp, ddp);
													}
												}
											}
											else
											{
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +TdBss+ */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												g_array_free(x, TRUE);
												g_array_free(yb, TRUE);
												g_array_free(delf, TRUE);
												g_array_free(stars, TRUE);
												x=g_array_new(FALSE, FALSE, sizeof(gdouble));
												yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
												lc=lcib;
												for (k=kib; k<sal; k++)
												{
													if (!strary[k]) continue;
													g_strchug(strary[k]);
													if (!g_strcmp0("", strary[k])) continue;
													if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
													if (lc<0) {lc++; continue;}
													strat=g_strsplit_set(strary[k], "\t,", 0);
													lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
													g_array_append_val(x, lcl);
													if (!strat[trc]) lcl=0;
													else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
													g_array_append_val(yb, lcl);
													g_strfreev(strat);
													lc++;
												}
												g_strfreev(strary);
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<=jdimx; j++)
												{
													iv=g_array_index(bsra, gdouble, j);
													k=0;
													while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
													st=k;
													iv=g_array_index(bspa, gdouble, j);
													while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
													sp=k-st;
													if (sp>zp) sp=zp;
													iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
													g_array_append_val(delf, iv);
													for (k=0; k<sp; k++)
													{
														clc=g_array_index(yb, gdouble, k+st)-ofs;
														if (clc<0)
														{
															clc=-exp(LNTOT*clc);
															y[k+(j*zp)]=log(++clc);
														}
														else y[k+(j*zp)]=-G_MAXDOUBLE;
													}
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<=jdimx; j++)
												{
													vzt=g_array_index(stars, gdouble, j*zp);
													iv=g_array_index(delf, gdouble, j);
													if (iv<DZE) idelf=G_MAXDOUBLE;
													else idelf=1/iv;
													iv=g_array_index(zwa, gdouble, j)*idelf/2;
													for (l=1; l<iv; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivd*=ivd;
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
														ivdt*=ivdt;
														vzt+=sqrt(ivd+ivdt);
													}
													if (vzt<DZE) vzt=G_MAXDOUBLE;
													else vzt=l/vzt;
													for (k=0; k<=kdimx; k++)
													{
														st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
														sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
														tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
														twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
														if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
														{
															vt=g_array_index(stars, gdouble, st+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
															phia=atan2(ivdt, vt);
															vt*=vt;
															ivdt*=ivdt;
															vt=sqrt(vt+ivdt);
															ivd=g_array_index(stars, gdouble, st+1+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
															phio=-atan2(ivdt, ivd);
															phia+=phio;
															ivd*=ivd;
															ivdt*=ivdt;
															vt+=sqrt(ivd+ivdt);
															{pn=0; cn=0; dst=0; ddp=0;}
															for (l=st+2; l<=sp; l++)
															{
																ivd=g_array_index(stars, gdouble, l+(j*zp));
																ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
														else {vt=0; dst=0; ddp=0;}
														g_array_append_val(vis, vt);
														g_array_append_val(doms, dst);
														g_array_append_val(chp, ddp);
													}
												}
											}
											else
											{
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss+ */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
												iv=g_array_index(bsra, gdouble, j);
												k=0;
												while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
												st=k;
												iv=g_array_index(bspa, gdouble, j);
												while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
												sp=k-st;
												if (sp>zp) sp=zp;
												iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
												g_array_append_val(delf, iv);
												for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(ofs-g_array_index(yb, gdouble, k+st));
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phia=atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														ivd=g_array_index(stars, gdouble, st+1+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
														phio=-atan2(ivdt, ivd);
														phia+=phio;
														ivd*=ivd;
														ivdt*=ivdt;
														vt+=sqrt(ivd+ivdt);
														{pn=0; cn=0; dst=0; ddp=0;}
														for (l=st+2; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
													else {vt=0; dst=0; ddp=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
													g_array_append_val(chp, ddp);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +RdBss+ */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
												iv=g_array_index(bsra, gdouble, j);
												k=0;
												while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
												st=k;
												iv=g_array_index(bspa, gdouble, j);
												while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
												sp=k-st;
												if (sp>zp) sp=zp;
												iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
												g_array_append_val(delf, iv);
												for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(g_array_index(yb, gdouble, k+st)-ofs);
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phia=atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														ivd=g_array_index(stars, gdouble, st+1+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
														phio=-atan2(ivdt, ivd);
														phia+=phio;
														ivd*=ivd;
														ivdt*=ivdt;
														vt+=sqrt(ivd+ivdt);
														{pn=0; cn=0; dst=0; ddp=0;}
														for (l=st+2; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
													else {vt=0; dst=0; ddp=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
													g_array_append_val(chp, ddp);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
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
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
												iv=g_array_index(bsra, gdouble, j);
												k=0;
												while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
												st=k;
												iv=g_array_index(bspa, gdouble, j);
												while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
												sp=k-st;
												if (sp>zp) sp=zp;
												iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
												g_array_append_val(delf, iv);
												for (k=0; k<sp; k++)
												{
													clc=-g_array_index(yb, gdouble, k+st)/ofs;
													clc++;
													if (clc>0) y[k+(j*zp)]=log(clc);
													else y[k+(j*zp)]=-G_MAXDOUBLE;
												}
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phia=atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														ivd=g_array_index(stars, gdouble, st+1+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
														phio=-atan2(ivdt, ivd);
														phia+=phio;
														ivd*=ivd;
														ivdt*=ivdt;
														vt+=sqrt(ivd+ivdt);
														{pn=0; cn=0; dst=0; ddp=0;}
														for (l=st+2; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
													else {vt=0; dst=0; ddp=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
													g_array_append_val(chp, ddp);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
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
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
												iv=g_array_index(bsra, gdouble, j);
												k=0;
												while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
												st=k;
												iv=g_array_index(bspa, gdouble, j);
												while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
												sp=k-st;
												if (sp>zp) sp=zp;
												iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
												g_array_append_val(delf, iv);
												for (k=0; k<sp; k++)
												{
													clc=g_array_index(yb, gdouble, k+st)/ofs;
													if (clc>0) y[k+(j*zp)]=log(clc);
													else y[k+(j*zp)]=-G_MAXDOUBLE;
												}
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phia=atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														ivd=g_array_index(stars, gdouble, st+1+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
														phio=-atan2(ivdt, ivd);
														phia+=phio;
														ivd*=ivd;
														ivdt*=ivdt;
														vt+=sqrt(ivd+ivdt);
														{pn=0; cn=0; dst=0; ddp=0;}
														for (l=st+2; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
													else {vt=0; dst=0; ddp=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
													g_array_append_val(chp, ddp);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
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
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
												iv=g_array_index(bsra, gdouble, j);
												k=0;
												while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
												st=k;
												iv=g_array_index(bspa, gdouble, j);
												while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
												sp=k-st;
												if (sp>zp) sp=zp;
												iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
												g_array_append_val(delf, iv);
												for (k=0; k<sp; k++)
												{
													clc=ofs-g_array_index(yb, gdouble, k+st);
													clc=-exp(LNTOT*clc);
													y[k+(j*zp)]=++clc;
												}
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phia=atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														ivd=g_array_index(stars, gdouble, st+1+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
														phio=-atan2(ivdt, ivd);
														phia+=phio;
														ivd*=ivd;
														ivdt*=ivdt;
														vt+=sqrt(ivd+ivdt);
														{pn=0; cn=0; dst=0; ddp=0;}
														for (l=st+2; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
															ivd+=ivdt;
															ivd=sqrt(ivd);
															vt+=ivd;
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
													else {vt=0; dst=0; ddp=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
													g_array_append_val(chp, ddp);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +TdB0+ */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
												iv=g_array_index(bsra, gdouble, j);
												k=0;
												while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
												st=k;
												iv=g_array_index(bspa, gdouble, j);
												while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
												sp=k-st;
												if (sp>zp) sp=zp;
												iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
												g_array_append_val(delf, iv);
												for (k=0; k<sp; k++)
												{
													clc=g_array_index(yb, gdouble, k+st)-ofs;
													clc=-exp(LNTOT*clc);
													y[k+(j*zp)]=++clc;
												}
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phia=atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														ivd=g_array_index(stars, gdouble, st+1+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
														phio=-atan2(ivdt, ivd);
														phia+=phio;
														ivd*=ivd;
														ivdt*=ivdt;
														vt+=sqrt(ivd+ivdt);
														{pn=0; cn=0; dst=0; ddp=0;}
														for (l=st+2; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
													else {vt=0; dst=0; ddp=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
													g_array_append_val(chp, ddp);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0+ */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
										}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
											iv=g_array_index(bsra, gdouble, j);
											k=0;
											while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
											st=k;
											iv=g_array_index(bspa, gdouble, j);
											while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
											sp=k-st;
											if (sp>zp) sp=zp;
											iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
											g_array_append_val(delf, iv);
											for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(ofs-g_array_index(yb, gdouble, k+st)));
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phia=atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													ivd=g_array_index(stars, gdouble, st+1+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
													phio=-atan2(ivdt, ivd);
													phia+=phio;
													ivd*=ivd;
													ivdt*=ivdt;
													vt+=sqrt(ivd+ivdt);
													{pn=0; cn=0; dst=0; ddp=0;}
													for (l=st+2; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0; ddp=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
												g_array_append_val(chp, ddp);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* +RdB0+ */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
										}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
											iv=g_array_index(bsra, gdouble, j);
											k=0;
											while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
											st=k;
											iv=g_array_index(bspa, gdouble, j);
											while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
											sp=k-st;
											if (sp>zp) sp=zp;
											iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
											g_array_append_val(delf, iv);
											for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(g_array_index(yb, gdouble, k+st)-ofs));
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phia=atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													ivd=g_array_index(stars, gdouble, st+1+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
													phio=-atan2(ivdt, ivd);
													phia+=phio;
													ivd*=ivd;
													ivdt*=ivdt;
													vt+=sqrt(ivd+ivdt);
													{pn=0; cn=0; dst=0; ddp=0;}
													for (l=st+2; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0; ddp=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
												g_array_append_val(chp, ddp);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0+ +Tl0+ */
						{
							if ((ofs<DZE)&&(ofs>NZE))
							{
								str=g_strdup(_("Offset must be nonzero for linear measurements."));
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
							}
							else
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
										}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
											iv=g_array_index(bsra, gdouble, j);
											k=0;
											while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
											st=k;
											iv=g_array_index(bspa, gdouble, j);
											while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
											sp=k-st;
											if (sp>zp) sp=zp;
											iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
											g_array_append_val(delf, iv);
											for (k=0; k<sp; k++)
											{
												clc=-g_array_index(yb, gdouble, k+st)/ofs;
												y[k+(j*zp)]=++clc;
											}
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phia=atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													ivd=g_array_index(stars, gdouble, st+1+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
													phio=-atan2(ivdt, ivd);
													phia+=phio;
													ivd*=ivd;
													ivdt*=ivdt;
													vt+=sqrt(ivd+ivdt);
													{pn=0; cn=0; dst=0; ddp=0;}
													for (l=st+2; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0; ddp=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
												g_array_append_val(chp, ddp);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
						}
						else /* -Rl0+ +Rl0+ */
						{
							if ((ofs<DZE)&&(ofs>NZE))
							{
								str=g_strdup(_("Offset must be nonzero for linear measurements."));
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
							}
							else
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
										}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
											iv=g_array_index(bsra, gdouble, j);
											k=0;
											while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
											st=k;
											iv=g_array_index(bspa, gdouble, j);
											while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
											sp=k-st;
											if (sp>zp) sp=zp;
											iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
											g_array_append_val(delf, iv);
											for (k=0; k<sp; k++) y[k+(j*zp)]=g_array_index(yb, gdouble, k+st)/ofs;
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp-1)/2))&&(sp<(zp/2))&&((sp-st)>1))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phia=atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													ivd=g_array_index(stars, gdouble, st+1+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st-1);
													phio=-atan2(ivdt, ivd);
													phia+=phio;
													ivd*=ivd;
													ivdt*=ivdt;
													vt+=sqrt(ivd+ivdt);
													{pn=0; cn=0; dst=0; ddp=0;}
													for (l=st+2; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0; ddp=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
												g_array_append_val(chp, ddp);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(twopionx)))
					{
						flags&=47;
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
								{
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss- */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												g_array_free(x, TRUE);
												g_array_free(yb, TRUE);
												g_array_free(delf, TRUE);
												g_array_free(stars, TRUE);
												x=g_array_new(FALSE, FALSE, sizeof(gdouble));
												yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
												lc=lcib;
												for (k=kib; k<sal; k++)
												{
													if (!strary[k]) continue;
													g_strchug(strary[k]);
													if (!g_strcmp0("", strary[k])) continue;
													if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
													if (lc<0) {lc++; continue;}
													strat=g_strsplit_set(strary[k], "\t,", 0);
													lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
													g_array_append_val(x, lcl);
													if (!strat[trc]) lcl=0;
													else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
													g_array_append_val(yb, lcl);
													g_strfreev(strat);
													lc++;
												}
												g_strfreev(strary);
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<=jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<=jdimx; j++)
												{
													vzt=g_array_index(stars, gdouble, j*zp);
													iv=g_array_index(delf, gdouble, j);
													if (iv<DZE) idelf=G_MAXDOUBLE;
													else idelf=1/iv;
													iv=g_array_index(zwa, gdouble, j)*idelf/2;
													for (l=1; l<iv; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivd*=ivd;
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
														ivdt*=ivdt;
														vzt+=sqrt(ivd+ivdt);
													}
													if (vzt<DZE) vzt=G_MAXDOUBLE;
													else vzt=l/vzt;
													for (k=0; k<=kdimx; k++)
													{
														st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
														sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
														tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
														twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
														if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
														{
															vt=g_array_index(stars, gdouble, st+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
															phio=-atan2(ivdt, vt);
															vt*=vt;
															ivdt*=ivdt;
															vt=sqrt(vt+ivdt);
															{dst=0; pn=0;}
															for (l=st+1; l<=sp; l++)
															{
																ivd=g_array_index(stars, gdouble, l+(j*zp));
																ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
														else {vt=0; dst=0;}
														g_array_append_val(vis, vt);
														g_array_append_val(doms, dst);
													}
												}
											}
											else
											{
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +TdBss- */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												g_array_free(x, TRUE);
												g_array_free(yb, TRUE);
												g_array_free(delf, TRUE);
												g_array_free(stars, TRUE);
												x=g_array_new(FALSE, FALSE, sizeof(gdouble));
												yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
												lc=lcib;
												for (k=kib; k<sal; k++)
												{
													if (!strary[k]) continue;
													g_strchug(strary[k]);
													if (!g_strcmp0("", strary[k])) continue;
													if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
													if (lc<0) {lc++; continue;}
													strat=g_strsplit_set(strary[k], "\t,", 0);
													lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
													g_array_append_val(x, lcl);
													if (!strat[trc]) lcl=0;
													else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
													g_array_append_val(yb, lcl);
													g_strfreev(strat);
													lc++;
												}
												g_strfreev(strary);
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<=jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<=jdimx; j++)
												{
													vzt=g_array_index(stars, gdouble, j*zp);
													iv=g_array_index(delf, gdouble, j);
													if (iv<DZE) idelf=G_MAXDOUBLE;
													else idelf=1/iv;
													iv=g_array_index(zwa, gdouble, j)*idelf/2;
													for (l=1; l<iv; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivd*=ivd;
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
														ivdt*=ivdt;
														vzt+=sqrt(ivd+ivdt);
													}
													if (vzt<DZE) vzt=G_MAXDOUBLE;
													else vzt=l/vzt;
													for (k=0; k<=kdimx; k++)
													{
														st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
														sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
														tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
														twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
														if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
														{
															vt=g_array_index(stars, gdouble, st+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
															phio=-atan2(ivdt, vt);
															vt*=vt;
															ivdt*=ivdt;
															vt=sqrt(vt+ivdt);
															{dst=0; pn=0;}
															for (l=st+1; l<=sp; l++)
															{
																ivd=g_array_index(stars, gdouble, l+(j*zp));
																ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
														else {vt=0; dst=0;}
														g_array_append_val(vis, vt);
														g_array_append_val(doms, dst);
													}
												}
											}
											else
											{
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss- */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phio=-atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														{dst=0; pn=0;}
														for (l=st+1; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
													else {vt=0; dst=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +RdBss- */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phio=-atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														{dst=0; pn=0;}
														for (l=st+1; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
													else {vt=0; dst=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss- +Tlss- */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
										}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phio=-atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													{dst=0; pn=0;}
													for (l=st+1; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* -Rlss- +Rlss- */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
										}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phio=-atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													{dst=0; pn=0;}
													for (l=st+1; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0- */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phio=-atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														{dst=0; pn=0;}
														for (l=st+1; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
													else {vt=0; dst=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +TdB0- */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phio=-atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														{dst=0; pn=0;}
														for (l=st+1; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
													else {vt=0; dst=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0- */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
										}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phio=-atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													{dst=0; pn=0;}
													for (l=st+1; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* +RdB0- */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
										}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phio=-atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													{dst=0; pn=0;}
													for (l=st+1; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0- +Tl0- */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
								strat2=g_strsplit_set(strary2[m], "\t,", 0);
								if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
								{
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									g_array_free(x, TRUE);
									g_array_free(yb, TRUE);
									g_array_free(delf, TRUE);
									g_array_free(stars, TRUE);
									x=g_array_new(FALSE, FALSE, sizeof(gdouble));
									yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
									lc=lcib;
									for (k=kib; k<sal; k++)
									{
										if (!strary[k]) continue;
										g_strchug(strary[k]);
										if (!g_strcmp0("", strary[k])) continue;
										if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
										if (lc<0) {lc++; continue;}
										strat=g_strsplit_set(strary[k], "\t,", 0);
										lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
										g_array_append_val(x, lcl);
										if (!strat[trc]) lcl=0;
										else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
										g_array_append_val(yb, lcl);
										g_strfreev(strat);
										lc++;
									}
									g_strfreev(strary);
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
									for (j=0; j<n; j++) y[j]=0;
									for (j=0; j<=jdimx; j++)
									{
									}
									fftw_execute(p);
									stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
									for (j=0; j<n; j++)
									{
										iv=star[j];
										g_array_append_val(stars, iv);
									}
									for (j=0; j<=jdimx; j++)
									{
										vzt=g_array_index(stars, gdouble, j*zp);
										iv=g_array_index(delf, gdouble, j);
										if (iv<DZE) idelf=G_MAXDOUBLE;
										else idelf=1/iv;
										iv=g_array_index(zwa, gdouble, j)*idelf/2;
										for (l=1; l<iv; l++)
										{
											ivd=g_array_index(stars, gdouble, l+(j*zp));
											ivd*=ivd;
											ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
											ivdt*=ivdt;
											vzt+=sqrt(ivd+ivdt);
										}
										if (vzt<DZE) vzt=G_MAXDOUBLE;
										else vzt=l/vzt;
										for (k=0; k<=kdimx; k++)
										{
											st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
											sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
											tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
											twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
											if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
											{
												vt=g_array_index(stars, gdouble, st+(j*zp));
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
												phio=-atan2(ivdt, vt);
												vt*=vt;
												ivdt*=ivdt;
												vt=sqrt(vt+ivdt);
												{dst=0; pn=0;}
												for (l=st+1; l<=sp; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
											else {vt=0; dst=0;}
											g_array_append_val(vis, vt);
											g_array_append_val(doms, dst);
										}
									}
								}
								else
								{
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									g_error_free(Err);
								}
								g_free(contents);
								g_strfreev(strat2);
							}
						}
						else /* -Rl0- +Rl0- */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
								strat2=g_strsplit_set(strary2[m], "\t,", 0);
								if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
								{
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									g_array_free(x, TRUE);
									g_array_free(yb, TRUE);
									g_array_free(delf, TRUE);
									g_array_free(stars, TRUE);
									x=g_array_new(FALSE, FALSE, sizeof(gdouble));
									yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
									lc=lcib;
									for (k=kib; k<sal; k++)
									{
										if (!strary[k]) continue;
										g_strchug(strary[k]);
										if (!g_strcmp0("", strary[k])) continue;
										if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
										if (lc<0) {lc++; continue;}
										strat=g_strsplit_set(strary[k], "\t,", 0);
										lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
										g_array_append_val(x, lcl);
										if (!strat[trc]) lcl=0;
										else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
										g_array_append_val(yb, lcl);
										g_strfreev(strat);
										lc++;
									}
									g_strfreev(strary);
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
									for (j=0; j<n; j++) y[j]=0;
									for (j=0; j<=jdimx; j++)
									{
									}
									fftw_execute(p);
									stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
									for (j=0; j<n; j++)
									{
										iv=star[j];
										g_array_append_val(stars, iv);
									}
									for (j=0; j<=jdimx; j++)
									{
										vzt=g_array_index(stars, gdouble, j*zp);
										iv=g_array_index(delf, gdouble, j);
										if (iv<DZE) idelf=G_MAXDOUBLE;
										else idelf=1/iv;
										iv=g_array_index(zwa, gdouble, j)*idelf/2;
										for (l=1; l<iv; l++)
										{
											ivd=g_array_index(stars, gdouble, l+(j*zp));
											ivd*=ivd;
											ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
											ivdt*=ivdt;
											vzt+=sqrt(ivd+ivdt);
										}
										if (vzt<DZE) vzt=G_MAXDOUBLE;
										else vzt=l/vzt;
										for (k=0; k<=kdimx; k++)
										{
											st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
											sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
											tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
											twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
											if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
											{
												vt=g_array_index(stars, gdouble, st+(j*zp));
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
												phio=-atan2(ivdt, vt);
												vt*=vt;
												ivdt*=ivdt;
												vt=sqrt(vt+ivdt);
												{dst=0; pn=0;}
												for (l=st+1; l<=sp; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
											else {vt=0; dst=0;}
											g_array_append_val(vis, vt);
											g_array_append_val(doms, dst);
										}
									}
								}
								else
								{
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									g_error_free(Err);
								}
								g_free(contents);
								g_strfreev(strat2);
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
					{
						flags&=47;
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss+ */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
												iv=g_array_index(bsra, gdouble, j);
												k=0;
												while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
												st=k;
												iv=g_array_index(bspa, gdouble, j);
												while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
												sp=k-st;
												if (sp>zp) sp=zp;
												iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
												g_array_append_val(delf, iv);
												for (k=0; k<sp; k++)
												{
													clc=ofs-g_array_index(yb, gdouble, k+st);
													if (clc<0)
													{
														clc=-exp(LNTOT*clc);
														y[k+(j*zp)]=log(++clc);
													}
													else y[k+(j*zp)]=-G_MAXDOUBLE;
												}
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phio=-atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														{dst=0; pn=0;}
														for (l=st+1; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
													else {vt=0; dst=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +TdBss+ */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											g_array_free(x, TRUE);
											g_array_free(yb, TRUE);
											g_array_free(delf, TRUE);
											g_array_free(stars, TRUE);
											x=g_array_new(FALSE, FALSE, sizeof(gdouble));
											yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
											lc=lcib;
											for (k=kib; k<sal; k++)
											{
												if (!strary[k]) continue;
												g_strchug(strary[k]);
												if (!g_strcmp0("", strary[k])) continue;
												if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
												if (lc<0) {lc++; continue;}
												strat=g_strsplit_set(strary[k], "\t,", 0);
												lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
												g_array_append_val(x, lcl);
												if (!strat[trc]) lcl=0;
												else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
												g_array_append_val(yb, lcl);
												g_strfreev(strat);
												lc++;
											}
											g_strfreev(strary);
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<=jdimx; j++)
											{
												iv=g_array_index(bsra, gdouble, j);
												k=0;
												while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
												st=k;
												iv=g_array_index(bspa, gdouble, j);
												while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
												sp=k-st;
												if (sp>zp) sp=zp;
												iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
												g_array_append_val(delf, iv);
												for (k=0; k<sp; k++)
												{
													clc=g_array_index(yb, gdouble, k+st)-ofs;
													if (clc<0)
													{
														clc=-exp(LNTOT*clc);
														y[k+(j*zp)]=log(++clc);
													}
													else y[k+(j*zp)]=-G_MAXDOUBLE;
												}
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<=jdimx; j++)
											{
												vzt=g_array_index(stars, gdouble, j*zp);
												iv=g_array_index(delf, gdouble, j);
												if (iv<DZE) idelf=G_MAXDOUBLE;
												else idelf=1/iv;
												iv=g_array_index(zwa, gdouble, j)*idelf/2;
												for (l=1; l<iv; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivd*=ivd;
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
													ivdt*=ivdt;
													vzt+=sqrt(ivd+ivdt);
												}
												if (vzt<DZE) vzt=G_MAXDOUBLE;
												else vzt=l/vzt;
												for (k=0; k<=kdimx; k++)
												{
													st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
													sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
													tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
													twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
													if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
													{
														vt=g_array_index(stars, gdouble, st+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
														phio=-atan2(ivdt, vt);
														vt*=vt;
														ivdt*=ivdt;
														vt=sqrt(vt+ivdt);
														{dst=0; pn=0;}
														for (l=st+1; l<=sp; l++)
														{
															ivd=g_array_index(stars, gdouble, l+(j*zp));
															ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
													else {vt=0; dst=0;}
													g_array_append_val(vis, vt);
													g_array_append_val(doms, dst);
												}
											}
										}
										else
										{
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss+ */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
										}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
											iv=g_array_index(bsra, gdouble, j);
											k=0;
											while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
											st=k;
											iv=g_array_index(bspa, gdouble, j);
											while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
											sp=k-st;
											if (sp>zp) sp=zp;
											iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
											g_array_append_val(delf, iv);
											for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(ofs-g_array_index(yb, gdouble, k+st));
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phio=-atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													{dst=0; pn=0;}
													for (l=st+1; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* +RdBss+ */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
										}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
											iv=g_array_index(bsra, gdouble, j);
											k=0;
											while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
											st=k;
											iv=g_array_index(bspa, gdouble, j);
											while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
											sp=k-st;
											if (sp>zp) sp=zp;
											iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
											g_array_append_val(delf, iv);
											for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(g_array_index(yb, gdouble, k+st)-ofs);
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phio=-atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													{dst=0; pn=0;}
													for (l=st+1; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
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
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
											}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
											iv=g_array_index(bsra, gdouble, j);
											k=0;
											while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
											st=k;
											iv=g_array_index(bspa, gdouble, j);
											while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
											sp=k-st;
											if (sp>zp) sp=zp;
											iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
											g_array_append_val(delf, iv);
											for (k=0; k<sp; k++)
											{
												clc=-g_array_index(yb, gdouble, k+st)/ofs;
												clc++;
												if (clc>0) y[k+(j*zp)]=log(clc);
												else y[k+(j*zp)]=-G_MAXDOUBLE;
											}
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phio=-atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													{dst=0; pn=0;}
													for (l=st+1; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
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
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
										}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
											iv=g_array_index(bsra, gdouble, j);
											k=0;
											while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
											st=k;
											iv=g_array_index(bspa, gdouble, j);
											while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
											sp=k-st;
											if (sp>zp) sp=zp;
											iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
											g_array_append_val(delf, iv);
											for (k=0; k<sp; k++)
											{
												clc=g_array_index(yb, gdouble, k+st)/ofs;
												if (clc>0) y[k+(j*zp)]=log(clc);
												else y[k+(j*zp)]=-G_MAXDOUBLE;
											}
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phio=-atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													{dst=0; pn=0;}
													for (l=st+1; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
					{
						flags&=47;
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0+ */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
										}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
											iv=g_array_index(bsra, gdouble, j);
											k=0;
											while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
											st=k;
											iv=g_array_index(bspa, gdouble, j);
											while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
											sp=k-st;
											if (sp>zp) sp=zp;
											iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
											g_array_append_val(delf, iv);
											for (k=0; k<sp; k++)
											{
												clc=ofs-g_array_index(yb, gdouble, k+st);
												clc=-exp(LNTOT*clc);
												y[k+(j*zp)]=++clc;
											}
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phio=-atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													{dst=0; pn=0;}
													for (l=st+1; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* +TdB0+ */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										g_array_free(x, TRUE);
										g_array_free(yb, TRUE);
										g_array_free(delf, TRUE);
										g_array_free(stars, TRUE);
										x=g_array_new(FALSE, FALSE, sizeof(gdouble));
										yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
										lc=lcib;
										for (k=kib; k<sal; k++)
										{
											if (!strary[k]) continue;
											g_strchug(strary[k]);
											if (!g_strcmp0("", strary[k])) continue;
											if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
											if (lc<0) {lc++; continue;}
											strat=g_strsplit_set(strary[k], "\t,", 0);
											lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
											g_array_append_val(x, lcl);
											if (!strat[trc]) lcl=0;
											else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
											g_array_append_val(yb, lcl);
											g_strfreev(strat);
											lc++;
										}
										g_strfreev(strary);
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<=jdimx; j++)
										{
											iv=g_array_index(bsra, gdouble, j);
											k=0;
											while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
											st=k;
											iv=g_array_index(bspa, gdouble, j);
											while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
											sp=k-st;
											if (sp>zp) sp=zp;
											iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
											g_array_append_val(delf, iv);
											for (k=0; k<sp; k++)
											{
												clc=g_array_index(yb, gdouble, k+st)-ofs;
												clc=-exp(LNTOT*clc);
												y[k+(j*zp)]=++clc;
											}
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<=jdimx; j++)
										{
											vzt=g_array_index(stars, gdouble, j*zp);
											iv=g_array_index(delf, gdouble, j);
											if (iv<DZE) idelf=G_MAXDOUBLE;
											else idelf=1/iv;
											iv=g_array_index(zwa, gdouble, j)*idelf/2;
											for (l=1; l<iv; l++)
											{
												ivd=g_array_index(stars, gdouble, l+(j*zp));
												ivd*=ivd;
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
												ivdt*=ivdt;
												vzt+=sqrt(ivd+ivdt);
											}
											if (vzt<DZE) vzt=G_MAXDOUBLE;
											else vzt=l/vzt;
											for (k=0; k<=kdimx; k++)
											{
												st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
												sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
												tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
												twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
												if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
												{
													vt=g_array_index(stars, gdouble, st+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
													phio=-atan2(ivdt, vt);
													vt*=vt;
													ivdt*=ivdt;
													vt=sqrt(vt+ivdt);
													{dst=0; pn=0;}
													for (l=st+1; l<=sp; l++)
													{
														ivd=g_array_index(stars, gdouble, l+(j*zp));
														ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
												else {vt=0; dst=0;}
												g_array_append_val(vis, vt);
												g_array_append_val(doms, dst);
											}
										}
									}
									else
									{
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0+ */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
								strat2=g_strsplit_set(strary2[m], "\t,", 0);
								if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
								{
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									g_array_free(x, TRUE);
									g_array_free(yb, TRUE);
									g_array_free(delf, TRUE);
									g_array_free(stars, TRUE);
									x=g_array_new(FALSE, FALSE, sizeof(gdouble));
									yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
									lc=lcib;
									for (k=kib; k<sal; k++)
									{
										if (!strary[k]) continue;
										g_strchug(strary[k]);
										if (!g_strcmp0("", strary[k])) continue;
										if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
										if (lc<0) {lc++; continue;}
										strat=g_strsplit_set(strary[k], "\t,", 0);
										lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
										g_array_append_val(x, lcl);
										if (!strat[trc]) lcl=0;
										else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
										g_array_append_val(yb, lcl);
										g_strfreev(strat);
										lc++;
									}
									g_strfreev(strary);
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
									for (j=0; j<n; j++) y[j]=0;
									for (j=0; j<=jdimx; j++)
									{
										iv=g_array_index(bsra, gdouble, j);
										k=0;
										while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
										st=k;
										iv=g_array_index(bspa, gdouble, j);
										while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
										sp=k-st;
										if (sp>zp) sp=zp;
										iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
										g_array_append_val(delf, iv);
										for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(ofs-g_array_index(yb, gdouble, k+st)));
									}
									fftw_execute(p);
									stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
									for (j=0; j<n; j++)
									{
										iv=star[j];
										g_array_append_val(stars, iv);
									}
									for (j=0; j<=jdimx; j++)
									{
										vzt=g_array_index(stars, gdouble, j*zp);
										iv=g_array_index(delf, gdouble, j);
										if (iv<DZE) idelf=G_MAXDOUBLE;
										else idelf=1/iv;
										iv=g_array_index(zwa, gdouble, j)*idelf/2;
										for (l=1; l<iv; l++)
										{
											ivd=g_array_index(stars, gdouble, l+(j*zp));
											ivd*=ivd;
											ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
											ivdt*=ivdt;
											vzt+=sqrt(ivd+ivdt);
										}
										if (vzt<DZE) vzt=G_MAXDOUBLE;
										else vzt=l/vzt;
										for (k=0; k<=kdimx; k++)
										{
											st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
											sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
											tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
											twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
											if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
											{
												vt=g_array_index(stars, gdouble, st+(j*zp));
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
												phio=-atan2(ivdt, vt);
												vt*=vt;
												ivdt*=ivdt;
												vt=sqrt(vt+ivdt);
												{dst=0; pn=0;}
												for (l=st+1; l<=sp; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
											else {vt=0; dst=0;}
											g_array_append_val(vis, vt);
											g_array_append_val(doms, dst);
										}
									}
								}
								else
								{
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									g_error_free(Err);
								}
								g_free(contents);
								g_strfreev(strat2);
							}
						}
						else /* +RdB0+ */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
								strat2=g_strsplit_set(strary2[m], "\t,", 0);
								if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
								{
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									g_array_free(x, TRUE);
									g_array_free(yb, TRUE);
									g_array_free(delf, TRUE);
									g_array_free(stars, TRUE);
									x=g_array_new(FALSE, FALSE, sizeof(gdouble));
									yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
									lc=lcib;
									for (k=kib; k<sal; k++)
									{
										if (!strary[k]) continue;
										g_strchug(strary[k]);
										if (!g_strcmp0("", strary[k])) continue;
										if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
										if (lc<0) {lc++; continue;}
										strat=g_strsplit_set(strary[k], "\t,", 0);
										lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
										g_array_append_val(x, lcl);
										if (!strat[trc]) lcl=0;
										else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
										g_array_append_val(yb, lcl);
										g_strfreev(strat);
										lc++;
									}
									g_strfreev(strary);
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
									for (j=0; j<n; j++) y[j]=0;
									for (j=0; j<=jdimx; j++)
									{
										iv=g_array_index(bsra, gdouble, j);
										k=0;
										while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
										st=k;
										iv=g_array_index(bspa, gdouble, j);
										while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
										sp=k-st;
										if (sp>zp) sp=zp;
										iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
										g_array_append_val(delf, iv);
										for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(g_array_index(yb, gdouble, k+st)-ofs));
									}
									fftw_execute(p);
									stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
									for (j=0; j<n; j++)
									{
										iv=star[j];
										g_array_append_val(stars, iv);
									}
									for (j=0; j<=jdimx; j++)
									{
										vzt=g_array_index(stars, gdouble, j*zp);
										iv=g_array_index(delf, gdouble, j);
										if (iv<DZE) idelf=G_MAXDOUBLE;
										else idelf=1/iv;
										iv=g_array_index(zwa, gdouble, j)*idelf/2;
										for (l=1; l<iv; l++)
										{
											ivd=g_array_index(stars, gdouble, l+(j*zp));
											ivd*=ivd;
											ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
											ivdt*=ivdt;
											vzt+=sqrt(ivd+ivdt);
										}
										if (vzt<DZE) vzt=G_MAXDOUBLE;
										else vzt=l/vzt;
										for (k=0; k<=kdimx; k++)
										{
											st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
											sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
											tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
											twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
											if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
											{
												vt=g_array_index(stars, gdouble, st+(j*zp));
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
												phio=-atan2(ivdt, vt);
												vt*=vt;
												ivdt*=ivdt;
												vt=sqrt(vt+ivdt);
												{dst=0; pn=0;}
												for (l=st+1; l<=sp; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
											else {vt=0; dst=0;}
											g_array_append_val(vis, vt);
											g_array_append_val(doms, dst);
										}
									}
								}
								else
								{
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									g_error_free(Err);
								}
								g_free(contents);
								g_strfreev(strat2);
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0+ +Tl0+ */
					{
						flags&=47;
						if ((ofs<DZE)&&(ofs>NZE))
						{
							str=g_strdup(_("Offset must be nonzero for linear measurements."));
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
						}
						else
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
								strat2=g_strsplit_set(strary2[m], "\t,", 0);
								if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
								{
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									g_array_free(x, TRUE);
									g_array_free(yb, TRUE);
									g_array_free(delf, TRUE);
									g_array_free(stars, TRUE);
									x=g_array_new(FALSE, FALSE, sizeof(gdouble));
									yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
									lc=lcib;
									for (k=kib; k<sal; k++)
									{
										if (!strary[k]) continue;
										g_strchug(strary[k]);
										if (!g_strcmp0("", strary[k])) continue;
										if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
										if (lc<0) {lc++; continue;}
										strat=g_strsplit_set(strary[k], "\t,", 0);
										lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
										g_array_append_val(x, lcl);
										if (!strat[trc]) lcl=0;
										else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
										g_array_append_val(yb, lcl);
										g_strfreev(strat);
										lc++;
									}
									g_strfreev(strary);
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
									for (j=0; j<n; j++) y[j]=0;
									for (j=0; j<=jdimx; j++)
									{
										iv=g_array_index(bsra, gdouble, j);
										k=0;
										while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
										st=k;
										iv=g_array_index(bspa, gdouble, j);
										while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
										sp=k-st;
										if (sp>zp) sp=zp;
										iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
										g_array_append_val(delf, iv);
										for (k=0; k<sp; k++)
										{
											clc=-g_array_index(yb, gdouble, k+st)/ofs;
											y[k+(j*zp)]=++clc;
										}
									}
									fftw_execute(p);
									stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
									for (j=0; j<n; j++)
									{
										iv=star[j];
										g_array_append_val(stars, iv);
									}
									for (j=0; j<=jdimx; j++)
									{
										vzt=g_array_index(stars, gdouble, j*zp);
										iv=g_array_index(delf, gdouble, j);
										if (iv<DZE) idelf=G_MAXDOUBLE;
										else idelf=1/iv;
										iv=g_array_index(zwa, gdouble, j)*idelf/2;
										for (l=1; l<iv; l++)
										{
											ivd=g_array_index(stars, gdouble, l+(j*zp));
											ivd*=ivd;
											ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
											ivdt*=ivdt;
											vzt+=sqrt(ivd+ivdt);
										}
										if (vzt<DZE) vzt=G_MAXDOUBLE;
										else vzt=l/vzt;
										for (k=0; k<=kdimx; k++)
										{
											st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
											sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
											tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
											twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
											if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
											{
												vt=g_array_index(stars, gdouble, st+(j*zp));
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
												phio=-atan2(ivdt, vt);
												vt*=vt;
												ivdt*=ivdt;
												vt=sqrt(vt+ivdt);
												{dst=0; pn=0;}
												for (l=st+1; l<=sp; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
											else {vt=0; dst=0;}
											g_array_append_val(vis, vt);
											g_array_append_val(doms, dst);
										}
									}
								}
								else
								{
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									g_error_free(Err);
								}
								g_free(contents);
								g_strfreev(strat2);
							}
						}
					}
					else /* -Rl0+ +Rl0+ */
					{
						flags&=47;
						if ((ofs<DZE)&&(ofs>NZE))
						{
							str=g_strdup(_("Offset must be nonzero for linear measurements."));
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
						}
						else
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
								strat2=g_strsplit_set(strary2[m], "\t,", 0);
								if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
								{
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									g_array_free(x, TRUE);
									g_array_free(yb, TRUE);
									g_array_free(delf, TRUE);
									g_array_free(stars, TRUE);
									x=g_array_new(FALSE, FALSE, sizeof(gdouble));
									yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
									lc=lcib;
									for (k=kib; k<sal; k++)
									{
										if (!strary[k]) continue;
										g_strchug(strary[k]);
										if (!g_strcmp0("", strary[k])) continue;
										if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
										if (lc<0) {lc++; continue;}
										strat=g_strsplit_set(strary[k], "\t,", 0);
										lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
										g_array_append_val(x, lcl);
										if (!strat[trc]) lcl=0;
										else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
										g_array_append_val(yb, lcl);
										g_strfreev(strat);
										lc++;
									}
									g_strfreev(strary);
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
									for (j=0; j<n; j++) y[j]=0;
									for (j=0; j<=jdimx; j++)
									{
										iv=g_array_index(bsra, gdouble, j);
										k=0;
										while ((k<lc)&&(iv>g_array_index(x, gdouble, k))) k++;
										st=k;
										iv=g_array_index(bspa, gdouble, j);
										while ((k<lc)&&(iv>=g_array_index(x, gdouble, k))) k++;
										sp=k-st;
										if (sp>zp) sp=zp;
										iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
										g_array_append_val(delf, iv);
										for (k=0; k<sp; k++) y[k+(j*zp)]=g_array_index(yb, gdouble, k+st)/ofs;
									}
									fftw_execute(p);
									stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
									for (j=0; j<n; j++)
									{
										iv=star[j];
										g_array_append_val(stars, iv);
									}
									for (j=0; j<=jdimx; j++)
									{
										vzt=g_array_index(stars, gdouble, j*zp);
										iv=g_array_index(delf, gdouble, j);
										if (iv<DZE) idelf=G_MAXDOUBLE;
										else idelf=1/iv;
										iv=g_array_index(zwa, gdouble, j)*idelf/2;
										for (l=1; l<iv; l++)
										{
											ivd=g_array_index(stars, gdouble, l+(j*zp));
											ivd*=ivd;
											ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
											ivdt*=ivdt;
											vzt+=sqrt(ivd+ivdt);
										}
										if (vzt<DZE) vzt=G_MAXDOUBLE;
										else vzt=l/vzt;
										for (k=0; k<=kdimx; k++)
										{
											st=ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
											sp=floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
											tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
											twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
											if ((st<((zp/2)-1))&&(sp<(zp/2))&&((sp-st)>0))
											{
												vt=g_array_index(stars, gdouble, st+(j*zp));
												ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-st);
												phio=-atan2(ivdt, vt);
												vt*=vt;
												ivdt*=ivdt;
												vt=sqrt(vt+ivdt);
												{dst=0; pn=0;}
												for (l=st+1; l<=sp; l++)
												{
													ivd=g_array_index(stars, gdouble, l+(j*zp));
													ivdt=g_array_index(stars, gdouble, ((j+1)*zp)-l);
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
											else {vt=0; dst=0;}
											g_array_append_val(vis, vt);
											g_array_append_val(doms, dst);
										}
									}
								}
								else
								{
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									g_error_free(Err);
								}
								g_free(contents);
								g_strfreev(strat2);
							}
						}
					}
					g_strfreev(strary2);
					fftw_destroy_plan(p);
					fftw_free(y);
					fftw_free(star);
					bnx=g_array_new(FALSE, FALSE, sizeof(gint));
					bsz=g_array_new(FALSE, FALSE, sizeof(gint));
					dx2=0;
					if ((flagd&2)==0)
					{
						if ((flagd&4)==0)/* single plot mode */
						{
							bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
							byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
							if ((flagd&16)==0)
							{
								if ((flagd&8)==0)
								{
									xx=g_array_index(msr, gdouble, 0);
									g_array_append_val(bxr, xx);
									mny=g_array_index(vis, gdouble, kdim+(jdim*kdimx));
									g_array_append_val(byr, mny);
									mxy=mny;
									for (j=1; j<mx; j++)
									{
										xx=g_array_index(msr, gdouble, j);
										g_array_append_val(bxr, xx);
										xx=g_array_index(vis, gdouble, kdim+((jdim+(j*jdimx))*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
								}
								else
								{
									xx=g_array_index(msr, gdouble, 0);
									g_array_append_val(bxr, xx);
									mny=g_array_index(doms, gdouble, kdim+(jdim*kdimx));
									g_array_append_val(byr, mny);
									mxy=mny;
									for (j=1; j<mx; j++)
									{
										xx=g_array_index(msr, gdouble, j);
										g_array_append_val(bxr, xx);
										xx=g_array_index(doms, gdouble, kdim+((jdim+(j*jdimx))*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
								}
							}
							else
							{
								xx=g_array_index(msr, gdouble, 0);
								g_array_append_val(bxr, xx);
								mny=g_array_index(chp, gdouble, kdim+(jdim*kdimx));
								g_array_append_val(byr, mny);
								mxy=mny;
								for (j=1; j<mx; j++)
								{
									xx=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, xx);
									xx=g_array_index(chp, gdouble, kdim+((jdim+(j*jdimx))*kdimx));
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
								}
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
						}
						else/* multiplot over k */
						{
							bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimx);
							byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimx);
							if ((flagd&16)==0)
							{
								if ((flagd&8)==0)
								{
									xx=g_array_index(msr, gdouble, 0);
									g_array_append_val(bxr, xx);
									mny=g_array_index(vis, gdouble, (jdim*kdimx));
									g_array_append_val(byr, mny);
									mxy=mny;
									for (j=1; j<mx; j++)
									{
										xx=g_array_index(msr, gdouble, j);
										g_array_append_val(bxr, xx);
										xx=g_array_index(vis, gdouble, ((jdim+(j*jdimx))*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									dx2+=mx;
									for (k=1; k<kdimx; k++)
									{
										for (j=0; j<mx; j++)
										{
											xx=g_array_index(msr, gdouble, j);
											g_array_append_val(bxr, xx);
											xx=g_array_index(vis, gdouble, k+((jdim+(j*jdimx))*kdimx));
											g_array_append_val(byr, xx);
											if (xx>mxy) mxy=xx;
											else if (xx<mny) mny=xx;
										}
										g_array_append_val(bsz, mx);
										g_array_append_val(bnx, dx2);
										dx2+=mx;
									}
								}
								else
								{
									xx=g_array_index(msr, gdouble, 0);
									g_array_append_val(bxr, xx);
									mny=g_array_index(doms, gdouble, (jdim*kdimx));
									g_array_append_val(byr, mny);
									mxy=mny;
									for (j=1; j<mx; j++)
									{
										xx=g_array_index(msr, gdouble, j);
										g_array_append_val(bxr, xx);
										xx=g_array_index(doms, gdouble, ((jdim+(j*jdimx))*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									dx2+=mx;
									for (k=1; k<kdimx; k++)
									{
										for (j=0; j<mx; j++)
										{
											xx=g_array_index(msr, gdouble, j);
											g_array_append_val(bxr, xx);
											xx=g_array_index(doms, gdouble, k+((jdim+(j*jdimx))*kdimx));
											g_array_append_val(byr, xx);
											if (xx>mxy) mxy=xx;
											else if (xx<mny) mny=xx;
										}
										g_array_append_val(bsz, mx);
										g_array_append_val(bnx, dx2);
										dx2+=mx;
									}
								}
							}
							else
							{
								xx=g_array_index(msr, gdouble, 0);
								g_array_append_val(bxr, xx);
								mny=g_array_index(chp, gdouble, (jdim*kdimx));
								g_array_append_val(byr, mny);
								mxy=mny;
								for (j=1; j<mx; j++)
								{
									xx=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, xx);
									xx=g_array_index(chp, gdouble, ((jdim+(j*jdimx))*kdimx));
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
								for (k=1; k<kdimx; k++)
								{
									for (j=0; j<mx; j++)
									{
										xx=g_array_index(msr, gdouble, j);
										g_array_append_val(bxr, xx);
										xx=g_array_index(chp, gdouble, k+((jdim+(j*jdimx))*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									dx2+=mx;
								}
							}
						}
					}
					else if ((flagd&4)==0)/* multiplot over j */
					{
						bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimx);
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimx);
						if ((flagd&16)==0)
						{
							if ((flagd&8)==0)
							{
								xx=g_array_index(msr, gdouble, 0);
								g_array_append_val(bxr, xx);
								mny=g_array_index(vis, gdouble, kdim);
								g_array_append_val(byr, mny);
								mxy=mny;
								for (j=1; j<mx; j++)
								{
									xx=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, xx);
									xx=g_array_index(vis, gdouble, kdim+(j*jdimx*kdimx));
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
								for (k=1; k<jdimx; k++)
								{
									for (j=0; j<mx; j++)
									{
										xx=g_array_index(msr, gdouble, j);
										g_array_append_val(bxr, xx);
										xx=g_array_index(vis, gdouble, kdim+((k+(j*jdimx))*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									dx2+=mx;
								}
							}
							else
							{
								xx=g_array_index(msr, gdouble, 0);
								g_array_append_val(bxr, xx);
								mny=g_array_index(doms, gdouble, kdim);
								g_array_append_val(byr, mny);
								mxy=mny;
								for (j=1; j<mx; j++)
								{
									xx=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, xx);
									xx=g_array_index(doms, gdouble, kdim+(j*jdimx*kdimx));
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
								for (k=1; k<jdimx; k++)
								{
									for (j=0; j<mx; j++)
									{
										xx=g_array_index(msr, gdouble, j);
										g_array_append_val(bxr, xx);
										xx=g_array_index(doms, gdouble, kdim+((k+(j*jdimx))*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									dx2+=mx;
								}
							}
						}
						else
						{
							xx=g_array_index(msr, gdouble, 0);
							g_array_append_val(bxr, xx);
							mny=g_array_index(chp, gdouble, kdim);
							g_array_append_val(byr, mny);
							mxy=mny;
							for (j=1; j<mx; j++)
							{
								xx=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, xx);
								xx=g_array_index(chp, gdouble, kdim+(j*jdimx*kdimx));
								g_array_append_val(byr, xx);
								if (xx>mxy) mxy=xx;
								else if (xx<mny) mny=xx;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
							for (k=1; k<jdimx; k++)
							{
								for (j=0; j<mx; j++)
								{
									xx=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, xx);
									xx=g_array_index(chp, gdouble, kdim+((k+(j*jdimx))*kdimx));
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
							}
						}
					}
					else
					{
						bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimx*kdimx);
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimx*kdimx);
						if ((flagd&16)==0)
						{
							if ((flagd&8)==0)
							{
								xx=g_array_index(msr, gdouble, 0);
								g_array_append_val(bxr, xx);
								mny=g_array_index(vis, gdouble, 0);
								g_array_append_val(byr, mny);
								mxy=mny;
								for (j=1; j<mx; j++)
								{
									xx=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, xx);
									xx=g_array_index(vis, gdouble, (j*jdimx*kdimx));
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
								for (k=1; k<kdimx; k++)
								{
									for (j=0; j<mx; j++)
									{
										xx=g_array_index(msr, gdouble, j);
										g_array_append_val(bxr, xx);
										xx=g_array_index(vis, gdouble, k+(j*jdimx*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									dx2+=mx;
								}
								for (l=1; l<jdimx; l++)
								{
									for (k=0; k<kdimx; k++)
									{
										for (j=0; j<mx; j++)
										{
											xx=g_array_index(msr, gdouble, j);
											g_array_append_val(bxr, xx);
											xx=g_array_index(vis, gdouble, k+((l+(j*jdimx))*kdimx));
											g_array_append_val(byr, xx);
											if (xx>mxy) mxy=xx;
											else if (xx<mny) mny=xx;
										}
										g_array_append_val(bsz, mx);
										g_array_append_val(bnx, dx2);
										dx2+=mx;
									}
								}
							}
							else
							{
								xx=g_array_index(msr, gdouble, 0);
								g_array_append_val(bxr, xx);
								mny=g_array_index(doms, gdouble, 0);
								g_array_append_val(byr, mny);
								mxy=mny;
								for (j=1; j<mx; j++)
								{
									xx=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, xx);
									xx=g_array_index(doms, gdouble, (j*jdimx*kdimx));
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
								for (k=1; k<kdimx; k++)
								{
									for (j=0; j<mx; j++)
									{
										xx=g_array_index(msr, gdouble, j);
										g_array_append_val(bxr, xx);
										xx=g_array_index(doms, gdouble, k+(j*jdimx*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									dx2+=mx;
								}
								for (l=1; l<jdimx; l++)
								{
									for (k=0; k<kdimx; k++)
									{
										for (j=0; j<mx; j++)
										{
											xx=g_array_index(msr, gdouble, j);
											g_array_append_val(bxr, xx);
											xx=g_array_index(doms, gdouble, k+((l+(j*jdimx))*kdimx));
											g_array_append_val(byr, xx);
											if (xx>mxy) mxy=xx;
											else if (xx<mny) mny=xx;
										}
										g_array_append_val(bsz, mx);
										g_array_append_val(bnx, dx2);
										dx2+=mx;
									}
								}
							}
						}
						else
						{
							xx=g_array_index(msr, gdouble, 0);
							g_array_append_val(bxr, xx);
							mny=g_array_index(chp, gdouble, 0);
							g_array_append_val(byr, mny);
							mxy=mny;
							for (j=1; j<mx; j++)
							{
								xx=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, xx);
								xx=g_array_index(chp, gdouble, (j*jdimx*kdimx));
								g_array_append_val(byr, xx);
								if (xx>mxy) mxy=xx;
								else if (xx<mny) mny=xx;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
							for (k=1; k<kdimx; k++)
							{
								for (j=0; j<mx; j++)
								{
									xx=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, xx);
									xx=g_array_index(chp, gdouble, k+(j*jdimx*kdimx));
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
							}
							for (l=1; l<jdimx; l++)
							{
								for (k=0; k<kdimx; k++)
								{
									for (j=0; j<mx; j++)
									{
										xx=g_array_index(msr, gdouble, j);
										g_array_append_val(bxr, xx);
										xx=g_array_index(chp, gdouble, k+((l+(j*jdimx))*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									dx2+=mx;
								}
							}
						}
					}
					dialog=gtk_dialog_new_with_buttons(_("Measurand Variable Type"), GTK_WINDOW(window), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Linear"), 1, _("Polar"), 2, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
					cont=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
					label=gtk_label_new(_("Select Parameter to save:"));
					gtk_container_add(GTK_CONTAINER(cont), label);
					switch (gtk_dialog_run(GTK_DIALOG(dialog)))
					{
						case 1:
						if ((flags&8)==0)
						{
							gtk_notebook_remove_page(GTK_NOTEBOOK(notebook2), 2);
							table=gtk_table_new(1, 1, FALSE);
							gtk_widget_show(table);
							plot3=plot_linear_new();
							g_signal_connect(plot3, "moved", G_CALLBACK(pltmv), NULL);
							gtk_widget_show(plot3);
							gtk_table_attach(GTK_TABLE(table), plot3, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
							label=gtk_label_new(_("Analysis Results"));
							gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), table, label);
							flags|=15;
						}
						else if ((flags&32)!=0)
						{
							gtk_notebook_remove_page(GTK_NOTEBOOK(notebook2), 2);
							table=gtk_table_new(1, 1, FALSE);
							gtk_widget_show(table);
							plot3=plot_linear_new();
							g_signal_connect(plot3, "moved", G_CALLBACK(pltmv), NULL);
							gtk_widget_show(plot3);
							gtk_table_attach(GTK_TABLE(table), plot3, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
							label=gtk_label_new(_("Analysis Results"));
							gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), table, label);
							flags^=32;
						}
						plt=PLOT_LINEAR(plot3);
						(plt->xdata)=bxr;
						(plt->ydata)=byr;
						(plt->sizes)=bsz;
						(plt->ind)=bnx;
						plot_linear_update_scale_pretty(plot3, xi, xf, mny, mxy);
						gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
						gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 2);
						break;
						case 2:
						if (((flags&8)==0)||((flags&32)==0))
						{
							gtk_notebook_remove_page(GTK_NOTEBOOK(notebook2), 2);
							table=gtk_table_new(1, 1, FALSE);
							gtk_widget_show(table);
							plot3=plot_polar_new();
							g_signal_connect(plot3, "moved", G_CALLBACK(pltmvp), NULL);
							gtk_widget_show(plot3);
							gtk_table_attach(GTK_TABLE(table), plot3, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
							label=gtk_label_new(_("Analysis Results"));
							gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), table, label);
							flags|=47;
						}
						plt2=PLOT_POLAR(plot3);
						(plt2->thdata)=bxr;
						(plt2->rdata)=byr;
						(plt2->sizes)=bsz;
						(plt2->ind)=bnx;
						plot_polar_update_scale_pretty(plot3, mny, mxy, xi, xf);
						gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
						gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 2);
						break;
						default:
						break;
					}
				}
				else
				{
					str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
					gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
					g_free(str);
					g_error_free(Err);
				}
				g_free(contents2);
				g_free(fin);
			}
		}
	}
	else /* single file mode */
	{
		wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
		g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
		if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
		{
			fin=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
			if (g_file_get_contents(fin, &contents, NULL, &Err))
			{
				strary=g_strsplit_set(contents, "\r\n", 0);
				sal=g_strv_length(strary);
				g_array_free(x, TRUE);
				g_array_free(yb, TRUE);
				g_array_free(specs, TRUE);
				if ((flags&8)!=0)
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
					flags^=8;
				}
				flags|=1;
				x=g_array_new(FALSE, FALSE, sizeof(gdouble));
				yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
				specs=g_array_new(FALSE, FALSE, sizeof(gdouble));
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(anosa))) {k=2; lc=-1;}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(sws))) {k=0; lc=-1;}
				else {k=0; lc=0;}
				while (k<sal)
				{
					if (!strary[k]) {k++; continue;}
					g_strchug(strary[k]);
					if (!g_strcmp0("", strary[k])) {k++; continue;}
					if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) {k++; continue;}
					if (lc<0) {lc++; k++; continue;}
					strat=g_strsplit_set(strary[k], "\t,", 0);
					lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
					g_array_append_val(x, lcl);
					if (lc==0)
					{
						satl=g_strv_length(strat);
						if (!strat[1]) lcl=0;
						else lcl=g_ascii_strtod(g_strstrip(strat[1]), NULL);
						mny=lcl;
						mxy=lcl;
					}
					else
					{
						if (!strat[1]) lcl=0;
						else lcl=g_ascii_strtod(g_strstrip(strat[1]), NULL);
						if (lcl<mny) mny=lcl;
						else if (lcl>mxy) mxy=lcl;
					}
					g_array_append_val(specs, lcl);
					g_array_append_val(yb, lcl);
					for (l=2; l<satl; l++)
					{
						if (!strat[l]) lcl=0;
						else lcl=g_ascii_strtod(g_strstrip(strat[l]), NULL);
						g_array_append_val(specs, lcl);
					}
					g_strfreev(strat);
					lc++;
					k++;
				}
				g_strfreev(strary);
				satl--;
				j=g_slist_length(group2);
				while (j<satl)
				{
					j++;
					g_snprintf(s, 4, "%d", j);
					trace=gtk_radio_menu_item_new_with_label(group2, s);
					group2=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(trace));
					g_signal_connect(G_OBJECT(trace), "toggled", G_CALLBACK(upg), NULL);
					gtk_menu_shell_append(GTK_MENU_SHELL(tracmenu), trace);
					gtk_widget_show(trace);
				}
				while (j>satl)
				{
					list=group2->next;
					gtk_widget_destroy(group2->data);
					group2=list;
					j--;
				}
				gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);
				gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 0);
				g_signal_connect(G_OBJECT(tr), "clicked", G_CALLBACK(trs), NULL);
				if (g_signal_handler_is_connected(G_OBJECT(pr), pr_id)) g_signal_handler_disconnect(G_OBJECT(pr), pr_id);
				str=g_strdup_printf(_("File: %s successfully loaded"), fin);
				gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
				g_free(str);
				plt=PLOT_LINEAR(plot1);
				sz=g_array_new(FALSE, FALSE, sizeof(gint));
				nx=g_array_new(FALSE, FALSE, sizeof(gint));
				g_array_append_val(sz, lc);/* adjust if multiple traces desired */
				(plt->sizes)=sz;
				zp=0;
				g_array_append_val(nx, zp);
				(plt->ind)=nx;
				(plt->xdata)=x;
				(plt->ydata)=yb;
				xi=g_array_index(x, gdouble, 0);
				xf=g_array_index(x, gdouble, (lc-1));
				plot_linear_update_scale_pretty(plot1, xi, xf, mny, mxy);
			}
			else
			{
				str=g_strdup_printf(_("Loading failed for file: %s, Error: %s"), fin, (gchar *) Err);
				gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
				g_free(str);
				g_error_free(Err);
			}
			g_free(contents);
			g_free(fin);
		}
		gtk_widget_destroy(wfile);
	}
}

void static upj(GtkWidget *widget, gpointer data)
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

void static upk(GtkWidget *widget, gpointer data)
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

void static upa2(GtkWidget *widget, gpointer data)
{
	gdouble *ptr;

	ptr=&g_array_index((GArray*)data, gdouble, jdim+(kdim*jdimx));
	*ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
}

void static upa1(GtkWidget *widget, gpointer data)
{
	gdouble *ptr;

	ptr=&g_array_index((GArray*)data, gdouble, jdim);
	*ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
}

void static reset(GtkWidget *widget, gpointer data)
{
	gdouble num;

	jdim=0;
	jdimx=0;
	kdim=0;
	kdimx=0;
	num=0;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind), 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(kind), 0);
	num=g_array_index(bsra, gdouble, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(bsr), num);
	g_array_free(bsra, TRUE);
	bsra=g_array_new(FALSE, FALSE, sizeof(gdouble));
	g_array_append_val(bsra, num);
	num=g_array_index(bspa, gdouble, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(bsp), num);
	g_array_free(bspa, TRUE);
	bspa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	g_array_append_val(bspa, num);
	num=g_array_index(zwa, gdouble, 0);
	g_array_free(zwa, TRUE);
	zwa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	g_array_append_val(zwa, num);
	num=g_array_index(isra, gdouble, 0);
	g_array_free(isra, TRUE);
	isra=g_array_new(FALSE, FALSE, sizeof(gdouble));
	g_array_append_val(isra, num);
	num=g_array_index(ispa, gdouble, 0);
	g_array_free(ispa, TRUE);
	ispa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	g_array_append_val(ispa, num);
	num=g_array_index(tca, gdouble, 0);
	g_array_free(tca, TRUE);
	tca=g_array_new(FALSE, FALSE, sizeof(gdouble));
	g_array_append_val(tca, num);
	num=g_array_index(twa, gdouble, 0);
	g_array_free(twa, TRUE);
	twa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	g_array_append_val(twa, num);
}

void static reset2(GtkWidget *widget, gpointer data)
{
	gdouble num, num2, num3, num4, num5;
	guint j;

	jdim=0;
	kdim=0;
	kdimx=0;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind2), 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(kind), 0);
	num=g_array_index(isra, gdouble, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(isr), num);
	g_array_free(isra, TRUE);
	isra=g_array_new(FALSE, FALSE, sizeof(gdouble));
	num2=g_array_index(ispa, gdouble, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(isp), num2);
	g_array_free(ispa, TRUE);
	ispa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	num3=g_array_index(tca, gdouble, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(tc), num3);
	g_array_free(tca, TRUE);
	tca=g_array_new(FALSE, FALSE, sizeof(gdouble));
	num4=g_array_index(twa, gdouble, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(tw), num4);
	g_array_free(twa, TRUE);
	twa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	num5=g_array_index(zwa, gdouble, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(zw), num5);
	g_array_free(zwa, TRUE);
	zwa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	for (j=0; j<=jdimx; j++)
	{
		g_array_append_val(isra, num);
		g_array_append_val(ispa, num2);
		g_array_append_val(tca, num3);
		g_array_append_val(twa, num4);
		g_array_append_val(zwa, num5);
	}
}

int main( int argc, char *argv[])
{
	GtkAdjustment *adj;
	GtkWidget *vbox, *mnb, *mnu, *smnu, *mni, *hpane, *table, *label, *butt;
	GtkAccelGroup *accel_group=NULL;
	GSList *group=NULL, *group3=NULL;
	gdouble fll=0;

	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE);
	if (!g_thread_supported()) g_thread_init(NULL);
	gdk_threads_init();
	gdk_threads_enter();
	gtk_init(&argc, &argv);
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), _("Harmonic Spectrum Analyser"));
	g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show(vbox);
	mnb=gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(vbox), mnb, FALSE, FALSE, 2);
	gtk_widget_show(mnb);
	mnu=gtk_menu_new();
	accel_group=gtk_accel_group_new();
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
	mni=gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(opd), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE, NULL);
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(sav), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_image_menu_item_new_from_stock(GTK_STOCK_PRINT, NULL);
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_p, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(prt), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accel_group);
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(gtk_main_quit), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_menu_item_new_with_mnemonic(_("_File"));
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), mnu);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnb), mni);
	mnu=gtk_menu_new();
	smnu=gtk_menu_new();
	agosa=gtk_radio_menu_item_new_with_label(group, "Agilent OSA");
	group=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(agosa));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), agosa);
	gtk_widget_show(agosa);
	agtl=gtk_radio_menu_item_new_with_label(group, "Agilent Tunable Laser");
	group=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(agtl));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), agtl);
	gtk_widget_show(agtl);
	anosa=gtk_radio_menu_item_new_with_label(group, "Ando OSA");
	group=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(anosa));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(anosa), TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), anosa);
	gtk_widget_show(anosa);
	sws=gtk_radio_menu_item_new_with_label(group, "JDSUniphase Swept Wavelength System");
	group=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(sws));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), sws);
	gtk_widget_show(sws);
	dlm=gtk_radio_menu_item_new_with_label(group, _("Raw Delimited Data"));
	group=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(dlm));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), dlm);
	gtk_widget_show(dlm);
	mni=gtk_menu_item_new_with_label(_("Data Format:"));
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), smnu);
	tracmenu=gtk_menu_new();
	trac=gtk_menu_item_new_with_label(_("Trace:"));
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), trac);
	gtk_widget_show(trac);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(trac), tracmenu);
	trans=gtk_check_menu_item_new_with_label(_("Transmission Measurement?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(trans), TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), trans);
	gtk_widget_show(trans);
	dBs=gtk_check_menu_item_new_with_label(_("Data in dBs?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(dBs), TRUE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), dBs);
	gtk_widget_show(dBs);
	neg=gtk_check_menu_item_new_with_label(_("Negate?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(neg), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), neg);
	gtk_widget_show(neg);
	mni=gtk_menu_item_new_with_label(_("Display Properties:"));
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_F2, 0, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(dpr), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_menu_item_new_with_mnemonic(_("_Properties"));
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), mnu);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnb), mni);
	mnu=gtk_menu_new();
	smnu=gtk_menu_new();
	ncmp=gtk_radio_menu_item_new_with_label(group3, _("None"));
	group3=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(ncmp));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), ncmp);
	gtk_widget_show(ncmp);
	lcmp=gtk_radio_menu_item_new_with_label(group3, _("1st order\nspectral shadowing"));
	group3=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(lcmp));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), lcmp);
	gtk_widget_show(lcmp);
	mni=gtk_menu_item_new_with_label(_("Nonlinear\nCompensation:"));
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), smnu);
	bat=gtk_check_menu_item_new_with_label(_("Batch Process Data?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bat), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), bat);
	gtk_widget_show(bat);
	twopionx=gtk_check_menu_item_new_with_label(_("Invert domain?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(twopionx), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), twopionx);
	gtk_widget_show(twopionx);
	chi=gtk_check_menu_item_new_with_label(_("Calculate Chirp?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(chi), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), chi);
	gtk_widget_show(chi);
	opttri=gtk_check_menu_item_new_with_label(_("Optimise Triangle fit?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(opttri), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), opttri);
	gtk_widget_show(opttri);
	mni=gtk_menu_item_new_with_mnemonic(_("_Advanced"));
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), mnu);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnb), mni);
	mnu=gtk_menu_new();
	mni=gtk_menu_item_new_with_label(_("Instructions"));
	gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_F1, 0, GTK_ACCEL_VISIBLE);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(help), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(about), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_menu_item_new_with_mnemonic(_("_Help"));
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), mnu);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnb), mni);
	gtk_menu_item_right_justify(GTK_MENU_ITEM(mni));
	bsra=g_array_new(FALSE, FALSE, sizeof(gdouble));
	bspa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	isra=g_array_new(FALSE, FALSE, sizeof(gdouble));
	ispa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	tca=g_array_new(FALSE, FALSE, sizeof(gdouble));
	twa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	zwa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	g_array_append_val(bsra, fll);
	g_array_append_val(bspa, fll);
	g_array_append_val(isra, fll);
	g_array_append_val(ispa, fll);
	g_array_append_val(tca, fll);
	g_array_append_val(twa, fll);
	g_array_append_val(zwa, fll);
	hpane=gtk_hpaned_new();
	gtk_widget_show(hpane);
	gtk_container_add(GTK_CONTAINER(vbox), hpane);
	notebook=gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
	table=gtk_table_new(5, 3, FALSE);
	gtk_widget_show(table);
	label=gtk_label_new(_("Spectrum Start:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(0, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	bsr=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(bsr), "value-changed", G_CALLBACK(upa1), (gpointer) bsra);
	gtk_table_attach(GTK_TABLE(table), bsr, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(bsr);
	label=gtk_label_new(_("Spectrum Stop:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(1, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	bsp=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(bsp), "value-changed", G_CALLBACK(upa2), (gpointer) bspa);
	gtk_table_attach(GTK_TABLE(table), bsp, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(bsp);
	label=gtk_label_new(_("Offset:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(1, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	fst=gtk_spin_button_new(adj, 0.5, 2);
	gtk_table_attach(GTK_TABLE(table), fst, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(fst);
	label=gtk_label_new(_("j index:"));
	gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, G_MAXINT8, 1.0, 5.0, 0.0);
	jind=gtk_spin_button_new(adj, 0, 0);
	g_signal_connect(G_OBJECT(jind), "value-changed", G_CALLBACK(upj), NULL);
	gtk_table_attach(GTK_TABLE(table), jind, 2, 3, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(jind);
	label=gtk_label_new(_("Zero Padding 2^:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(12, 0, 31, 1.0, 5.0, 0.0);
	zpd=gtk_spin_button_new(adj, 0, 0);
	gtk_table_attach(GTK_TABLE(table), zpd, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(zpd);
	butt=gtk_button_new_with_label(_("Reset\nArrays"));
	g_signal_connect(G_OBJECT(butt), "clicked", G_CALLBACK(reset), NULL);
	gtk_table_attach(GTK_TABLE(table), butt, 2, 3, 2, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(butt);
	tr=gtk_button_new_with_label(_("Transform Spectrum"));
	gtk_table_attach(GTK_TABLE(table), tr, 0, 3, 4, 5, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(tr);
	label=gtk_label_new(_("Spectrum"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table, label);
	table=gtk_table_new(6, 3, FALSE);
	gtk_widget_show(table);
	label=gtk_label_new(_("Inverse Spectrum Start:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(0, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	isr=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(isr), "value-changed", G_CALLBACK(upa2), (gpointer) isra);
	gtk_table_attach(GTK_TABLE(table), isr, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(isr);
	label=gtk_label_new(_("Inverse Spectrum Stop:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(1, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	isp=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(isp), "value-changed", G_CALLBACK(upa2), (gpointer) ispa);
	gtk_table_attach(GTK_TABLE(table), isp, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(isp);
	label=gtk_label_new(_("Triangle Centre:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(0, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	tc=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(tc), "value-changed", G_CALLBACK(upa2), (gpointer) tca);
	gtk_table_attach(GTK_TABLE(table), tc, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(tc);
	label=gtk_label_new(_("Triangle Full Width:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(2, DZE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	tw=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(tw), "value-changed", G_CALLBACK(upa2), (gpointer) twa);
	gtk_table_attach(GTK_TABLE(table), tw, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(tw);
	label=gtk_label_new(_("DC Peak Width:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 4, 5, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(2, DZE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	zw=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(zw), "value-changed", G_CALLBACK(upa1), (gpointer) zwa);
	gtk_table_attach(GTK_TABLE(table), zw, 1, 2, 5, 6, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(zw);
	label=gtk_label_new(_("j index:"));
	gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, 0, 1.0, 5.0, 0.0);
	jind2=gtk_spin_button_new(adj, 0, 0);
	g_signal_connect(G_OBJECT(jind2), "value-changed", G_CALLBACK(upj), NULL);
	gtk_table_attach(GTK_TABLE(table), jind2, 2, 3, 1, 2, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 2, 2);
	gtk_widget_show(jind2);
	label=gtk_label_new(_("k index:"));
	gtk_table_attach(GTK_TABLE(table), label, 2, 3, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, 0, 1.0, 5.0, 0.0);
	kind=gtk_spin_button_new(adj, 0, 0);
	g_signal_connect(G_OBJECT(kind), "value-changed", G_CALLBACK(upk), NULL);
	gtk_table_attach(GTK_TABLE(table), kind, 2, 3, 3, 4, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 2, 2);
	gtk_widget_show(kind);
	butt=gtk_button_new_with_label(_("Reset\nArrays"));
	g_signal_connect(G_OBJECT(butt), "clicked", G_CALLBACK(reset2), NULL);
	gtk_table_attach(GTK_TABLE(table), butt, 2, 3, 4, 6, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(butt);
	pr=gtk_button_new_with_label(_("Process\nSpectrum"));
	gtk_table_attach(GTK_TABLE(table), pr, 0, 1, 4, 6, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(pr);
	label=gtk_label_new(_("Inverse Spectrum"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table, label);
	gtk_widget_show(notebook);
	gtk_paned_add1(GTK_PANED(hpane), notebook);
	notebook2=gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook2), GTK_POS_TOP);
	table=gtk_table_new(1, 1, FALSE);
	gtk_widget_show(table);
	plot1=plot_linear_new();
	g_signal_connect(plot1, "moved", G_CALLBACK(pltmv), NULL);
	gtk_widget_show(plot1);
	gtk_table_attach(GTK_TABLE(table), plot1, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK |GTK_EXPAND, 2, 2);
	label=gtk_label_new(_("Spectrum"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), table, label);
	table=gtk_table_new(1, 1, FALSE);
	gtk_widget_show(table);
	plot2=plot_linear_new();
	((PLOT_LINEAR(plot2))->xlab)=g_strdup(_("Inverse Domain"));
	g_signal_connect(plot2, "moved", G_CALLBACK(pltmv), NULL);
	gtk_widget_show(plot2);
	gtk_table_attach(GTK_TABLE(table), plot2, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	label=gtk_label_new(_("Inverse Spectrum"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), table, label);
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
	gtk_widget_show(notebook2);
	gtk_paned_add2(GTK_PANED(hpane), notebook2);
	statusbar=gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 2);
	gtk_widget_show(statusbar);
	x=g_array_new(FALSE, FALSE, sizeof(gdouble));
	yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
	specs=g_array_new(FALSE, FALSE, sizeof(gdouble));
	stars=g_array_new(FALSE, FALSE, sizeof(gdouble));
	xsb=g_array_new(FALSE, FALSE, sizeof(gdouble));
	ysb=g_array_new(FALSE, FALSE, sizeof(gdouble));
	delf=g_array_new(FALSE, FALSE, sizeof(gdouble));
	vis=g_array_new(FALSE, FALSE, sizeof(gdouble));
	doms=g_array_new(FALSE, FALSE, sizeof(gdouble));
	chp=g_array_new(FALSE, FALSE, sizeof(gdouble));
	msr=g_array_new(FALSE, FALSE, sizeof(gdouble));
	gtk_widget_show(window);
	gtk_main();
	gdk_threads_leave();
	return 0;
}
