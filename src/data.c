/***************************************************************************
 *            data.c
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

#include "data.h"

void prt(GtkWidget *widget, gpointer data)
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

void sav(GtkWidget *widget, gpointer data)
{
	GtkWidget *wfile, *dialog, *cont, *label;
	PlotLinear *plt;
	gchar *contents, *str, *str2, *fout=NULL;
	gchar s1[10], s2[10], s3[10], s4[10];
	gint j, k, sz4;
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
				if (jdimxf==1)
				{
					if (kdimxf==1)
					{
						dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Visibility"), 1, _("Domain Shift"), 2, _("Chirp"), 3, _("All"), 4, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
						cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
						label=gtk_label_new(_("Select Parameter to save:"));
						gtk_container_add(GTK_CONTAINER(cont), label);
						switch (gtk_dialog_run(GTK_DIALOG(dialog)))
						{
							case 4:
							contents=g_strdup(_("MEAS     \tVISIBILTY\tDOMN_SHFT\tCHIRP    "));
							for (j=0; j<mx; j++)
							{
								g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, j));
								g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, j));
								g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, j));
								str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
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
								str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								g_error_free(Err);
							}
							break;
							case 3:
							contents=g_strdup(_("MEAS     \tCHIRP    "));
							for (j=0; j<mx; j++)
							{
								g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
								g_snprintf(s2, 10, "%f", g_array_index(chp, gdouble, j));
								str2=g_strjoin("\t", s1, s2, NULL);
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
								str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								g_error_free(Err);
							}
							break;
							case 2:
							contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
							for (j=0; j<mx; j++)
							{
								g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
								g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, j));
								str2=g_strjoin("\t", s1, s2, NULL);
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
								str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								g_error_free(Err);
							}
							break;
							case 1:
							contents=g_strdup(_("MEAS     \tVISIBILTY"));
							for (j=0; j<mx; j++)
							{
								g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, j));
								str2=g_strjoin("\t", s1, s2, NULL);
								str=g_strdup(contents);
								g_free(contents);
								contents=g_strjoin(DLMT, str, str2, NULL);
								{g_free(str); g_free(str2);}
							}
							g_file_set_contents(fout, contents, -1, &Err);
							g_free(contents);
							if (Err)
							{
								str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								g_error_free(Err);
							}
							break;
							default:
							break;
						}
					}
					else
					{
						dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Visibility"), 1, _("Domain Shift"), 2, _("Chirp"), 3, _("All"), 4, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
						cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
						label=gtk_label_new(_("Select Parameter to save:"));
						gtk_container_add(GTK_CONTAINER(cont), label);
						switch (gtk_dialog_run(GTK_DIALOG(dialog)))
						{
							case 4:
							contents=g_strdup(_("MEAS     \tVISIBILTY\tDOMN_SHFT\tCHIRP    "));
							str2=g_strdup(contents);
							for (k=1; k<kdimxf; k++)
							{
								str=g_strjoin("\t", contents, str2, NULL);
								g_free(contents);
								contents=g_strdup(str);
								g_free(str);
							}
							g_free(str2);
							for (j=0; j<mx; j++)
							{
								g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (jdim+(j*jdimxf))*kdimxf));
								g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, (jdim+(j*jdimxf))*kdimxf));
								g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, (jdim+(j*jdimxf))*kdimxf));
								str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
								k=1;
								while (k<kdimxf)
								{
									g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
									g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
									g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
									str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
									g_free(str2);
									str2=g_strdup(str);
									g_free(str);
									k++;
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
								str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								g_error_free(Err);
							}
							break;
							case 3:
							contents=g_strdup(_("MEAS     \tCHIRP    "));
							str2=g_strdup(contents);
							for (k=1; k<kdimxf; k++)
							{
								str=g_strjoin("\t", contents, str2, NULL);
								g_free(contents);
								contents=g_strdup(str);
								g_free(str);
							}
							g_free(str2);
							for (j=0; j<mx; j++)
							{
								g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
								g_snprintf(s2, 10, "%f", g_array_index(chp, gdouble, (jdim+(j*jdimxf))*kdimxf));
								str2=g_strjoin("\t", s1, s2, NULL);
								k=1;
								while (k<kdimxf)
								{
									g_snprintf(s2, 10, "%f", g_array_index(chp, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
									str=g_strjoin("\t", str2, s1, s2, NULL);
									g_free(str2);
									str2=g_strdup(str);
									g_free(str);
									k++;
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
								str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								g_error_free(Err);
							}
							break;
							case 2:
							contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
							str2=g_strdup(contents);
							for (k=1; k<kdimxf; k++)
							{
								str=g_strjoin("\t", contents, str2, NULL);
								g_free(contents);
								contents=g_strdup(str);
								g_free(str);
							}
							g_free(str2);
							for (j=0; j<mx; j++)
							{
								g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
								g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, (jdim+(j*jdimxf))*kdimxf));
								str2=g_strjoin("\t", s1, s2, NULL);
								k=1;
								while (k<kdimxf)
								{
									g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
									str=g_strjoin("\t", str2, s1, s2, NULL);
									g_free(str2);
									str2=g_strdup(str);
									g_free(str);
									k++;
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
								str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								g_error_free(Err);
							}
							break;
							case 1:
							contents=g_strdup(_("MEAS     \tVISIBILTY"));
							str2=g_strdup(contents);
							for (k=1; k<kdimxf; k++)
							{
								str=g_strjoin("\t", contents, str2, NULL);
								g_free(contents);
								contents=g_strdup(str);
								g_free(str);
							}
							g_free(str2);
							for (j=0; j<mx; j++)
							{
								g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (jdim+(j*jdimxf))*kdimxf));
								str2=g_strjoin("\t", s1, s2, NULL);
								k=1;
								while (k<kdimxf)
								{
									g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
									str=g_strjoin("\t", str2, s1, s2, NULL);
									g_free(str2);
									str2=g_strdup(str);
									g_free(str);
									k++;
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
								str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								g_error_free(Err);
							}
							break;
							default:
							break;
						}
					}
				}
				else if (kdimxf==1)
				{
					dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Visibility"), 1, _("Domain Shift"), 2, _("Chirp"), 3, _("All"), 4, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
					cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
					label=gtk_label_new(_("Select Parameter to save:"));
					gtk_container_add(GTK_CONTAINER(cont), label);
					switch (gtk_dialog_run(GTK_DIALOG(dialog)))
					{
						case 4:
						contents=g_strdup(_("MEAS     \tVISIBILTY\tDOMN_SHFT\tCHIRP    "));
						str2=g_strdup(contents);
						for (j=1; j<jdimxf; j++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (k=0; k<mx; k++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, k));
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (k*jdimxf*kdimxf)+kdim));
							g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, (k*jdimxf*kdimxf)+kdim));
							g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, (k*jdimxf*kdimxf)+kdim));
							str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
							j=1;
							while (j<jdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								j++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 3:
						contents=g_strdup(_("MEAS     \tCHIRP    "));
						str2=g_strdup(contents);
						for (j=1; j<jdimxf; j++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (k=0; k<mx; k++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, k));
							g_snprintf(s2, 10, "%f", g_array_index(chp, gdouble, (k*jdimxf*kdimxf)+kdim));
							str2=g_strjoin("\t", s1, s2, NULL);
							j=1;
							while (j<jdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(chp, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								str=g_strjoin("\t", str2, s1, s2, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								j++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 2:
						contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
						str2=g_strdup(contents);
						for (j=1; j<jdimxf; j++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (k=0; k<mx; k++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, k));
							g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, (k*jdimxf*kdimxf)+kdim));
							str2=g_strjoin("\t", s1, s2, NULL);
							j=1;
							while (j<jdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								str=g_strjoin("\t", str2, s1, s2, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								j++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 1:
						contents=g_strdup(_("MEAS     \tVISIBILTY"));
						str2=g_strdup(contents);
						for (j=1; j<jdimxf; j++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (k=0; k<mx; k++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, k));
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (k*jdimxf*kdimxf)+kdim));
							str2=g_strjoin("\t", s1, s2, NULL);
							j=1;
							while (j<jdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								str=g_strjoin("\t", str2, s1, s2, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								j++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						default:
						break;
					}
				}
				else
				{
					dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Visibility\nover J"), 1, _("Domain Shift\nover J"), 2, _("Chirp\nover J"), 3, _("All\nover J"), 4, _("Visibility\nover K"), 5, _("Domain Shift\nover K"), 6, _("Chirp\nover K"), 7, _("All\nover K"), 8, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
					cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
					label=gtk_label_new(_("Select Parameter to save:"));
					gtk_container_add(GTK_CONTAINER(cont), label);
					switch (gtk_dialog_run(GTK_DIALOG(dialog)))
					{
						case 8:
						contents=g_strdup(_("MEAS     \tVISIBILTY\tDOMN_SHFT\tCHIRP    "));
						str2=g_strdup(contents);
						for (k=1; k<kdimxf; k++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (j=0; j<mx; j++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (jdim+(j*jdimxf))*kdimxf));
							g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, (jdim+(j*jdimxf))*kdimxf));
							g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, (jdim+(j*jdimxf))*kdimxf));
							str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
							k=1;
							while (k<kdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
								g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
								g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
								str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								k++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 7:
						contents=g_strdup(_("MEAS     \tCHIRP    "));
						str2=g_strdup(contents);
						for (k=1; k<kdimxf; k++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (j=0; j<mx; j++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
							g_snprintf(s2, 10, "%f", g_array_index(chp, gdouble, (jdim+(j*jdimxf))*kdimxf));
							str2=g_strjoin("\t", s1, s2, NULL);
							k=1;
							while (k<kdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(chp, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
								str=g_strjoin("\t", str2, s1, s2, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								k++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 6:
						contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
						str2=g_strdup(contents);
						for (k=1; k<kdimxf; k++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (j=0; j<mx; j++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
							g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, (jdim+(j*jdimxf))*kdimxf));
							str2=g_strjoin("\t", s1, s2, NULL);
							k=1;
							while (k<kdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
								str=g_strjoin("\t", str2, s1, s2, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								k++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 5:
						contents=g_strdup(_("MEAS     \tVISIBILTY"));
						str2=g_strdup(contents);
						for (k=1; k<kdimxf; k++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (j=0; j<mx; j++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (jdim+(j*jdimxf))*kdimxf));
							str2=g_strjoin("\t", s1, s2, NULL);
							k=1;
							while (k<kdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
								str=g_strjoin("\t", str2, s1, s2, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								k++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 4:
						contents=g_strdup(_("MEAS     \tVISIBILTY\tDOMN_SHFT\tCHIRP    "));
						str2=g_strdup(contents);
						for (j=1; j<jdimxf; j++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (k=0; k<mx; k++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, k));
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (k*jdimxf*kdimxf)+kdim));
							g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, (k*jdimxf*kdimxf)+kdim));
							g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, (k*jdimxf*kdimxf)+kdim));
							str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
							j=1;
							while (j<jdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								j++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 3:
						contents=g_strdup(_("MEAS     \tCHIRP    "));
						str2=g_strdup(contents);
						for (j=1; j<jdimxf; j++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (k=0; k<mx; k++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, k));
							g_snprintf(s2, 10, "%f", g_array_index(chp, gdouble, (k*jdimxf*kdimxf)+kdim));
							str2=g_strjoin("\t", s1, s2, NULL);
							j=1;
							while (j<jdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(chp, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								str=g_strjoin("\t", str2, s1, s2, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								j++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 2:
						contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
						str2=g_strdup(contents);
						for (j=1; j<jdimxf; j++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (k=0; k<mx; k++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, k));
							g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, (k*jdimxf*kdimxf)+kdim));
							str2=g_strjoin("\t", s1, s2, NULL);
							j=1;
							while (j<jdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								str=g_strjoin("\t", str2, s1, s2, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								j++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 1:
						contents=g_strdup(_("MEAS     \tVISIBILTY"));
						str2=g_strdup(contents);
						for (j=1; j<jdimxf; j++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (k=0; k<mx; k++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, k));
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (k*jdimxf*kdimxf)+kdim));
							str2=g_strjoin("\t", s1, s2, NULL);
							j=1;
							while (j<jdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								str=g_strjoin("\t", str2, s1, s2, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								j++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						default:
						break;
					}
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
				if (jdimxf==1)
				{
					if (kdimxf==1)
					{
						dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Visibility"), 1, _("Domain Shift"), 2, _("Both"), 3, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
						cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
						label=gtk_label_new(_("Select Parameter to save:"));
						gtk_container_add(GTK_CONTAINER(cont), label);
						switch (gtk_dialog_run(GTK_DIALOG(dialog)))
						{
							case 3:
							contents=g_strdup(_("MEAS     \tVISIBILTY\tDOMN_SHFT"));
							for (j=0; j<mx; j++)
							{
								g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, j));
								g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, j));
								str2=g_strjoin("\t", s1, s2, s3, NULL);
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
								str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								g_error_free(Err);
							}
							break;
							case 2:
							contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
							for (j=0; j<mx; j++)
							{
								g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
								g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, j));
								str2=g_strjoin("\t", s1, s2, NULL);
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
								str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								g_error_free(Err);
							}
							break;
							case 1:
							contents=g_strdup(_("MEAS     \tVISIBILTY"));
							for (j=0; j<mx; j++)
							{
								g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, j));
								str2=g_strjoin("\t", s1, s2, NULL);
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
								str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								g_error_free(Err);
							}
							break;
							default:
							break;
						}
					}
					else
					{
						dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Visibility"), 1, _("Domain Shift"), 2, _("Both"), 3, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
						cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
						label=gtk_label_new(_("Select Parameter to save:"));
						gtk_container_add(GTK_CONTAINER(cont), label);
						switch (gtk_dialog_run(GTK_DIALOG(dialog)))
						{
							case 3:
							contents=g_strdup(_("MEAS     \tVISIBILTY\tDOMN_SHFT"));
							str2=g_strdup(contents);
							for (k=1; k<kdimxf; k++)
							{
								str=g_strjoin("\t", contents, str2, NULL);
								g_free(contents);
								contents=g_strdup(str);
								g_free(str);
							}
							g_free(str2);
							for (j=0; j<mx; j++)
							{
								g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (jdim+(j*jdimxf))*kdimxf));
								g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, (jdim+(j*jdimxf))*kdimxf));
								str2=g_strjoin("\t", s1, s2, s3, NULL);
								k=1;
								while (k<kdimxf)
								{
									g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
									g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
									str=g_strjoin("\t", str2, s1, s2, s3, NULL);
									g_free(str2);
									str2=g_strdup(str);
									g_free(str);
									k++;
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
								str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								g_error_free(Err);
							}
							break;
							case 2:
							contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
							str2=g_strdup(contents);
							for (k=1; k<kdimxf; k++)
							{
								str=g_strjoin("\t", contents, str2, NULL);
								g_free(contents);
								contents=g_strdup(str);
								g_free(str);
							}
							g_free(str2);
							for (j=0; j<mx; j++)
							{
								g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
								g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, (jdim+(j*jdimxf))*kdimxf));
								str2=g_strjoin("\t", s1, s2, NULL);
								k=1;
								while (k<kdimxf)
								{
									g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
									str=g_strjoin("\t", str2, s1, s2, NULL);
									g_free(str2);
									str2=g_strdup(str);
									g_free(str);
									k++;
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
								str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								g_error_free(Err);
							}
							break;
							case 1:
							contents=g_strdup(_("MEAS     \tVISIBILTY"));
							str2=g_strdup(contents);
							for (k=1; k<kdimxf; k++)
							{
								str=g_strjoin("\t", contents, str2, NULL);
								g_free(contents);
								contents=g_strdup(str);
								g_free(str);
							}
							g_free(str2);
							for (j=0; j<mx; j++)
							{
								g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (jdim+(j*jdimxf))*kdimxf));
								str2=g_strjoin("\t", s1, s2, NULL);
								k=1;
								while (k<kdimxf)
								{
									g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
									str=g_strjoin("\t", str2, s1, s2, NULL);
									g_free(str2);
									str2=g_strdup(str);
									g_free(str);
									k++;
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
								str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
								gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
								g_free(str);
								g_error_free(Err);
							}
							break;
							default:
							break;
						}
					}
				}
				else if (kdimxf==1)
				{
					dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Visibility"), 1, _("Domain Shift"), 2, _("Both"), 3, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
					cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
					label=gtk_label_new(_("Select Parameter to save:"));
					gtk_container_add(GTK_CONTAINER(cont), label);
					switch (gtk_dialog_run(GTK_DIALOG(dialog)))
					{
						case 3:
						contents=g_strdup(_("MEAS     \tVISIBILTY\tDOMN_SHFT"));
						str2=g_strdup(contents);
						for (j=1; j<jdimxf; j++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (k=0; k<mx; k++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, k));
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (k*jdimxf*kdimxf)+kdim));
							g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, (k*jdimxf*kdimxf)+kdim));
							str2=g_strjoin("\t", s1, s2, s3, NULL);
							j=1;
							while (j<jdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								str=g_strjoin("\t", str2, s1, s2, s3, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								j++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 2:
						contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
						str2=g_strdup(contents);
						for (j=1; j<jdimxf; j++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (k=0; k<mx; k++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, k));
							g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, (k*jdimxf*kdimxf)+kdim));
							str2=g_strjoin("\t", s1, s2, NULL);
							j=1;
							while (j<jdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								str=g_strjoin("\t", str2, s1, s2, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								j++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 1:
						contents=g_strdup(_("MEAS     \tVISIBILTY"));
						str2=g_strdup(contents);
						for (j=1; j<jdimxf; j++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (k=0; k<mx; k++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, k));
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (k*jdimxf*kdimxf)+kdim));
							str2=g_strjoin("\t", s1, s2, NULL);
							j=1;
							while (j<jdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								str=g_strjoin("\t", str2, s1, s2, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								j++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						default:
						break;
					}
				}
				else
				{
					dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Visibility\nover J"), 1, _("Domain Shift\nover J"), 2, _("Both\nover J"), 3, _("Visibility\nover K"), 4, _("Domain Shift\nover K"), 5, _("Both\nover K"), 6, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
					cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
					label=gtk_label_new(_("Select Parameter to save:"));
					gtk_container_add(GTK_CONTAINER(cont), label);
					switch (gtk_dialog_run(GTK_DIALOG(dialog)))
					{
						case 6:
						contents=g_strdup(_("MEAS     \tVISIBILTY\tDOMN_SHFT"));
						str2=g_strdup(contents);
						for (k=1; k<kdimxf; k++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (j=0; j<mx; j++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (jdim+(j*jdimxf))*kdimxf));
							g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, (jdim+(j*jdimxf))*kdimxf));
							str2=g_strjoin("\t", s1, s2, s3, NULL);
							k=1;
							while (k<kdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
								g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
								str=g_strjoin("\t", str2, s1, s2, s3, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								k++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 5:
						contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
						str2=g_strdup(contents);
						for (k=1; k<kdimxf; k++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (j=0; j<mx; j++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
							g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, (jdim+(j*jdimxf))*kdimxf));
							str2=g_strjoin("\t", s1, s2, NULL);
							k=1;
							while (k<kdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
								str=g_strjoin("\t", str2, s1, s2, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								k++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 4:
						contents=g_strdup(_("MEAS     \tVISIBILTY"));
						str2=g_strdup(contents);
						for (k=1; k<kdimxf; k++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (j=0; j<mx; j++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, j));
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (jdim+(j*jdimxf))*kdimxf));
							str2=g_strjoin("\t", s1, s2, NULL);
							k=1;
							while (k<kdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((jdim+(j*jdimxf))*kdimxf)+k));
								str=g_strjoin("\t", str2, s1, s2, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								k++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 3:
						contents=g_strdup(_("MEAS     \tVISIBILTY\tDOMN_SHFT"));
						str2=g_strdup(contents);
						for (j=1; j<jdimxf; j++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (k=0; k<mx; k++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, k));
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (k*jdimxf*kdimxf)+kdim));
							g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, (k*jdimxf*kdimxf)+kdim));
							str2=g_strjoin("\t", s1, s2, s3, NULL);
							j=1;
							while (j<jdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								str=g_strjoin("\t", str2, s1, s2, s3, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								j++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 2:
						contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
						str2=g_strdup(contents);
						for (j=1; j<jdimxf; j++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (k=0; k<mx; k++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, k));
							g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, (k*jdimxf*kdimxf)+kdim));
							str2=g_strjoin("\t", s1, s2, NULL);
							j=1;
							while (j<jdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(doms, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								str=g_strjoin("\t", str2, s1, s2, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								j++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						case 1:
						contents=g_strdup(_("MEAS     \tVISIBILTY"));
						str2=g_strdup(contents);
						for (j=1; j<jdimxf; j++)
						{
							str=g_strjoin("\t", contents, str2, NULL);
							g_free(contents);
							contents=g_strdup(str);
							g_free(str);
						}
						g_free(str2);
						for (k=0; k<mx; k++)
						{
							g_snprintf(s1, 10, "%f", g_array_index(msr, gdouble, k));
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (k*jdimxf*kdimxf)+kdim));
							str2=g_strjoin("\t", s1, s2, NULL);
							j=1;
							while (j<jdimxf)
							{
								g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, ((j+(k*jdimxf))*kdimxf)+kdim));
								str=g_strjoin("\t", str2, s1, s2, NULL);
								g_free(str2);
								str2=g_strdup(str);
								g_free(str);
								j++;
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						break;
						default:
						break;
					}
				}
				gtk_widget_destroy(dialog);
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&20)==20)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				if (jdimxf==1)
				{
					if (kdimxf==1)
					{
						str=g_strdup(_("VISIBILTY\tDOMN_SHFT\tCHIRP    "));
						g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, 0));
						g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, 0));
						g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, 0));
						str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
						contents=g_strjoin(DLMT, str, str2, NULL);
						g_free(str);
						g_free(str2);
						g_file_set_contents(fout, contents, -1, &Err);
						g_free(contents);
						if (Err)
						{
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
					}
					else
					{
						contents=g_strdup(_("K        \tVISIBILTY\tDOMN_SHFT\tCHIRP    "));
						for (k=0; k<kdimxf; k++)
						{
							g_snprintf(s1, 10, "%d", k);
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, k));
							g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, k));
							g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, k));
							str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
					}
				}
				else
				{
					str2=g_strdup(_("J        \tVISIBILTY\tDOMN_SHFT\tCHIRP    "));
					contents=g_strdup(str2);
					for (k=1; k<kdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					for (j=0; j<jdimxf; j++)
					{
						g_snprintf(s1, 10, "%d", j);
						g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, j*kdimxf));
						g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, j*kdimxf));
						g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, j*kdimxf));
						str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
						k=1;
						while (k<kdimxf)
						{
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (j*kdimxf)+k));
							g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, (j*kdimxf)+k));
							g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, (j*kdimxf)+k));
							str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
							k++;
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
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
				}
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&4)!=0)
		{
			wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
			g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
			gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				if (jdimxf==1)
				{
					if (kdimxf==1)
					{
						str=g_strdup(_("VISIBILTY\tDOMN_SHFT"));
						g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, 0));
						g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, 0));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						contents=g_strjoin(DLMT, str, str2, NULL);
						g_free(str);
						g_free(str2);
						g_file_set_contents(fout, contents, -1, &Err);
						g_free(contents);
						if (Err)
						{
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
					}
					else
					{
						contents=g_strdup(_("K        \tVISIBILTY\tDOMN_SHFT"));
						for (k=0; k<kdimxf; k++)
						{
							g_snprintf(s1, 10, "%d", k);
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, k));
							g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, k));
							str2=g_strjoin("\t", s1, s2, s3, NULL);
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
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
					}
				}
				else
				{
					str2=g_strdup(_("J        \tVISIBILTY\tDOMN_SHFT"));
					contents=g_strdup(str2);
					for (k=1; k<kdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					for (j=0; j<jdimxf; j++)
					{
						g_snprintf(s1, 10, "%d", j);
						g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, j*kdimxf));
						g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, j*kdimxf));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<kdimxf)
						{
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, (j*kdimxf)+k));
							g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, (j*kdimxf)+k));
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
							k++;
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
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
				}
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
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Real/Imaginary"), 1, _("Magnitude/Phase"), 2, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 1:
					str2=g_strdup(_("INVERSE_D\tREAL_VAL \tIMAG_VAL "));
					contents=g_strdup(str2);
					for (k=1; k<jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz4=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz4));
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
					for (j=1; j<sz4; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, j));
						g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*sz4)-j));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, (2*k*sz4)+j));
							g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*(++k)*sz4)-j));
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
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					g_free(fout);
					break;
					case 2:
					str2=g_strdup(_("INVERSE_D\tMAGNITUDE\tPHASE    "));
					contents=g_strdup(str2);
					for (k=1; k<jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz4=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz4));
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
					for (j=1; j<sz4; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						num=g_array_index(stars, gdouble, j);
						num2=g_array_index(stars, gdouble, (2*sz4)-j);
						g_snprintf(s3, 10, "%f", atan2(num2, num));
						num*=num;
						num2*=num2;
						num+=num2;
						num=sqrt(num);
						g_snprintf(s2, 10, "%f", num);
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							num=g_array_index(stars, gdouble, (2*k*sz4)+j);
							num2=g_array_index(stars, gdouble, (2*(++k)*sz4)-j);
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
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
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
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Real/Imaginary"), 1, _("Magnitude/Phase"), 2, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 1:
					str2=g_strdup(_("INVERSE_D\tREAL_VAL \tIMAG_VAL "));
					contents=g_strdup(str2);
					for (k=1; k<jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz4=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz4));
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
					for (j=1; j<sz4; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, j));
						g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*sz4)-j));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, (2*k*sz4)+j));
							g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*(++k)*sz4)-j));
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
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					g_free(fout);
					break;
					case 2:
					str2=g_strdup(_("INVERSE_D\tMAGNITUDE\tPHASE    "));
					contents=g_strdup(str2);
					for (k=1; k<jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz4=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz4));
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
					for (j=1; j<sz4; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						num=g_array_index(stars, gdouble, j);
						num2=g_array_index(stars, gdouble, (2*sz4)-j);
						g_snprintf(s3, 10, "%f", atan2(num2, num));
						num*=num;
						num2*=num2;
						num+=num2;
						num=sqrt(num);
						g_snprintf(s2, 10, "%f", num);
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							num=g_array_index(stars, gdouble, (2*k*sz4)+j);
							num2=g_array_index(stars, gdouble, (2*(++k)*sz4)-j);
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
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
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
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Real/Imaginary"), 1, _("Magnitude/Phase"), 2, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 1:
					str2=g_strdup(_("INVERSE_D\tREAL_VAL \tIMAG_VAL "));
					contents=g_strdup(str2);
					for (k=1; k<jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz4=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz4));
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
					for (j=1; j<sz4; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, j));
						g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*sz4)-j));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							g_snprintf(s2, 10, "%f", g_array_index(stars, gdouble, (2*k*sz4)+j));
							g_snprintf(s3, 10, "%f", g_array_index(stars, gdouble, (2*(++k)*sz4)-j));
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
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
					g_free(fout);
					break;
					case 2:
					str2=g_strdup(_("INVERSE_D\tMAGNITUDE\tPHASE    "));
					contents=g_strdup(str2);
					for (k=1; k<jdimxf; k++)
					{
						str=g_strjoin("\t", contents, str2, NULL);
						g_free(contents);
						contents=g_strdup(str);
						g_free(str);
					}
					g_free(str2);
					plt=PLOT_LINEAR(plot2);
					sz4=g_array_index((plt->sizes), gint, 0);
					g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 0));
					str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
					for (k=1; k<jdimxf; k++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(stars, gdouble, 2*k*sz4));
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
					for (j=1; j<sz4; j++)
					{
						g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
						num=g_array_index(stars, gdouble, j);
						num2=g_array_index(stars, gdouble, (2*sz4)-j);
						g_snprintf(s3, 10, "%f", atan2(num2, num));
						num*=num;
						num2*=num2;
						num+=num2;
						num=sqrt(num);
						g_snprintf(s2, 10, "%f", num);
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<jdimxf)
						{
							g_snprintf(s1, 10, "%f", j*g_array_index(delf, gdouble, 0));
							num=g_array_index(stars, gdouble, (2*k*sz4)+j);
							num2=g_array_index(stars, gdouble, (2*(++k)*sz4)-j);
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
						str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
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

