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

GtkPrintSettings *prst=NULL;

void prf(GtkPrintOperation *prto, GtkPrintContext *ctex, int page_nr)
{
	cairo_t *cr=gtk_print_context_get_cairo_context(ctex);
	gchar *str;

	if (page_nr) return;
	switch (gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook2)))
	{
		case 2:
		if ((flags&(PROC_BAT|PROC_PRS))==(PROC_BAT|PROC_PRS))
		{
		}
		else if ((flags&PROC_TRS)!=0)
		{
		}
		else if ((flags&PROC_OPN)!=0)
		{
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
		if ((flags&PROC_TRS)!=0)
		{
		}
		else if ((flags&PROC_OPN)!=0)
		{
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
		if ((flags&PROC_OPN)!=0)
		{
		}
		else
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 0);
			str=g_strdup(_("No available image."));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
		}
		break;
	}
}

void prb(GtkPrintOperation *prto, GtkPrintContext *ctex, int page_nr)
{
	gtk_print_operation_set_current_page(prto, 0);
	gtk_print_operation_set_has_selection(prto, FALSE);
}

void prt(GtkWidget *widget, gpointer data)
{
	GtkPrintOperation *prto;
	GtkPageSetup *prps;
	GtkPrintOperationResult res;
	GError *Err=NULL;
	gchar *str;

	prto=gtk_print_operation_new();
	if (prst!=NULL) gtk_print_operation_set_print_settings(prto, prst);
	/*prps=gtk_print_operation_get_default_page_setup(prto);
	gtk_page_setup_set_orientation(prps, GTK_PAGE_ORIENTATION_LANDSCAPE);
	gtk_print_operation_set_default_page_setup(prto, prps);*/
	g_signal_connect(prto, "begin_print", G_CALLBACK(prb), NULL);
	g_signal_connect(prto, "draw_page", G_CALLBACK(prf), NULL);
	res=gtk_print_operation_run(prto, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW(data), &Err);
	if (res==GTK_PRINT_OPERATION_RESULT_ERROR)
	{
		str=g_strdup_printf(_("An error occured while printing: %s."), (Err->message));
		gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
		g_free(str);
		g_error_free(Err);
	}
	else if (res==GTK_PRINT_OPERATION_RESULT_APPLY)
	{
		if (prst!=NULL) g_object_unref(prst);
		prst=g_object_ref(gtk_print_operation_get_print_settings(prto));
	}
	g_object_unref(prto);
}

