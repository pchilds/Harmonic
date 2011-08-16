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
						if ((flags&PROC_POL)!=0) plot_polar_print_eps(plot3, fout);
						else plot_linear_print_eps(plot3, fout);
					}
					else
					{
						fout2=g_strconcat(fout, ".eps", NULL);
						if ((flags&PROC_POL)!=0) plot_polar_print_eps(plot3, fout2);
						else plot_linear_print_eps(plot3, fout2);
						g_free(fout2);
					}
				}
				else if (filt==svgfilt)
				{
					if (g_str_has_suffix(fout, ".svg"))
					{
						if ((flags&PROC_POL)!=0) plot_polar_print_svg(plot3, fout);
						else plot_linear_print_svg(plot3, fout);
					}
					else
					{
						fout2=g_strconcat(fout, ".svg", NULL);
						if ((flags&PROC_POL)!=0) plot_polar_print_svg(plot3, fout2);
						else plot_linear_print_svg(plot3, fout2);
						g_free(fout2);
					}
				}
				else if (g_str_has_suffix(fout, ".png"))
				{
					if ((flags&PROC_POL)!=0) plot_polar_print_png(plot3, fout);
					else plot_linear_print_png(plot3, fout);
				}
				else
				{
					fout2=g_strconcat(fout, ".png", NULL);
					if ((flags&PROC_POL)!=0) plot_polar_print_png(plot3, fout2);
					else plot_linear_print_png(plot3, fout2);
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
					if (g_str_has_suffix(fout, ".eps")) plot_linear_print_eps(plot2, fout);
					else
					{
						fout2=g_strconcat(fout, ".eps", NULL);
						plot_linear_print_eps(plot2, fout2);
						g_free(fout2);
					}
				}
				else if (filt==svgfilt)
				{
					if (g_str_has_suffix(fout, ".svg")) plot_linear_print_svg(plot2, fout);
					else
					{
						fout2=g_strconcat(fout, ".svg", NULL);
						plot_linear_print_svg(plot2, fout2);
						g_free(fout2);
					}
				}
				else if (g_str_has_suffix(fout, ".png")) plot_linear_print_png(plot2, fout);
				else
				{
					fout2=g_strconcat(fout, ".png", NULL);
					plot_linear_print_png(plot2, fout2);
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
					if (g_str_has_suffix(fout, ".eps")) plot_linear_print_eps(plot1, fout);
					else
					{
						fout2=g_strconcat(fout, ".eps", NULL);
						plot_linear_print_eps(plot1, fout2);
						g_free(fout2);
					}
				}
				else if (filt==svgfilt)
				{
					if (g_str_has_suffix(fout, ".svg")) plot_linear_print_svg(plot1, fout);
					else
					{
						fout2=g_strconcat(fout, ".svg", NULL);
						plot_linear_print_svg(plot1, fout2);
						g_free(fout2);
					}
				}
				else if (g_str_has_suffix(fout, ".png")) plot_linear_print_png(plot1, fout);
				else
				{
					fout2=g_strconcat(fout, ".png", NULL);
					plot_linear_print_png(plot1, fout2);
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
					if (g_str_has_suffix(fout, ".eps")) plot_linear_print_eps(plot2, fout);
					else
					{
						fout2=g_strconcat(fout, ".eps", NULL);
						plot_linear_print_eps(plot2, fout2);
						g_free(fout2);
					}
				}
				else if (filt==svgfilt)
				{
					if (g_str_has_suffix(fout, ".svg")) plot_linear_print_svg(plot2, fout);
					else
					{
						fout2=g_strconcat(fout, ".svg", NULL);
						plot_linear_print_svg(plot2, fout2);
						g_free(fout2);
					}
				}
				else if (g_str_has_suffix(fout, ".png")) plot_linear_print_png(plot2, fout);
				else
				{
					fout2=g_strconcat(fout, ".png", NULL);
					plot_linear_print_png(plot2, fout2);
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
					if (g_str_has_suffix(fout, ".eps")) plot_linear_print_eps(plot1, fout);
					else
					{
						fout2=g_strconcat(fout, ".eps", NULL);
						plot_linear_print_eps(plot1, fout2);
						g_free(fout2);
					}
				}
				else if (filt==svgfilt)
				{
					if (g_str_has_suffix(fout, ".svg")) plot_linear_print_svg(plot1, fout);
					else
					{
						fout2=g_strconcat(fout, ".svg", NULL);
						plot_linear_print_svg(plot1, fout2);
						g_free(fout2);
					}
				}
				else if (g_str_has_suffix(fout, ".png")) plot_linear_print_png(plot1, fout);
				else
				{
					fout2=g_strconcat(fout, ".png", NULL);
					plot_linear_print_png(plot1, fout2);
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
					if (g_str_has_suffix(fout, ".eps")) plot_linear_print_eps(plot1, fout);
					else
					{
						fout2=g_strconcat(fout, ".eps", NULL);
						plot_linear_print_eps(plot1, fout2);
						g_free(fout2);
					}
				}
				else if (filt==svgfilt)
				{
					if (g_str_has_suffix(fout, ".svg")) plot_linear_print_svg(plot1, fout);
					else
					{
						fout2=g_strconcat(fout, ".svg", NULL);
						plot_linear_print_svg(plot1, fout2);
						g_free(fout2);
					}
				}
				else if (g_str_has_suffix(fout, ".png")) plot_linear_print_png(plot1, fout);
				else
				{
					fout2=g_strconcat(fout, ".png", NULL);
					plot_linear_print_png(plot1, fout2);
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
								str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
								str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
								str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
								str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
								str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
								str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
								str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
								str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
								str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
								str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
								str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
								str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
								str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
								str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
						str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
							str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
						str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						g_free(str);
						g_error_free(Err);
					}
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
						str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
						str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
						str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
						str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
						str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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
						str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
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