void prg(GtkWidget *widget, gpointer data)
{
	GtkWidget *wfile;
	GtkFileFilter *epsfilt, *pngfilt, *svgfilt, *filt;
	gchar *str, *fout=NULL, *fout2=NULL;

	switch (gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook2)))
	{
		case 2:
		if ((flags&(PROC_BAT|PROC_PRS))==(PROC_BAT|PROC_PRS))
		{
wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(wfile), FALSE);
gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
pngfilt=gtk_file_filter_new();
gtk_file_filter_set_name(pngfilt, "Portable Network Graphics (PNG)");
gtk_file_filter_add_mime_type(pngfilt, "image/png");
gtk_file_filter_add_mime_type(pngfilt, "application/png");
gtk_file_filter_add_mime_type(pngfilt, "application/x-png");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), pngfilt);
epsfilt=gtk_file_filter_new();
gtk_file_filter_set_name(epsfilt, "Encapsulated Postscript (EPS)");
gtk_file_filter_add_mime_type(epsfilt, "application/postscript");
gtk_file_filter_add_mime_type(epsfilt, "application/eps");
gtk_file_filter_add_mime_type(epsfilt, "image/eps");
gtk_file_filter_add_mime_type(epsfilt, "image/x-eps");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), epsfilt);
svgfilt=gtk_file_filter_new();
gtk_file_filter_set_name(svgfilt, "Scalable Vector Graphics (SVG)");
gtk_file_filter_add_mime_type(svgfilt, "image/svg+xml");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), svgfilt);
			if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
			{
				g_free(folr);
				folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
				fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				filt=gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(wfile));
				if (filt==epsfilt)
				{
					if (g_str_has_suffix(fout, ".eps"))
					{
						if ((flags&PROC_POL)!=0) gtk_plot_polar_print_eps(plot3, fout);
						else gtk_plot_linear_print_eps(plot3, fout);
					}
					else
					{
						fout2=g_strconcat(fout, ".eps", NULL);
						if ((flags&PROC_POL)!=0) gtk_plot_polar_print_eps(plot3, fout2);
						else gtk_plot_linear_print_eps(plot3, fout2);
						g_free(fout2);
					}
				}
				else if (filt==svgfilt)
				{
					if (g_str_has_suffix(fout, ".svg"))
					{
						if ((flags&PROC_POL)!=0) gtk_plot_polar_print_svg(plot3, fout);
						else gtk_plot_linear_print_svg(plot3, fout);
					}
					else
					{
						fout2=g_strconcat(fout, ".svg", NULL);
						if ((flags&PROC_POL)!=0) gtk_plot_polar_print_svg(plot3, fout2);
						else gtk_plot_linear_print_svg(plot3, fout2);
						g_free(fout2);
					}
				}
				else if (g_str_has_suffix(fout, ".png"))
				{
					if ((flags&PROC_POL)!=0) gtk_plot_polar_print_png(plot3, fout);
					else gtk_plot_linear_print_png(plot3, fout);
				}
				else
				{
					fout2=g_strconcat(fout, ".png", NULL);
					if ((flags&PROC_POL)!=0) gtk_plot_polar_print_png(plot3, fout2);
					else gtk_plot_linear_print_png(plot3, fout2);
					g_free(fout2);
				}
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&PROC_TRS)!=0)
		{
wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(wfile), FALSE);
gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
pngfilt=gtk_file_filter_new();
gtk_file_filter_set_name(pngfilt, "Portable Network Graphics (PNG)");
gtk_file_filter_add_mime_type(pngfilt, "image/png");
gtk_file_filter_add_mime_type(pngfilt, "application/png");
gtk_file_filter_add_mime_type(pngfilt, "application/x-png");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), pngfilt);
epsfilt=gtk_file_filter_new();
gtk_file_filter_set_name(epsfilt, "Encapsulated Postscript (EPS)");
gtk_file_filter_add_mime_type(epsfilt, "application/postscript");
gtk_file_filter_add_mime_type(epsfilt, "application/eps");
gtk_file_filter_add_mime_type(epsfilt, "image/eps");
gtk_file_filter_add_mime_type(epsfilt, "image/x-eps");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), epsfilt);
svgfilt=gtk_file_filter_new();
gtk_file_filter_set_name(svgfilt, "Scalable Vector Graphics (SVG)");
gtk_file_filter_add_mime_type(svgfilt, "image/svg+xml");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), svgfilt);
if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
{
	g_free(folr);
	folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
	fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
	filt=gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(wfile));
	if (filt==epsfilt)
	{
		if (g_str_has_suffix(fout, ".eps")) gtk_plot_linear_print_eps(plot2, fout);
		else
		{
			fout2=g_strconcat(fout, ".eps", NULL);
			gtk_plot_linear_print_eps(plot2, fout2);
			g_free(fout2);
		}
	}
	else if (filt==svgfilt)
	{
		if (g_str_has_suffix(fout, ".svg")) gtk_plot_linear_print_svg(plot2, fout);
		else
		{
			fout2=g_strconcat(fout, ".svg", NULL);
			gtk_plot_linear_print_svg(plot2, fout2);
			g_free(fout2);
		}
	}
	else if (g_str_has_suffix(fout, ".png")) gtk_plot_linear_print_png(plot2, fout);
	else
	{
		fout2=g_strconcat(fout, ".png", NULL);
		gtk_plot_linear_print_png(plot2, fout2);
		g_free(fout2);
	}
	g_free(fout);
}
gtk_widget_destroy(wfile);
		}
		else if ((flags&PROC_OPN)!=0)
		{
wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(wfile), FALSE);
gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
pngfilt=gtk_file_filter_new();
gtk_file_filter_set_name(pngfilt, "Portable Network Graphics (PNG)");
gtk_file_filter_add_mime_type(pngfilt, "image/png");
gtk_file_filter_add_mime_type(pngfilt, "application/png");
gtk_file_filter_add_mime_type(pngfilt, "application/x-png");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), pngfilt);
epsfilt=gtk_file_filter_new();
gtk_file_filter_set_name(epsfilt, "Encapsulated Postscript (EPS)");
gtk_file_filter_add_mime_type(epsfilt, "application/postscript");
gtk_file_filter_add_mime_type(epsfilt, "application/eps");
gtk_file_filter_add_mime_type(epsfilt, "image/eps");
gtk_file_filter_add_mime_type(epsfilt, "image/x-eps");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), epsfilt);
svgfilt=gtk_file_filter_new();
gtk_file_filter_set_name(svgfilt, "Scalable Vector Graphics (SVG)");
gtk_file_filter_add_mime_type(svgfilt, "image/svg+xml");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), svgfilt);
if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
{
	g_free(folr);
	folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
	fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
	filt=gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(wfile));
	if (filt==epsfilt)
	{
		if (g_str_has_suffix(fout, ".eps")) gtk_plot_linear_print_eps(plot1, fout);
		else
		{
			fout2=g_strconcat(fout, ".eps", NULL);
			gtk_plot_linear_print_eps(plot1, fout2);
			g_free(fout2);
		}
	}
	else if (filt==svgfilt)
	{
		if (g_str_has_suffix(fout, ".svg")) gtk_plot_linear_print_svg(plot1, fout);
		else
		{
			fout2=g_strconcat(fout, ".svg", NULL);
			gtk_plot_linear_print_svg(plot1, fout2);
			g_free(fout2);
		}
	}
	else if (g_str_has_suffix(fout, ".png")) gtk_plot_linear_print_png(plot1, fout);
	else
	{
		fout2=g_strconcat(fout, ".png", NULL);
		gtk_plot_linear_print_png(plot1, fout2);
		g_free(fout2);
	}
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
		if ((flags&PROC_TRS)!=0)
		{
wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(wfile), FALSE);
gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
pngfilt=gtk_file_filter_new();
gtk_file_filter_set_name(pngfilt, "Portable Network Graphics (PNG)");
gtk_file_filter_add_mime_type(pngfilt, "image/png");
gtk_file_filter_add_mime_type(pngfilt, "application/png");
gtk_file_filter_add_mime_type(pngfilt, "application/x-png");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), pngfilt);
epsfilt=gtk_file_filter_new();
gtk_file_filter_set_name(epsfilt, "Encapsulated Postscript (EPS)");
gtk_file_filter_add_mime_type(epsfilt, "application/postscript");
gtk_file_filter_add_mime_type(epsfilt, "application/eps");
gtk_file_filter_add_mime_type(epsfilt, "image/eps");
gtk_file_filter_add_mime_type(epsfilt, "image/x-eps");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), epsfilt);
svgfilt=gtk_file_filter_new();
gtk_file_filter_set_name(svgfilt, "Scalable Vector Graphics (SVG)");
gtk_file_filter_add_mime_type(svgfilt, "image/svg+xml");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), svgfilt);
if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
{
	g_free(folr);
	folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
	fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
	filt=gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(wfile));
	if (filt==epsfilt)
	{
		if (g_str_has_suffix(fout, ".eps")) gtk_plot_linear_print_eps(plot2, fout);
		else
		{
			fout2=g_strconcat(fout, ".eps", NULL);
			gtk_plot_linear_print_eps(plot2, fout2);
			g_free(fout2);
		}
	}
	else if (filt==svgfilt)
	{
		if (g_str_has_suffix(fout, ".svg")) gtk_plot_linear_print_svg(plot2, fout);
		else
		{
			fout2=g_strconcat(fout, ".svg", NULL);
			gtk_plot_linear_print_svg(plot2, fout2);
			g_free(fout2);
		}
	}
	else if (g_str_has_suffix(fout, ".png")) gtk_plot_linear_print_png(plot2, fout);
	else
	{
		fout2=g_strconcat(fout, ".png", NULL);
		gtk_plot_linear_print_png(plot2, fout2);
		g_free(fout2);
	}
	g_free(fout);
}
gtk_widget_destroy(wfile);
		}
		else if ((flags&PROC_OPN)!=0)
		{
wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(wfile), FALSE);
gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
pngfilt=gtk_file_filter_new();
gtk_file_filter_set_name(pngfilt, "Portable Network Graphics (PNG)");
gtk_file_filter_add_mime_type(pngfilt, "image/png");
gtk_file_filter_add_mime_type(pngfilt, "application/png");
gtk_file_filter_add_mime_type(pngfilt, "application/x-png");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), pngfilt);
epsfilt=gtk_file_filter_new();
gtk_file_filter_set_name(epsfilt, "Encapsulated Postscript (EPS)");
gtk_file_filter_add_mime_type(epsfilt, "application/postscript");
gtk_file_filter_add_mime_type(epsfilt, "application/eps");
gtk_file_filter_add_mime_type(epsfilt, "image/eps");
gtk_file_filter_add_mime_type(epsfilt, "image/x-eps");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), epsfilt);
svgfilt=gtk_file_filter_new();
gtk_file_filter_set_name(svgfilt, "Scalable Vector Graphics (SVG)");
gtk_file_filter_add_mime_type(svgfilt, "image/svg+xml");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), svgfilt);
if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
{
	g_free(folr);
	folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
	fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
	filt=gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(wfile));
	if (filt==epsfilt)
	{
		if (g_str_has_suffix(fout, ".eps")) gtk_plot_linear_print_eps(plot1, fout);
		else
		{
			fout2=g_strconcat(fout, ".eps", NULL);
			gtk_plot_linear_print_eps(plot1, fout2);
			g_free(fout2);
		}
	}
	else if (filt==svgfilt)
	{
		if (g_str_has_suffix(fout, ".svg")) gtk_plot_linear_print_svg(plot1, fout);
		else
		{
			fout2=g_strconcat(fout, ".svg", NULL);
			gtk_plot_linear_print_svg(plot1, fout2);
			g_free(fout2);
		}
	}
	else if (g_str_has_suffix(fout, ".png")) gtk_plot_linear_print_png(plot1, fout);
	else
	{
		fout2=g_strconcat(fout, ".png", NULL);
		gtk_plot_linear_print_png(plot1, fout2);
		g_free(fout2);
	}
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
		if ((flags&PROC_OPN)!=0)
		{
wfile=gtk_file_chooser_dialog_new(_("Select Image File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(wfile), FALSE);
gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
pngfilt=gtk_file_filter_new();
gtk_file_filter_set_name(pngfilt, "Portable Network Graphics (PNG)");
gtk_file_filter_add_mime_type(pngfilt, "image/png");
gtk_file_filter_add_mime_type(pngfilt, "application/png");
gtk_file_filter_add_mime_type(pngfilt, "application/x-png");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), pngfilt);
epsfilt=gtk_file_filter_new();
gtk_file_filter_set_name(epsfilt, "Encapsulated Postscript (EPS)");
gtk_file_filter_add_mime_type(epsfilt, "application/postscript");
gtk_file_filter_add_mime_type(epsfilt, "application/eps");
gtk_file_filter_add_mime_type(epsfilt, "image/eps");
gtk_file_filter_add_mime_type(epsfilt, "image/x-eps");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), epsfilt);
svgfilt=gtk_file_filter_new();
gtk_file_filter_set_name(svgfilt, "Scalable Vector Graphics (SVG)");
gtk_file_filter_add_mime_type(svgfilt, "image/svg+xml");
gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(wfile), svgfilt);
if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
{
	g_free(folr);
	folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
	fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
	filt=gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(wfile));
	if (filt==epsfilt)
	{
		if (g_str_has_suffix(fout, ".eps")) gtk_plot_linear_print_eps(plot1, fout);
		else
		{
			fout2=g_strconcat(fout, ".eps", NULL);
			gtk_plot_linear_print_eps(plot1, fout2);
			g_free(fout2);
		}
	}
	else if (filt==svgfilt)
	{
		if (g_str_has_suffix(fout, ".svg")) gtk_plot_linear_print_svg(plot1, fout);
		else
		{
			fout2=g_strconcat(fout, ".svg", NULL);
			gtk_plot_linear_print_svg(plot1, fout2);
			g_free(fout2);
		}
	}
	else if (g_str_has_suffix(fout, ".png")) gtk_plot_linear_print_png(plot1, fout);
	else
	{
		fout2=g_strconcat(fout, ".png", NULL);
		gtk_plot_linear_print_png(plot1, fout2);
		g_free(fout2);
	}
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
	gchar *contents, *fout=NULL, *str, *str2;
	gchar s0[10], s1[10], s2[10], s3[10], s4[10];
	gdouble num, num2;
	GError *Err=NULL;
	gint dim, j, jdimx, k, st4, sz4;
	GtkPlot *pt;
	GtkPlotLinear *plt;
	GtkPlotPolar *plt2;
	GtkWidget *cont, *dialog, *label, *wfile;

	switch (gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook2)))
	{
		case 2:
		if ((flags&(PROC_CHP|PROC_BAT|PROC_PRS))==(PROC_CHP|PROC_BAT|PROC_PRS))
		{
wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
{
	g_free(folr);
	folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
	fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				pt=GTK_PLOT(plot3);
				sz4=g_array_index(pt->sizes, gint, 0);
				st4=g_array_index(pt->stride, gint, 0);
				if (kdimxf==1)
				{
					if (st4==1)
					{
						dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Visibility"), 1, _("Domain Shift"), 2, _("Chirp"), 3, _("All"), 4, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
						cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
						label=gtk_label_new(_("Select Parameter to save:"));
						gtk_container_add(GTK_CONTAINER(cont), label);
						switch (gtk_dialog_run(GTK_DIALOG(dialog)))
						{
							case 4:
							contents=g_strdup(_("MEAS     \tVISIBILTY\tDOMN_SHFT\tCHIRP    "));
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (j=0; j<sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, j));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, j));
g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, j+sz4));
g_snprintf(s4, 10, "%f", g_array_index(plt2->rdata, gdouble, j+(2*sz4)));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (j=0; j<sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, j));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, j));
g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, j+sz4));
g_snprintf(s4, 10, "%f", g_array_index(plt->ydata, gdouble, j+(2*sz4)));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 3:
							contents=g_strdup(_("MEAS     \tCHIRP    "));
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (j=2*sz4; j<3*sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, j));
g_snprintf(s4, 10, "%f", g_array_index(plt2->rdata, gdouble, j));
str2=g_strjoin("\t", s1, s2, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (j=2*sz4; j<3*sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, j));
g_snprintf(s4, 10, "%f", g_array_index(plt->ydata, gdouble, j));
str2=g_strjoin("\t", s1, s2, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 2:
							contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (j=sz4; j<2*sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, j));
g_snprintf(s4, 10, "%f", g_array_index(plt2->rdata, gdouble, j));
str2=g_strjoin("\t", s1, s2, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (j=sz4; j<2*sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, j));
g_snprintf(s4, 10, "%f", g_array_index(plt->ydata, gdouble, j));
str2=g_strjoin("\t", s1, s2, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 1:
							contents=g_strdup(_("MEAS     \tVISIBILTY"));
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (j=0; j<sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, j));
g_snprintf(s4, 10, "%f", g_array_index(plt2->rdata, gdouble, j));
str2=g_strjoin("\t", s1, s2, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (j=0; j<sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, j));
g_snprintf(s4, 10, "%f", g_array_index(plt->ydata, gdouble, j));
str2=g_strjoin("\t", s1, s2, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
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
for (j=1; j<st4; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k*st4));
g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, (k+sz4)*st4));
g_snprintf(s4, 10, "%f", g_array_index(plt2->rdata, gdouble, (k+2*sz4)*st4));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, j+(k*st4)));
	g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, j+((k+sz4)*st4)));
	g_snprintf(s4, 10, "%f", g_array_index(plt2->rdata, gdouble, (j++)+((k+(2*sz4))*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k*st4));
g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, (k+sz4)*st4));
g_snprintf(s4, 10, "%f", g_array_index(plt->ydata, gdouble, (k+2*sz4)*st4));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, j+(k*st4)));
	g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, j+((k+sz4)*st4)));
	g_snprintf(s4, 10, "%f", g_array_index(plt->ydata, gdouble, (j++)+((k+(2*sz4))*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 3:
							contents=g_strdup(_("MEAS     \tCHIRP    "));
str2=g_strdup(contents);
for (j=1; j<st4; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=2*sz4; k<3*sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=2*sz4; k<3*sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 2:
							contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
str2=g_strdup(contents);
for (j=1; j<st4; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=sz4; k<2*sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=sz4; k<2*sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 1:
							contents=g_strdup(_("MEAS     \tVISIBILTY"));
str2=g_strdup(contents);
for (j=1; j<st4; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							default:
							break;
						}
					}
				}
				else
				{
					jdimx=st4/kdimxf;
					if (jdimx==1)
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
for (j=1; j<st4; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k*st4));
g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, (k+sz4)*st4));
g_snprintf(s4, 10, "%f", g_array_index(plt2->rdata, gdouble, (k+2*sz4)*st4));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, j+(k*st4)));
	g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, j+((k+sz4)*st4)));
	g_snprintf(s4, 10, "%f", g_array_index(plt2->rdata, gdouble, (j++)+((k+(2*sz4))*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k*st4));
g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, (k+sz4)*st4));
g_snprintf(s4, 10, "%f", g_array_index(plt->ydata, gdouble, (k+2*sz4)*st4));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, j+(k*st4)));
	g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, j+((k+sz4)*st4)));
	g_snprintf(s4, 10, "%f", g_array_index(plt->ydata, gdouble, (j++)+((k+(2*sz4))*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 3:
							contents=g_strdup(_("MEAS     \tCHIRP    "));
str2=g_strdup(contents);
for (j=1; j<st4; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=2*sz4; k<3*sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=2*sz4; k<3*sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 2:
							contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
str2=g_strdup(contents);
for (j=1; j<st4; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=sz4; k<2*sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=sz4; k<2*sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 1:
							contents=g_strdup(_("MEAS     \tVISIBILTY"));
str2=g_strdup(contents);
for (j=1; j<st4; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
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
for (j=1; j<kdimxf; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							dim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(jind));
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (j=0; j<sz4; j++)
								{
									g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, (dim*kdimxf)+(j*st4)));
									g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (dim*kdimxf)+(j*st4)));
									g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, (dim*kdimxf)+((j+sz4)*st4)));
									g_snprintf(s4, 10, "%f", g_array_index(plt2->rdata, gdouble, (dim*kdimxf)+((j+(2*sz4))*st4)));
									str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
									k=1;
									while (k<kdimxf)
									{
										g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k+(dim*kdimxf)+(j*st4)));
										g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, k+(dim*kdimxf)+((j+sz4)*st4)));
										g_snprintf(s4, 10, "%f", g_array_index(plt2->rdata, gdouble, (k++)+(dim*kdimxf)+((j+(2*sz4))*st4)));
										str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
										g_free(str2);
										str2=g_strdup(str);
										g_free(str);
									}
									str=g_strdup(contents);
									g_free(contents);
									contents=g_strjoin(DLMT, str, str2, NULL);
									{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (j=0; j<sz4; j++)
								{
									g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, (dim*kdimxf)+(j*st4)));
									g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (dim*kdimxf)+(j*st4)));
									g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, (dim*kdimxf)+((j+sz4)*st4)));
									g_snprintf(s4, 10, "%f", g_array_index(plt->ydata, gdouble, (dim*kdimxf)+((j+(2*sz4))*st4)));
									str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
									k=1;
									while (k<kdimxf)
									{
										g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k+(dim*kdimxf)+(j*st4)));
										g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, k+(dim*kdimxf)+((j+sz4)*st4)));
										g_snprintf(s4, 10, "%f", g_array_index(plt->ydata, gdouble, (k++)+(dim*kdimxf)+((j+(2*sz4))*st4)));
										str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
										g_free(str2);
										str2=g_strdup(str);
										g_free(str);
									}
									str=g_strdup(contents);
									g_free(contents);
									contents=g_strjoin(DLMT, str, str2, NULL);
									{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 7:
							contents=g_strdup(_("MEAS     \tCHIRP    "));
str2=g_strdup(contents);
for (j=1; j<kdimxf; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (j=2*sz4; j<3*sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, (dim*kdimxf)+(j*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (dim*kdimxf)+(j*st4)));
str2=g_strjoin("\t", s1, s2, NULL);
k=1;
while (k<kdimxf)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k+(dim*kdimxf)+(j*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (j=2*sz4; j<3*sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, (dim*kdimxf)+(j*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (dim*kdimxf)+(j*st4)));
str2=g_strjoin("\t", s1, s2, NULL);
k=1;
while (k<kdimxf)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k+(dim*kdimxf)+(j*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 6:
							contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
str2=g_strdup(contents);
for (j=1; j<kdimxf; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (j=sz4; j<2*sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, (dim*kdimxf)+(j*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (dim*kdimxf)+(j*st4)));
str2=g_strjoin("\t", s1, s2, NULL);
k=1;
while (k<kdimxf)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k+(dim*kdimxf)+(j*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (j=sz4; j<2*sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, (dim*kdimxf)+(j*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (dim*kdimxf)+(j*st4)));
str2=g_strjoin("\t", s1, s2, NULL);
k=1;
while (k<kdimxf)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k+(dim*kdimxf)+(j*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 5:
							contents=g_strdup(_("MEAS     \tVISIBILTY"));
str2=g_strdup(contents);
for (j=1; j<kdimxf; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (j=0; j<sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, (dim*kdimxf)+(j*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (dim*kdimxf)+(j*st4)));
str2=g_strjoin("\t", s1, s2, NULL);
k=1;
while (k<kdimxf)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k+(dim*kdimxf)+(j*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (j=0; j<sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, (dim*kdimxf)+(j*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (dim*kdimxf)+(j*st4)));
str2=g_strjoin("\t", s1, s2, NULL);
k=1;
while (k<kdimxf)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k+(dim*kdimxf)+(j*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 4:
							contents=g_strdup(_("MEAS     \tVISIBILTY\tDOMN_SHFT\tCHIRP    "));
str2=g_strdup(contents);
for (j=1; j<jdimx; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							dim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(kind));
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=0; k<sz4; k++)
								{
									g_snprintf(s2, 10, "%f", g_array_index(plt2->thdata, gdouble, dim+(k*st4)));
									g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+(k*st4)));
									g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+((k+sz4)*st4)));
									g_snprintf(s4, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+((k+(2*sz4))*st4)));
									str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
									k=1;
									while (j<jdimx)
									{
										g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+(j*kdimxf)+(k*st4)));
										g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+(j*kdimxf)+((k+sz4)*st4)));
										g_snprintf(s4, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+((j++)*kdimxf)+((k+(2*sz4))*st4)));
										str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
										g_free(str2);
										str2=g_strdup(str);
										g_free(str);
									}
									str=g_strdup(contents);
									g_free(contents);
									contents=g_strjoin(DLMT, str, str2, NULL);
									{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=0; k<sz4; k++)
								{
									g_snprintf(s2, 10, "%f", g_array_index(plt->xdata, gdouble, dim+(k*st4)));
									g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, dim+(k*st4)));
									g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, dim+((k+sz4)*st4)));
									g_snprintf(s4, 10, "%f", g_array_index(plt->ydata, gdouble, dim+((k+(2*sz4))*st4)));
									str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
									k=1;
									while (j<jdimx)
									{
										g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, dim+(j*kdimxf)+(k*st4)));
										g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, dim+(j*kdimxf)+((k+sz4)*st4)));
										g_snprintf(s4, 10, "%f", g_array_index(plt->ydata, gdouble, dim+((j++)*kdimxf)+((k+(2*sz4))*st4)));
										str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
										g_free(str2);
										str2=g_strdup(str);
										g_free(str);
									}
									str=g_strdup(contents);
									g_free(contents);
									contents=g_strjoin(DLMT, str, str2, NULL);
									{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 3:
							contents=g_strdup(_("MEAS     \tCHIRP    "));
str2=g_strdup(contents);
for (j=1; j<jdimx; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=2*sz4; k<3*sz4; k++)
								{
g_snprintf(s2, 10, "%f", g_array_index(plt2->thdata, gdouble, dim+(k*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+(k*st4)));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
k=1;
while (j<jdimx)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+(j*kdimxf)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=2*sz4; k<3*sz4; k++)
								{
g_snprintf(s2, 10, "%f", g_array_index(plt->xdata, gdouble, dim+(k*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, dim+(k*st4)));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
k=1;
while (j<jdimx)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, dim+(j*kdimxf)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 2:
							contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
str2=g_strdup(contents);
for (j=1; j<jdimx; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=sz4; k<2*sz4; k++)
								{
g_snprintf(s2, 10, "%f", g_array_index(plt2->thdata, gdouble, dim+(k*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+(k*st4)));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
k=1;
while (j<jdimx)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+(j*kdimxf)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=sz4; k<2*sz4; k++)
								{
g_snprintf(s2, 10, "%f", g_array_index(plt->xdata, gdouble, dim+(k*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, dim+(k*st4)));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
k=1;
while (j<jdimx)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, dim+(j*kdimxf)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 1:
							contents=g_strdup(_("MEAS     \tVISIBILTY"));
str2=g_strdup(contents);
for (j=1; j<jdimx; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s2, 10, "%f", g_array_index(plt2->thdata, gdouble, dim+(k*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+(k*st4)));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
k=1;
while (j<jdimx)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+(j*kdimxf)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s2, 10, "%f", g_array_index(plt->xdata, gdouble, dim+(k*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, dim+(k*st4)));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
k=1;
while (j<jdimx)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, dim+(j*kdimxf)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							default:
							break;
						}
					}
				}
				{gtk_widget_destroy(dialog); g_free(fout);}
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&(PROC_BAT|PROC_PRS))==(PROC_BAT|PROC_PRS))
		{
wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
{
	g_free(folr);
	folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
	fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				pt=GTK_PLOT(plot3);
				sz4=g_array_index(pt->sizes, gint, 0);
				st4=g_array_index(pt->stride, gint, 0);
				if (kdimxf==1)
				{
					if (st4==1)
					{
						dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Visibility"), 1, _("Domain Shift"), 2, _("Both"), 3, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
						cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
						label=gtk_label_new(_("Select Parameter to save:"));
						gtk_container_add(GTK_CONTAINER(cont), label);
						switch (gtk_dialog_run(GTK_DIALOG(dialog)))
						{
							case 3:
							contents=g_strdup(_("MEAS     \tVISIBILTY\tDOMN_SHFT"));
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (j=0; j<sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, j));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, j));
g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, j+sz4));
str2=g_strjoin("\t", s1, s2, s3, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (j=0; j<sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, j));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, j));
g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, j+sz4));
str2=g_strjoin("\t", s1, s2, s3, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 2:
							contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (j=sz4; j<2*sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, j));