void sesssav(GtkWidget *widget, gpointer data)
{
	GtkWidget *wfile;
	GSList *list;
	gchar *contents, *contents2, *str, *fout=NULL;
	guint16 fv, fv2;
	GError *Err=NULL;

	wfile=gtk_file_chooser_dialog_new(_("Select Session File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
	if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
	{
		g_free(folr);
		folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
		str=g_strdup("#menuitems");
		{fv=0; list=group4;}/*need to ensure fv values don't conflict with DLMT, \t \0 etc */
		while (list)
		{
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data))) break;
			{list=(list->next); fv++;}
		}
		{fv<<=2; list=group5;}
		while (list)
		{
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data))) break;
			{list=(list->next); fv++;}
		}
		{fv<<=2; list=group3;}
		while (list)
		{
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data))) break;
			{list=(list->next); fv++;}
		}
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) fv|=256;
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) fv|=512;
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs))) fv|=1024;
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(twopionx))) fv|=2048;
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(wll))) fv|=4096;
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(opttri))) fv|=8192;
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(chi))) fv|=16384;
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(oft))) fv|=32768;
		{fv2=0; list=group2;}
		while (list)
		{
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data))) break;
			{list=(list->next); fv2++;}
		}
		{fv2<<=8; list=group;}
		while (list)
		{
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data))) break;
			{list=(list->next); fv2++;}
		}
		str=g_strjoin("\t", str, (gchar) fv, (gchar) fv2, NULL);
		contents2=g_strdup(contents);
		g_free(contents);
		contents=g_strjoin(DLMT, contents2, str, NULL);
		g_free(str);
		fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
		g_file_set_contents(fout, contents, -1, &Err);
		g_free(contents);
		if (Err)
		{
			str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
			g_error_free(Err);
		}
		g_free(contents);
		g_free(fout);
	}
	gtk_widget_destroy(wfile);
}

void sessres(GtkWidget *widget, gpointer data)
{
	GtkWidget *wfile;
	gchar *contents, *str, *fin=NULL;
	GError *Err=NULL;

	wfile=gtk_file_chooser_dialog_new(_("Select Session File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
	if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
	{
		g_free(folr);
		folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
		fin=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
		if (g_file_get_contents(fin, &contents, NULL, &Err))
		{
		}
		else
		{
			str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
			gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
			g_free(str);
			g_error_free(Err);
		}
		g_free(contents);
		g_free(fin);
	}
	gtk_widget_destroy(wfile);
}

void opd(GtkWidget *widget, gpointer data)
{
	GtkWidget *wfile, *label, *mni;
	GSList *list;
	gdouble xi, xf, mny, mxy, lcl;
	gint j, k, l, sal, zp;
	gchar *contents=NULL, *fin=NULL, *str;
	gchar **strat=NULL, **strary=NULL;
	gchar s[5];
	GError *Err=NULL;
	PlotLinear *plt;

	wfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(wfile), FALSE);
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(wfile), FALSE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), fold);
	g_signal_connect(G_OBJECT(wfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wfile));
	if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
	{
		g_free(fold);
		g_free(folr);
		fold=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
		folr=g_strdup(fold);
		fin=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
		if (g_file_get_contents(fin, &contents, NULL, &Err))
		{
			strary=g_strsplit_set(contents, "\r\n", 0);
			sal=g_strv_length(strary);
			{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(sz, TRUE); g_array_free(nx, TRUE); g_array_free(specs, TRUE);}
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
				bxr=g_array_new(FALSE, FALSE, sizeof(gdouble));
				byr=g_array_new(FALSE, FALSE, sizeof(gdouble));
				bsz=g_array_new(FALSE, FALSE, sizeof(gint));
				bnx=g_array_new(FALSE, FALSE, sizeof(gint));
				flags^=PROC_BAT;
			}
			flags|=PROC_OPN;
			x=g_array_new(FALSE, FALSE, sizeof(gdouble));
			yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
			specs=g_array_new(FALSE, FALSE, sizeof(gdouble));
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
				plt=PLOT_LINEAR(plot1);
				xi=g_array_index(x, gdouble, 0);
				xf=g_array_index(x, gdouble, (lc-1));
				sz=g_array_new(FALSE, FALSE, sizeof(gint));
				nx=g_array_new(FALSE, FALSE, sizeof(gint));
				g_array_append_val(sz, lc);/* adjust if multiple traces desired */
				(plt->sizes)=sz;
				zp=0;
				g_array_append_val(nx, zp);
				(plt->ind)=nx;
				(plt->xdata)=x;
				(plt->ydata)=yb;
				plot_linear_update_scale_pretty(plot1, xi, xf, mny, mxy);
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
			g_free(str);
			g_error_free(Err);
		}
		g_free(contents);
		g_free(fin);
	}
	gtk_widget_destroy(wfile);
}