g_snprintf(s4, 10, "%f", g_array_index(plt2->rdata, gdouble, j));
str2=g_strjoin("\t", s1, s2, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (j=sz4; j<2*sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, j));
g_snprintf(s4, 10, "%f", g_array_index(plt->ydata, gdouble, j));
str2=g_strjoin("\t", s1, s2, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 1:
							contents=g_strdup(_("MEAS     \tVISIBILTY"));
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (j=0; j<sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, j));
g_snprintf(s4, 10, "%f", g_array_index(plt2->rdata, gdouble, j));
str2=g_strjoin("\t", s1, s2, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (j=0; j<sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, j));
g_snprintf(s4, 10, "%f", g_array_index(plt->ydata, gdouble, j));
str2=g_strjoin("\t", s1, s2, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
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
for (j=1; j<st4; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k*st4));
g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, (k+sz4)*st4));
str2=g_strjoin("\t", s1, s2, s3, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, j+(k*st4)));
	g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, (j++)+((k+sz4)*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k*st4));
g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, (k+sz4)*st4));
str2=g_strjoin("\t", s1, s2, s3, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, j+(k*st4)));
	g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, (j++)+((k+sz4)*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 2:
							contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
str2=g_strdup(contents);
for (j=1; j<st4; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=sz4; k<2*sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=sz4; k<2*sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 1:
							contents=g_strdup(_("MEAS     \tVISIBILTY"));
str2=g_strdup(contents);
for (j=1; j<st4; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							default:
							break;
						}
					}
				}
				else
				{
					jdimx=st4/kdimxf;
					if (jdimx==1)
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
for (j=1; j<st4; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k*st4));
g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, (k+sz4)*st4));
str2=g_strjoin("\t", s1, s2, s3, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, j+(k*st4)));
	g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, (j++)+((k+sz4)*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k*st4));
g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, (k+sz4)*st4));
str2=g_strjoin("\t", s1, s2, s3, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, j+(k*st4)));
	g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, (j++)+((k+sz4)*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 2:
							contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
str2=g_strdup(contents);
for (j=1; j<st4; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=sz4; k<2*sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=sz4; k<2*sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 1:
							contents=g_strdup(_("MEAS     \tVISIBILTY"));
str2=g_strdup(contents);
for (j=1; j<st4; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, k*st4));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k*st4));
str2=g_strjoin("\t", s1, s2, NULL);
j=1;
while (j<st4)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (j++)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
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
for (j=1; j<kdimxf; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							dim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(jind));
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (j=0; j<sz4; j++)
								{
									g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, (dim*kdimxf)+(j*st4)));
									g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (dim*kdimxf)+(j*st4)));
									g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, (dim*kdimxf)+((j+sz4)*st4)));
									str2=g_strjoin("\t", s1, s2, s3, NULL);
									k=1;
									while (k<kdimxf)
									{
										g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k+(dim*kdimxf)+(j*st4)));
										g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, (k++)+(dim*kdimxf)+((j+sz4)*st4)));
										str=g_strjoin("\t", str2, s1, s2, s3, NULL);
										g_free(str2);
										str2=g_strdup(str);
										g_free(str);
									}
									str=g_strdup(contents);
									g_free(contents);
									contents=g_strjoin(DLMT, str, str2, NULL);
									{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (j=0; j<sz4; j++)
								{
									g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, (dim*kdimxf)+(j*st4)));
									g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (dim*kdimxf)+(j*st4)));
									g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, (dim*kdimxf)+((j+sz4)*st4)));
									str2=g_strjoin("\t", s1, s2, s3, NULL);
									k=1;
									while (k<kdimxf)
									{
										g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k+(dim*kdimxf)+(j*st4)));
										g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, (k++)+(dim*kdimxf)+((j+sz4)*st4)));
										str=g_strjoin("\t", str2, s1, s2, s3, NULL);
										g_free(str2);
										str2=g_strdup(str);
										g_free(str);
									}
									str=g_strdup(contents);
									g_free(contents);
									contents=g_strjoin(DLMT, str, str2, NULL);
									{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 5:
							contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
str2=g_strdup(contents);
for (j=1; j<kdimxf; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (j=sz4; j<2*sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, (dim*kdimxf)+(j*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (dim*kdimxf)+(j*st4)));
str2=g_strjoin("\t", s1, s2, NULL);
k=1;
while (k<kdimxf)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k+(dim*kdimxf)+(j*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (j=sz4; j<2*sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, (dim*kdimxf)+(j*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (dim*kdimxf)+(j*st4)));
str2=g_strjoin("\t", s1, s2, NULL);
k=1;
while (k<kdimxf)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k+(dim*kdimxf)+(j*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 4:
							contents=g_strdup(_("MEAS     \tVISIBILTY"));
str2=g_strdup(contents);
for (j=1; j<kdimxf; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (j=0; j<sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt2->thdata, gdouble, (dim*kdimxf)+(j*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, (dim*kdimxf)+(j*st4)));
str2=g_strjoin("\t", s1, s2, NULL);
k=1;
while (k<kdimxf)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, k+(dim*kdimxf)+(j*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (j=0; j<sz4; j++)
								{
g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, (dim*kdimxf)+(j*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, (dim*kdimxf)+(j*st4)));
str2=g_strjoin("\t", s1, s2, NULL);
k=1;
while (k<kdimxf)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, k+(dim*kdimxf)+(j*st4)));
	str=g_strjoin("\t", str2, s1, s2, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 3:
							contents=g_strdup(_("MEAS     \tVISIBILTY\tDOMN_SHFT"));
str2=g_strdup(contents);
for (j=1; j<jdimx; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							dim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(kind));
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=0; k<sz4; k++)
								{
									g_snprintf(s2, 10, "%f", g_array_index(plt2->thdata, gdouble, dim+(k*st4)));
									g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+(k*st4)));
									g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+((k+sz4)*st4)));
									str2=g_strjoin("\t", s1, s2, s3, NULL);
									k=1;
									while (j<jdimx)
									{
										g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+(j*kdimxf)+(k*st4)));
										g_snprintf(s3, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+((j++)*kdimxf)+((k+sz4)*st4)));
										str=g_strjoin("\t", str2, s1, s2, s3, NULL);
										g_free(str2);
										str2=g_strdup(str);
										g_free(str);
									}
									str=g_strdup(contents);
									g_free(contents);
									contents=g_strjoin(DLMT, str, str2, NULL);
									{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=0; k<sz4; k++)
								{
									g_snprintf(s2, 10, "%f", g_array_index(plt->xdata, gdouble, dim+(k*st4)));
									g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, dim+(k*st4)));
									g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, dim+((k+sz4)*st4)));
									str2=g_strjoin("\t", s1, s2, s3, NULL);
									k=1;
									while (j<jdimx)
									{
										g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, dim+(j*kdimxf)+(k*st4)));
										g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, dim+((j++)*kdimxf)+((k+sz4)*st4)));
										str=g_strjoin("\t", str2, s1, s2, s3, NULL);
										g_free(str2);
										str2=g_strdup(str);
										g_free(str);
									}
									str=g_strdup(contents);
									g_free(contents);
									contents=g_strjoin(DLMT, str, str2, NULL);
									{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 2:
							contents=g_strdup(_("MEAS     \tDOMN_SHFT"));
str2=g_strdup(contents);
for (j=1; j<jdimx; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=sz4; k<2*sz4; k++)
								{
g_snprintf(s2, 10, "%f", g_array_index(plt2->thdata, gdouble, dim+(k*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+(k*st4)));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
k=1;
while (j<jdimx)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+(j*kdimxf)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=sz4; k<2*sz4; k++)
								{
g_snprintf(s2, 10, "%f", g_array_index(plt->xdata, gdouble, dim+(k*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, dim+(k*st4)));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
k=1;
while (j<jdimx)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, dim+(j*kdimxf)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							case 1:
							contents=g_strdup(_("MEAS     \tVISIBILTY"));
str2=g_strdup(contents);
for (j=1; j<jdimx; j++)
	{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
							if ((flags&PROC_POL)==PROC_POL)
							{
								plt2=GTK_PLOT_POLAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s2, 10, "%f", g_array_index(plt2->thdata, gdouble, dim+(k*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+(k*st4)));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
k=1;
while (j<jdimx)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt2->rdata, gdouble, dim+(j*kdimxf)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
							else
							{
								plt=GTK_PLOT_LINEAR(plot3);
								for (k=0; k<sz4; k++)
								{
g_snprintf(s2, 10, "%f", g_array_index(plt->xdata, gdouble, dim+(k*st4)));
g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, dim+(k*st4)));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
k=1;
while (j<jdimx)
{
	g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, dim+(j*kdimxf)+(k*st4)));
	str=g_strjoin("\t", str2, s1, s2, s3, s4, NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
								}
							}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
							break;
							default:
							break;
						}
					}
				}
				{g_free(fout); gtk_widget_destroy(dialog);}
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&(PROC_CHP|PROC_PRS))==(PROC_CHP|PROC_PRS))
		{
wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
{
	g_free(folr);
	folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
	fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				if (kdimxf==1)
				{
					if ((vis->len)==1)
					{
						str=g_strdup(_("VISIBILTY\tDOMN_SHFT\tCHIRP    "));
						g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, 0));
						g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, 0));
						g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, 0));
						str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
						contents=g_strjoin(DLMT, str, str2, NULL);
						{g_free(str); g_free(str2);}
					}
					else
					{
						contents=g_strdup(_("J        \tVISIBILTY\tDOMN_SHFT\tCHIRP    "));
						for (k=0; k<(vis->len); k++)
						{
g_snprintf(s1, 10, "%d", k);
g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, k));
g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, k));
g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, k));
str2=g_strjoin("\t", s1, s2, s3, s4, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
						}
					}
				}
				else if ((vis->len)==kdimxf)
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
{g_free(str); g_free(str2);}
					}
				}
				else
				{
					jdimx=(vis->len)/kdimxf;
					contents=g_strdup(_("J        \tK        \tVISIBILTY\tDOMN_SHFT\tCHIRP    "));
					for (j=0; j<jdimx; j++)
					{
						g_snprintf(s1, 10, "%d", j);
						for (k=0; k<kdimxf; k++)
						{
							g_snprintf(s1, 10, "%d", k);
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, k+(j*kdimxf)));
							g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, k+(j*kdimxf)));
							g_snprintf(s4, 10, "%f", g_array_index(chp, gdouble, k+(j*kdimxf)));
							str2=g_strjoin("\t", s0, s1, s2, s3, s4, NULL);
							str=g_strdup(contents);
							g_free(contents);
							contents=g_strjoin(DLMT, str, str2, NULL);
							{g_free(str); g_free(str2);}
						}
					}
				}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&PROC_PRS)!=0)
		{
wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
{
	g_free(folr);
	folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
	fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				if (kdimxf==1)
				{
					if ((vis->len)==1)
					{
						str=g_strdup(_("VISIBILTY\tDOMN_SHFT"));
						g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, 0));
						g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, 0));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						contents=g_strjoin(DLMT, str, str2, NULL);
						{g_free(str); g_free(str2);}
					}
					else
					{
						contents=g_strdup(_("J        \tVISIBILTY\tDOMN_SHFT"));
						for (k=0; k<(vis->len); k++)
						{
g_snprintf(s1, 10, "%d", k);
g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, k));
g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, k));
str2=g_strjoin("\t", s1, s2, s3, NULL);
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
						}
					}
				}
				else if ((vis->len)==kdimxf)
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
{g_free(str); g_free(str2);}
					}
				}
				else
				{
					jdimx=(vis->len)/kdimxf;
					contents=g_strdup(_("J        \tK        \tVISIBILTY\tDOMN_SHFT"));
					for (j=0; j<jdimx; j++)
					{
						g_snprintf(s1, 10, "%d", j);
						for (k=0; k<kdimxf; k++)
						{
							g_snprintf(s1, 10, "%d", k);
							g_snprintf(s2, 10, "%f", g_array_index(vis, gdouble, k+(j*kdimxf)));
							g_snprintf(s3, 10, "%f", g_array_index(doms, gdouble, k+(j*kdimxf)));
							str2=g_strjoin("\t", s0, s1, s2, s3, NULL);
							str=g_strdup(contents);
							g_free(contents);
							contents=g_strjoin(DLMT, str, str2, NULL);
							{g_free(str); g_free(str2);}
						}
					}
				}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
				g_free(fout);
			}
			gtk_widget_destroy(wfile);
		}
		else if ((flags&PROC_TRS)!=0)
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 1);
wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
{
	g_free(folr);
	folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
	fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Real/Imaginary"), 1, _("Magnitude/Phase"), 2, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 2:
					str2=g_strdup(_("INVERSE_D\tMAGNITUDE\tPHASE    "));
pt=GTK_PLOT(plot2);
plt=GTK_PLOT_LINEAR(plot2);
sz4=g_array_index((pt->sizes), gint, 0);
jdimx=(plt->xdata->len)/(2*sz4);
contents=g_strdup(str2);
for (k=1; k<jdimx; k++)
{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
g_snprintf(s1, 10, "%f", g_array_index(plt->ydata, gdouble, 0));
str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
for (k=1; k<jdimx; k++)
{
	g_snprintf(s1, 10, "%f", g_array_index(plt->ydata, gdouble, 2*k*sz4));
	str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
					for (j=1; j<sz4; j++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, 2*j));
						g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, 2*j));
						g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, (2*j)+1));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<jdimx)
						{
							g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, 2*(j+(k*sz4))));
							g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, 2*(j+(k*sz4))));
							g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, (2*(j+((k++)*sz4)))+1));
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						{g_free(str); g_free(str2);}
					}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
					g_free(fout);
					break;
					case 1:
					str2=g_strdup(_("INVERSE_D\tREAL_VAL \tIMAG_VAL "));
pt=GTK_PLOT(plot2);
plt=GTK_PLOT_LINEAR(plot2);
sz4=g_array_index((pt->sizes), gint, 0);
jdimx=(plt->xdata->len)/(2*sz4);
contents=g_strdup(str2);
for (k=1; k<jdimx; k++)
{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
g_snprintf(s1, 10, "%f", g_array_index(plt->ydata, gdouble, 0));
str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
for (k=1; k<jdimx; k++)
{
	g_snprintf(s1, 10, "%f", g_array_index(plt->ydata, gdouble, 2*k*sz4));
	str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
					for (j=1; j<sz4; j++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, 2*j));
						num=g_array_index(plt->ydata, gdouble, 2*j);
						num2=g_array_index(plt->ydata, gdouble, (2*j)+1);
						g_snprintf(s2, 10, "%f", num*cos(num2));
						g_snprintf(s3, 10, "%f", num*sin(num2));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<jdimx)
						{
							g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, 2*(j+(k*sz4))));
							num=g_array_index(plt->ydata, gdouble, 2*(j+(k*sz4)));
							num2=g_array_index(plt->ydata, gdouble, (2*(j+((k++)*sz4)))+1);
							g_snprintf(s2, 10, "%f", num*cos(num2));
							g_snprintf(s3, 10, "%f", num*sin(num2));
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						{g_free(str); g_free(str2);}
					}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
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
		if ((flags&PROC_TRS)!=0)
		{
wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
{
	g_free(folr);
	folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
	fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Real/Imaginary"), 1, _("Magnitude/Phase"), 2, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 2:
					str2=g_strdup(_("INVERSE_D\tMAGNITUDE\tPHASE    "));
pt=GTK_PLOT(plot2);
plt=GTK_PLOT_LINEAR(plot2);
sz4=g_array_index((pt->sizes), gint, 0);
jdimx=(plt->xdata->len)/(2*sz4);
contents=g_strdup(str2);
for (k=1; k<jdimx; k++)
{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
g_snprintf(s1, 10, "%f", g_array_index(plt->ydata, gdouble, 0));
str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
for (k=1; k<jdimx; k++)
{
	g_snprintf(s1, 10, "%f", g_array_index(plt->ydata, gdouble, 2*k*sz4));
	str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
					for (j=1; j<sz4; j++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, 2*j));
						g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, 2*j));
						g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, (2*j)+1));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<jdimx)
						{
							g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, 2*(j+(k*sz4))));
							g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, 2*(j+(k*sz4))));
							g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, (2*(j+((k++)*sz4)))+1));
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						{g_free(str); g_free(str2);}
					}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
					g_free(fout);
					break;
					case 1:
					str2=g_strdup(_("INVERSE_D\tREAL_VAL \tIMAG_VAL "));
pt=GTK_PLOT(plot2);
plt=GTK_PLOT_LINEAR(plot2);
sz4=g_array_index((pt->sizes), gint, 0);
jdimx=(plt->xdata->len)/(2*sz4);
contents=g_strdup(str2);
for (k=1; k<jdimx; k++)
{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
g_snprintf(s1, 10, "%f", g_array_index(plt->ydata, gdouble, 0));
str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
for (k=1; k<jdimx; k++)
{
	g_snprintf(s1, 10, "%f", g_array_index(plt->ydata, gdouble, 2*k*sz4));
	str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
					for (j=1; j<sz4; j++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, 2*j));
						num=g_array_index(plt->ydata, gdouble, 2*j);
						num2=g_array_index(plt->ydata, gdouble, (2*j)+1);
						g_snprintf(s2, 10, "%f", num*cos(num2));
						g_snprintf(s3, 10, "%f", num*sin(num2));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<jdimx)
						{
							g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, 2*(j+(k*sz4))));
							num=g_array_index(plt->ydata, gdouble, 2*(j+(k*sz4)));
							num2=g_array_index(plt->ydata, gdouble, (2*(j+((k++)*sz4)))+1);
							g_snprintf(s2, 10, "%f", num*cos(num2));
							g_snprintf(s3, 10, "%f", num*sin(num2));
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						{g_free(str); g_free(str2);}
					}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
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
		if ((flags&PROC_TRS)!=0)
		{
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 1);
wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
{
	g_free(folr);
	folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
	fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				dialog=gtk_dialog_new_with_buttons(_("Parameter selection"), GTK_WINDOW(wfile), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Real/Imaginary"), 1, _("Magnitude/Phase"), 2, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
				cont=gtk_dialog_get_content_area(GTK_DIALOG (dialog));
				label=gtk_label_new(_("Select Parameter to save:"));
				gtk_container_add(GTK_CONTAINER(cont), label);
				switch (gtk_dialog_run(GTK_DIALOG(dialog)))
				{
					case 2:
					str2=g_strdup(_("INVERSE_D\tMAGNITUDE\tPHASE    "));
pt=GTK_PLOT(plot2);
plt=GTK_PLOT_LINEAR(plot2);
sz4=g_array_index((pt->sizes), gint, 0);
jdimx=(plt->xdata->len)/(2*sz4);
contents=g_strdup(str2);
for (k=1; k<jdimx; k++)
{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
g_snprintf(s1, 10, "%f", g_array_index(plt->ydata, gdouble, 0));
str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
for (k=1; k<jdimx; k++)
{
	g_snprintf(s1, 10, "%f", g_array_index(plt->ydata, gdouble, 2*k*sz4));
	str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
					for (j=1; j<sz4; j++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, 2*j));
						g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, 2*j));
						g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, (2*j)+1));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<jdimx)
						{
							g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, 2*(j+(k*sz4))));
							g_snprintf(s2, 10, "%f", g_array_index(plt->ydata, gdouble, 2*(j+(k*sz4))));
							g_snprintf(s3, 10, "%f", g_array_index(plt->ydata, gdouble, (2*(j+((k++)*sz4)))+1));
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						{g_free(str); g_free(str2);}
					}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
					g_free(fout);
					break;
					case 1:
					str2=g_strdup(_("INVERSE_D\tREAL_VAL \tIMAG_VAL "));
pt=GTK_PLOT(plot2);
plt=GTK_PLOT_LINEAR(plot2);
sz4=g_array_index((pt->sizes), gint, 0);
jdimx=(plt->xdata->len)/(2*sz4);
contents=g_strdup(str2);
for (k=1; k<jdimx; k++)
{
	str=g_strjoin("\t", contents, str2, NULL);
	g_free(contents);
	contents=g_strdup(str);
	g_free(str);
}
g_free(str2);
g_snprintf(s1, 10, "%f", g_array_index(plt->ydata, gdouble, 0));
str2=g_strjoin("\t", "0.0000000", s1, "0.0000000", NULL);
for (k=1; k<jdimx; k++)
{
	g_snprintf(s1, 10, "%f", g_array_index(plt->ydata, gdouble, 2*k*sz4));
	str=g_strjoin("\t", str2, "0.0000000", s1, "0.0000000", NULL);
	g_free(str2);
	str2=g_strdup(str);
	g_free(str);
}
str=g_strdup(contents);
g_free(contents);
contents=g_strjoin(DLMT, str, str2, NULL);
{g_free(str); g_free(str2);}
					for (j=1; j<sz4; j++)
					{
						g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, 2*j));
						num=g_array_index(plt->ydata, gdouble, 2*j);
						num2=g_array_index(plt->ydata, gdouble, (2*j)+1);
						g_snprintf(s2, 10, "%f", num*cos(num2));
						g_snprintf(s3, 10, "%f", num*sin(num2));
						str2=g_strjoin("\t", s1, s2, s3, NULL);
						k=1;
						while (k<jdimx)
						{
							g_snprintf(s1, 10, "%f", g_array_index(plt->xdata, gdouble, 2*(j+(k*sz4))));
							num=g_array_index(plt->ydata, gdouble, 2*(j+(k*sz4)));
							num2=g_array_index(plt->ydata, gdouble, (2*(j+((k++)*sz4)))+1);
							g_snprintf(s2, 10, "%f", num*cos(num2));
							g_snprintf(s3, 10, "%f", num*sin(num2));
							str=g_strjoin("\t", str2, s1, s2, s3, NULL);
							g_free(str2);
							str2=g_strdup(str);
							g_free(str);
						}
						str=g_strdup(contents);
						g_free(contents);
						contents=g_strjoin(DLMT, str, str2, NULL);
						{g_free(str); g_free(str2);}
					}
g_file_set_contents(fout, contents, -1, &Err);
g_free(contents);
if (Err)
{
	str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
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

void sesssav(GtkWidget *widget, gpointer data)
{
	gchar *fout, *pdr, *str;
	GError *Err=NULL;
	gint fv;
	GKeyFile *key;
	gsize size;
	GSList *list;
	GtkPlot *pt;
	GtkPlotLinear *plt;

	pdr=g_build_filename(g_get_user_config_dir(), PACKAGE, NULL);
	if (g_mkdir_with_parents (pdr, 0700) == 0)
	{
		key=g_key_file_new();
		list=group;
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data)))
		{
			g_key_file_set_string(key, "MenuItems", "Device", "RawDelimited");
			goto bottomofcheck1a;
		}
		list=(list->next);
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data)))
		{
			g_key_file_set_string(key, "MenuItems", "Device", "JDSUSWS");
			goto bottomofcheck1a;
		}
		list=(list->next);
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data)))
		{
			g_key_file_set_string(key, "MenuItems", "Device", "AndoOSA");
			goto bottomofcheck1a;
		}
		list=(list->next);
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data))) g_key_file_set_string(key, "MenuItems", "Device", "AgilentTL");
		else g_key_file_set_string(key, "MenuItems", "Device", "AgilentOSA");
bottomofcheck1a:
		{fv=0; list=group2;}
		fv=g_slist_length(group2);
		g_key_file_set_integer(key, "MenuItems", "Traces", fv);
		while (list)
		{
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data)))
			{
				g_key_file_set_integer(key, "MenuItems", "Trace", fv);
				break;
			}
			{list=(list->next); fv--;}
		}
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(group3->data))) g_key_file_set_string(key, "MenuItems", "Compensation", "FirstOrder");
		else g_key_file_set_string(key, "MenuItems", "Compensation", "None");
		list=group4;
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data)))
		{
			g_key_file_set_string(key, "MenuItems", "Basis", "Morlet");
			goto bottomofcheck4a;
		}
		list=(list->next);
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data)))
		{
			g_key_file_set_string(key, "MenuItems", "Basis", "Meyer");
			goto bottomofcheck4a;
		}
		list=(list->next);
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data)))
		{
			g_key_file_set_string(key, "MenuItems", "Basis", "Daubechies8");
			goto bottomofcheck4a;
		}
		list=(list->next);
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data))) g_key_file_set_string(key, "MenuItems", "Basis", "Daubechies4");
		else g_key_file_set_string(key, "MenuItems", "Basis", "Fourier");
bottomofcheck4a:
		list=group5;
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data)))
		{
			g_key_file_set_string(key, "MenuItems", "Complex", "RealImaginary");
			goto bottomofcheck5a;
		}
		list=(list->next);
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data))) g_key_file_set_string(key, "MenuItems", "Complex", "MagnitudePhase");
		else g_key_file_set_string(key, "MenuItems", "Complex", "MagnitudeOnly");
bottomofcheck5a:
		g_key_file_set_boolean(key, "MenuItems", "Transmission", gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)));
		g_key_file_set_boolean(key, "MenuItems", "Negated", gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg)));
		g_key_file_set_boolean(key, "MenuItems", "dBs", gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)));
		g_key_file_set_boolean(key, "MenuItems", "Wavenumber", gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(twopionx)));
		g_key_file_set_boolean(key, "MenuItems", "EdgeCorrection", gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(wll)));
		g_key_file_set_boolean(key, "MenuItems", "OptimisedTriangles", gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(opttri)));
		g_key_file_set_boolean(key, "MenuItems", "Chirp", gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(chi)));
		g_key_file_set_boolean(key, "MenuItems", "OffsetTracking", gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(oft)));
		g_key_file_set_double(key, "Params", "Offset", gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst)));
		g_key_file_set_integer(key, "Params", "ZeroPadding", gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(zpd)));
		g_key_file_set_integer(key, "Params", "j", gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(jind)));
		g_key_file_set_integer(key, "Params", "k", gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(kind)));
		g_key_file_set_integer(key, "Param", "kMax", kdimxf);
		g_key_file_set_string(key, "Params", "DataFolder", fold);
		g_key_file_set_string(key, "Params", "ResultsFolder", folr);
		plt=GTK_PLOT_LINEAR(plot1);
		pt=GTK_PLOT(plot1);
		str=g_strdup(plt->xlab);
		g_key_file_set_string(key, "Plot", "SpectrumTextX", str);
		g_free(str);
		str=g_strdup(plt->ylab);
		g_key_file_set_string(key, "Plot", "SpectrumTextX", str);
		g_free(str);
		str=pango_font_description_to_string(pt->lfont);
		g_key_file_set_string(key, "Plot", "SpectrumLabel", str);
		g_free(str);
		str=pango_font_description_to_string(pt->afont);
		g_key_file_set_string(key, "Plot", "SpectrumAxis", str);
		g_free(str);
		g_key_file_set_double(key, "Plot", "SpectrumRed", g_array_index((pt->rd), gdouble, 0));
		g_key_file_set_double(key, "Plot", "SpectrumGreen", g_array_index((pt->gr), gdouble, 0));
		g_key_file_set_double(key, "Plot", "SpectrumBlue", g_array_index((pt->bl), gdouble, 0));
		plt=GTK_PLOT_LINEAR(plot2);
		pt=GTK_PLOT(plot2);
		str=g_strdup(plt->xlab);
		g_key_file_set_string(key, "Plot", "InverseSpectrumTextX", str);
		g_free(str);
		str=g_strdup(plt->ylab);
		g_key_file_set_string(key, "Plot", "InverseSpectrumTextX", str);
		g_free(str);
		str=pango_font_description_to_string(pt->lfont);
		g_key_file_set_string(key, "Plot", "InverseSpectrumLabel", str);
		g_free(str);
		str=pango_font_description_to_string(pt->afont);
		g_key_file_set_string(key, "Plot", "InverseSpectrumAxis", str);
		g_free(str);
		str=g_key_file_to_data(key, &size, NULL);
		fout=g_build_filename(pdr, "session.conf", NULL);
		g_file_set_contents(fout, str, size, &Err);
		{g_free(str); g_free(fout); g_key_file_free(key);}
		if (Err)
		{
			str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			{g_free(str); g_error_free(Err);}
		}
	}
	else
	{
		str=g_strdup(_("Error Creating config directory."));
		gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
		g_free(str);
	}
	g_free(pdr);
}

void sessres(GtkWidget *widget, gpointer data)
{
	GArray *caa, *cab, *cag, *car;
	gchar *str, *str2, *fout;
	gchar s[5];
	gdouble mny, mxy, val, xf, xi;
	GError *Err=NULL;
	gint j, k;
	GKeyFile *key;
	GSList *list;
	GtkPlot *pt;
	GtkPlotLinear *plt;
	GtkWidget *mni;
	PangoFontDescription *ds1, *ds2;

	key=g_key_file_new();
	fout=g_build_filename(g_get_user_config_dir(), PACKAGE, "session.conf", NULL);
	if (g_key_file_load_from_file(key, fout, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, &Err))
	{
		if (g_key_file_has_key(key, "MenuItems", "Device", &Err))
		{
			str=g_key_file_get_string(key, "MenuItems", "Device", &Err);
			list=group;
			if (!g_strcmp0(str, "RawDelimited"))
			{
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(list->data), TRUE);
				goto bottomofcheck1b;
			}
			else list=list->next;
			if (!g_strcmp0(str, "JDSUSWS"))
			{
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(list->data), TRUE);
				goto bottomofcheck1b;
			}
			else list=list->next;
			if (!g_strcmp0(str, "AndoOSA"))
			{
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(list->data), TRUE);
				goto bottomofcheck1b;
			}
			else list=list->next;
			if (!g_strcmp0(str, "AgilentTL")) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(list->data), TRUE);
			else if (!g_strcmp0(str, "AgilentOSA"))
			{
				list=list->next;
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(list->data), TRUE);
			}
bottomofcheck1b:
			g_free(str);
		}
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "MenuItems", "Traces", &Err))
		{
			k=g_key_file_get_integer(key, "MenuItems", "Traces", &Err);
			j=g_slist_length(group2);
			while (j<k)
			{
				g_snprintf(s, 4, "%d", (++j));
				mni=gtk_radio_menu_item_new_with_label(group2, s);
				group2=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mni));
				g_signal_connect(G_OBJECT(mni), "toggled", G_CALLBACK(upg), NULL);
				gtk_menu_shell_append(GTK_MENU_SHELL(tracmenu), mni);
				gtk_widget_show(mni);
			}
			while (j>k)
			{
				list=(group2->next);
				gtk_widget_destroy(group2->data);
				group2=list;
				j--;
			}
		}
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "MenuItems", "Trace", &Err))
		{
			j=g_key_file_get_integer(key, "MenuItems", "Trace", &Err);
			if (j>=g_slist_length(group2)) j=0;
			else j=g_slist_length(group2)-j;
			list=group2;
			while (j>0) {list=list->next; j--;}
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(list->data), TRUE);
		}
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "MenuItems", "Compensation", &Err))
		{
			str=g_key_file_get_string(key, "MenuItems", "Compensation", &Err);
			if (!g_strcmp0(str, "FirstOrder")) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(group3->data), TRUE);
			else
			{
				list=group3->next;
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(list->data), TRUE);
			}
			g_free(str);
		}
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "MenuItems", "Basis", &Err))
		{
			str=g_key_file_get_string(key, "MenuItems", "Basis", &Err);
			list=group4;
			if (!g_strcmp0(str, "Morlet"))
			{
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(list->data), TRUE);
				goto bottomofcheck4b;
			}
			else list=list->next;
			if (!g_strcmp0(str, "Meyer"))
			{
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(list->data), TRUE);
				goto bottomofcheck4b;
			}
			else list=list->next;
			if (!g_strcmp0(str, "Daubechies8"))
			{
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(list->data), TRUE);
				goto bottomofcheck4b;
			}
			else list=list->next;
			if (!g_strcmp0(str, "Daubechies4")) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(list->data), TRUE);
			else if (!g_strcmp0(str, "Fourier"))
			{
				list=list->next;
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(list->data), TRUE);
			}
bottomofcheck4b:
			g_free(str);
		}
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "MenuItems", "Complex", &Err))
		{
			str=g_key_file_get_string(key, "MenuItems", "Complex", &Err);
			list=group5;
			if (!g_strcmp0(str, "RealImaginary"))
			{
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(list->data), TRUE);
				goto bottomofcheck5b;
			}
			else list=list->next;
			if (!g_strcmp0(str, "MagnitudePhase")) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(list->data), TRUE);
			else if (!g_strcmp0(str, "MagnitudeOnly"))
			{
				list=list->next;
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(list->data), TRUE);
			}
bottomofcheck5b:
			g_free(str);
		}
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "MenuItems", "Transmission", &Err)) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(trans), g_key_file_get_boolean(key, "MenuItems", "Transmission", &Err));
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "MenuItems", "Negated", &Err)) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(neg), g_key_file_get_boolean(key, "MenuItems", "Negated", &Err));
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "MenuItems", "dBs", &Err)) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(dBs), g_key_file_get_boolean(key, "MenuItems", "dBs", &Err));
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "MenuItems", "Wavenumber", &Err)) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(twopionx), g_key_file_get_boolean(key, "MenuItems", "Wavenumber", &Err));
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "MenuItems", "EdgeCorrection", &Err)) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wll), g_key_file_get_boolean(key, "MenuItems", "EdgeCorrection", &Err));
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "MenuItems", "OptimisedTriangles", &Err)) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(opttri), g_key_file_get_boolean(key, "MenuItems", "OptimisedTriangles", &Err));
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "MenuItems", "Chirp", &Err)) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(chi), g_key_file_get_boolean(key, "MenuItems", "Chirp", &Err));
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "MenuItems", "OffsetTracking", &Err)) gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(oft), g_key_file_get_boolean(key, "MenuItems", "OffsetTracking", &Err));
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "Params", "Offset", &Err)) gtk_spin_button_set_value(GTK_SPIN_BUTTON(fst), g_key_file_get_double(key, "Params", "Offset", &Err));
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "Params", "ZeroPadding", &Err)) gtk_spin_button_set_value(GTK_SPIN_BUTTON(zpd), g_key_file_get_integer(key, "Params", "ZeroPadding", &Err));
		else if (Err) g_error_free(Err);
		/*if (g_key_file_has_key(key, "Params", "j", &Err)) gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind), g_key_file_get_integer(key, "Params", "j", &Err));
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "Params", "k", &Err)) gtk_spin_button_set_value(GTK_SPIN_BUTTON(kind), g_key_file_get_integer(key, "Params", "k", &Err));
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "Params", "kMax", &Err)) kdimxf=g_key_file_get_integer(key, "Params", "kdimxf", &Err);
		else if (Err) g_error_free(Err);*/
		if (g_key_file_has_key(key, "Params", "DataFolder", &Err))
		{
			g_free(fold);
			fold=g_strdup(g_key_file_get_string(key, "Params", "DataFolder", &Err));
		}
		else if (Err) g_error_free(Err);
		if (g_key_file_has_key(key, "Params", "ResultsFolder", &Err))
		{
			g_free(folr);
			folr=g_strdup(g_key_file_get_string(key, "Params", "ResultsFolder", &Err));
		}
		else if (Err) g_error_free(Err);
		plt=GTK_PLOT_LINEAR(plot1);
		pt=GTK_PLOT(plot1);
		if (g_key_file_has_key(key, "Plot", "SpectrumTextX", &Err)) str=g_key_file_get_string(key, "Plot", "SpectrumTextX", &Err);
		else
		{
			str=g_strdup(plt->xlab);
			if (Err) g_error_free(Err);
		}
		if (g_key_file_has_key(key, "Plot", "SpectrumTextY", &Err)) str2=g_key_file_get_string(key, "Plot", "SpectrumTextY", &Err);
		else
		{
			str2=g_strdup(plt->ylab);
			if (Err) g_error_free(Err);
		}
		gtk_plot_linear_set_label(plt, str, str2);
		{g_free(str); g_free(str2);}
		if (g_key_file_has_key(key, "Plot", "SpectrumLabel", &Err)) ds1=pango_font_description_from_string(g_key_file_get_string(key, "Plot", "SpectrumLabel", &Err));
		else
		{
			ds1=pango_font_description_copy(pt->lfont);
			if (Err) g_error_free(Err);
		}
		if (g_key_file_has_key(key, "Plot", "SpectrumAxis", &Err)) ds2=pango_font_description_from_string(g_key_file_get_string(key, "Plot", "SpectrumAxis", &Err));
		else
		{
			ds2=pango_font_description_copy(pt->afont);
			if (Err) g_error_free(Err);
		}
		gtk_plot_set_font(pt, ds1, ds2);
		{pango_font_description_free(ds1); pango_font_description_free(ds2);}
		if (g_key_file_has_key(key, "Plot", "SpectrumRed", &Err)) val=g_key_file_get_double(key, "Plot", "SpectrumRed", &Err);
		else
		{
			val=g_array_index((pt->rd), gdouble, 0);
			if (Err) g_error_free(Err);
		}
		car=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 1);
		g_array_append_val(car, val);
		if (g_key_file_has_key(key, "Plot", "SpectrumGreen", &Err)) val=g_key_file_get_double(key, "Plot", "SpectrumGreen", &Err);
		else
		{
			val=g_array_index((pt->gr), gdouble, 0);
			if (Err) g_error_free(Err);
		}
		cag=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 1);
		g_array_append_val(cag, val);
		if (g_key_file_has_key(key, "Plot", "SpectrumBlue", &Err)) val=g_key_file_get_double(key, "Plot", "SpectrumBlue", &Err);
		else
		{
			val=g_array_index((pt->bl), gdouble, 0);
			if (Err) g_error_free(Err);
		}
		cab=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 1);
		g_array_append_val(cab, val);
		val=1;
		caa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 1);
		g_array_append_val(caa, val);
		gtk_plot_set_colour(pt, car, cag, cab, caa);
		{g_array_unref(car); g_array_unref(cag); g_array_unref(cab); g_array_unref(caa);}
		g_object_get(G_OBJECT(plot1), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
		gtk_plot_linear_update_scale(plot1, xi, xf, mny, mxy);
		plt=GTK_PLOT_LINEAR(plot2);
		pt=GTK_PLOT(plot2);
		if (g_key_file_has_key(key, "Plot", "InverseSpectrumTextX", &Err)) str=g_key_file_get_string(key, "Plot", "InverseSpectrumTextX", &Err);
		else
		{
			str=g_strdup(plt->xlab);
			if (Err) g_error_free(Err);
		}
		if (g_key_file_has_key(key, "Plot", "InverseSpectrumTextY", &Err)) str2=g_key_file_get_string(key, "Plot", "InverseSpectrumTextY", &Err);
		else
		{
			str2=g_strdup(plt->ylab);
			if (Err) g_error_free(Err);
		}
		gtk_plot_linear_set_label(plt, str, str2);
		{g_free(str); g_free(str2);}
		if (g_key_file_has_key(key, "Plot", "InverseSpectrumLabel", &Err)) ds1=pango_font_description_from_string(g_key_file_get_string(key, "Plot", "InverseSpectrumLabel", &Err));
		else
		{
			ds1=pango_font_description_copy(pt->lfont);
			if (Err) g_error_free(Err);
		}
		if (g_key_file_has_key(key, "Plot", "InverseSpectrumAxis", &Err)) ds2=pango_font_description_from_string(g_key_file_get_string(key, "Plot", "InverseSpectrumAxis", &Err));
		else
		{
			ds2=pango_font_description_copy(pt->afont);
			if (Err) g_error_free(Err);
		}
		gtk_plot_set_font(pt, ds1, ds2);
		{pango_font_description_free(ds1); pango_font_description_free(ds2);}
		g_object_get(G_OBJECT(plot2), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
		gtk_plot_linear_update_scale(plot2, xi, xf, mny, mxy);
		g_key_file_free(key);
	}
	else
	{
		str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fout, (Err->message));
		gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
		{g_free(str); g_error_free(Err);}
	}
	g_free(fout);
}

void opd(GtkWidget *widget, gpointer data)
{
	GArray *nx, *st, *sz, *x, *y;
	gchar *contents=NULL, *fin=NULL, *str;
	gchar **strat=NULL, **strary=NULL;
	gchar s[5];
	gdouble lcl, mny, mxy, xf, xi;
	GError *Err=NULL;
	gint j, k, l, lc, sal, satl, zp;
	GSList *list;
	GtkPlot *pt;
	GtkPlotLinear *plt;
	GtkWidget *label, *mni, *wfile;

	wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(wfile), FALSE);
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(wfile), FALSE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), fold);
	g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
	if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
	{
		{g_free(fold); g_free(folr);}
		fold=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
		folr=g_strdup(fold);
		fin=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
		if (g_file_get_contents(fin, &contents, NULL, &Err))
		{
			strary=g_strsplit_set(contents, "\r\n", 0);
			sal=g_strv_length(strary);
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
			flags|=PROC_OPN;
			x=g_array_new(FALSE, FALSE, sizeof(gdouble));
			y=g_array_new(FALSE, FALSE, sizeof(gdouble));
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(anosa))) {k=2; lc=-1;}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(sws))) {k=0; lc=-1;}
			else {k=0; lc=0;}
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mg)))
			{
				while (k<sal)
				{
					if (!strary[k]) {k++; continue;}
					g_strchug(strary[k]);
					if (!g_strcmp0("", strary[k])) {k++; continue;}
					if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) {k++; continue;}
					if (lc<0) {lc++; k++; continue;}
					strat=g_strsplit_set(strary[k], "\t", 0);
					lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
					g_array_append_val(x, lcl);
					if (lc==0)
					{
						satl=g_strv_length(strat);
						if (!strat[1]) lcl=0;
						else lcl=g_ascii_strtod(g_strstrip(strat[1]), NULL);
						{mny=lcl; mxy=lcl;}
					}
					else
					{
						if (!strat[1]) lcl=0;
						else lcl=g_ascii_strtod(g_strstrip(strat[1]), NULL);
						if (lcl<mny) mny=lcl;
						else if (lcl>mxy) mxy=lcl;
					}
					g_array_append_val(y, lcl);
					for (l=2; l<satl; l++)
					{
						if (!strat[l]) lcl=0;
						else lcl=g_ascii_strtod(g_strstrip(strat[l]), NULL);
						g_array_append_val(y, lcl);
					}
					g_strfreev(strat);
					{lc++; k++;}
				}
				g_strfreev(strary);
				satl--;
				j=g_slist_length(group2);
				while (j<satl)
				{
					g_snprintf(s, 4, "%d", (++j));
					mni=gtk_radio_menu_item_new_with_label(group2, s);
					group2=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mni));
					g_signal_connect(G_OBJECT(mni), "toggled", G_CALLBACK(upg), NULL);
					gtk_menu_shell_append(GTK_MENU_SHELL(tracmenu), mni);
					gtk_widget_show(mni);
				}
				while (j>satl)
				{
					list=(group2->next);
					gtk_widget_destroy(group2->data);
					group2=list;
					j--;
				}
				str=g_strdup_printf(_("File: %s successfully loaded."), fin);
				gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
				g_free(str);
				plt=GTK_PLOT_LINEAR(plot1);
				xi=g_array_index(x, gdouble, 0);
				xf=g_array_index(x, gdouble, (lc-1));
				{st=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1); nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1);}
				g_array_append_val(st, satl);
				g_array_append_val(sz, lc);/* adjust if multiple traces desired */
				zp=0;
				g_array_append_val(nx, zp);
				gtk_plot_linear_set_data(plt, x, y, nx, sz, st);
				{g_array_unref(x); g_array_unref(y); g_array_unref(nx); g_array_unref(sz); g_array_unref(st);}
				gtk_plot_linear_update_scale_pretty(plot1, xi, xf, mny, mxy);
				flags&=(PROC_OPN|PROC_TRS|PROC_PRS|PROC_CHP|PROC_OFT|PROC_POL);
			}
			else
			{
				str=g_strdup("Complex data not yet supported.");
				gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
				g_free(str);
				flags|=PROC_COM;
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(ri))) flags|=PROC_RI;
				else flags&=(PROC_OPN|PROC_TRS|PROC_PRS|PROC_CHP|PROC_OFT|PROC_POL|PROC_COM);
			}
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);
			gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 0);
		}
		else
		{
			str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			{g_free(str); g_error_free(Err);}
		}
		{g_free(contents); g_free(fin);}
	}
	gtk_widget_destroy(wfile);
}
