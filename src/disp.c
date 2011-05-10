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

void dpr(GtkWidget *widget, gpointer data)
{
	GtkWidget *helpwin, *content, *table, *vbox, *hbox, * hbox2, *colour, *entry1, *entry2, *label, *butt1, *butt2, *spr, *ck, *ck2, *ck3, *ck4, *jind3, *kind2;
	GtkAdjustment *adj;
	PlotLinear *plt;
	PlotPolar *plt2;
	gdouble xi, xf, mny, mxy, iv, clc, rcn, thc;
	gint dx, dx2, j, k, l, bl1, bl2, bl3;
	gchar *str;
	const gchar *ast, *lstr;
	
	helpwin=gtk_dialog_new_with_buttons(_("Display Properties"), GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL, GTK_STOCK_APPLY, GTK_RESPONSE_APPLY, NULL);
	g_signal_connect_swapped(G_OBJECT(helpwin), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(helpwin));
	gtk_widget_show(helpwin);
	content=gtk_dialog_get_content_area(GTK_DIALOG(helpwin));
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	vbox=gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	if ((flags&32)!=0)
	{
		table=gtk_table_new(4, 2, FALSE);
		gtk_widget_show(table);
		label=gtk_label_new(_("Axis label font:"));
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		label=gtk_label_new(_("Tick label font:"));
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		entry1=gtk_entry_new();
		entry2=gtk_entry_new();
		spr=gtk_hseparator_new();
		gtk_widget_show(spr);
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
		str=pango_font_description_to_string(plt2->lfont);
		butt1=gtk_font_button_new_with_font(str);
		g_free(str);
		gtk_font_button_set_show_style(GTK_FONT_BUTTON(butt1), TRUE);
		gtk_font_button_set_show_size(GTK_FONT_BUTTON(butt1), TRUE);
		gtk_font_button_set_use_font(GTK_FONT_BUTTON(butt1), TRUE);
		gtk_font_button_set_use_size(GTK_FONT_BUTTON(butt1), FALSE);
		gtk_font_button_set_title(GTK_FONT_BUTTON(butt1), _("Font Selection for Axis Labels"));
		str=pango_font_description_to_string(plt2->afont);
		butt2=gtk_font_button_new_with_font(str);
		g_free(str);
		gtk_font_button_set_show_style(GTK_FONT_BUTTON(butt2), TRUE);
		gtk_font_button_set_show_size(GTK_FONT_BUTTON(butt2), TRUE);
		gtk_font_button_set_use_font(GTK_FONT_BUTTON(butt2), TRUE);
		gtk_font_button_set_use_size(GTK_FONT_BUTTON(butt2), FALSE);
		gtk_font_button_set_title(GTK_FONT_BUTTON(butt2), _("Font Selection for Tick Mark Labels"));
		gtk_widget_show(entry1);
		gtk_widget_show(entry2);
		gtk_widget_show(butt1);
		gtk_widget_show(butt2);
		gtk_table_attach(GTK_TABLE(table), entry1, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_table_attach(GTK_TABLE(table), entry2, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_table_attach(GTK_TABLE(table), butt1, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_table_attach(GTK_TABLE(table), butt2, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), spr, FALSE, FALSE, 2);
		ck4=gtk_combo_box_new_text();
		gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Visibility"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Domain Shift"));
		if ((flags&16)!=0)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Chirp"));
			gtk_combo_box_set_active(GTK_COMBO_BOX(ck4), (flagd&24)/8);
		}
		else gtk_combo_box_set_active(GTK_COMBO_BOX(ck4), (flagd&8)/8);
		gtk_widget_show(ck4);
		gtk_box_pack_start(GTK_BOX(vbox), ck4, FALSE, FALSE, 2);
		table=gtk_table_new(2, 2, FALSE);
		gtk_widget_show(table);
		ck2=gtk_check_button_new_with_label(_("Multiple plots for Results over index j"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck2), (flagd&2));
		gtk_widget_show(ck2);
		gtk_table_attach(GTK_TABLE(table), ck2, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		adj=(GtkAdjustment *) gtk_adjustment_new(jdim, 0, jdimx-1, 1.0, 5.0, 0.0);
		jind3=gtk_spin_button_new(adj, 0, 0);
		g_signal_connect(G_OBJECT(jind3), "value-changed", G_CALLBACK(upj), NULL);
		gtk_widget_show(jind3);
		gtk_table_attach(GTK_TABLE(table), jind3, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		ck3=gtk_check_button_new_with_label(_("Multiple plots for Results over index k"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck3), (flagd&4));
		gtk_widget_show(ck3);
		gtk_table_attach(GTK_TABLE(table), ck3, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		adj=(GtkAdjustment *) gtk_adjustment_new(kdim, 0, kdimx-1, 1.0, 5.0, 0.0);
		kind2=gtk_spin_button_new(adj, 0, 0);
		g_signal_connect(G_OBJECT(kind2), "value-changed", G_CALLBACK(upk), NULL);
		gtk_widget_show(kind2);
		gtk_table_attach(GTK_TABLE(table), kind2, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
		gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 2);
		spr=gtk_vseparator_new();
		gtk_widget_show(spr);
		gtk_box_pack_start(GTK_BOX(hbox), spr, FALSE, FALSE, 2);
		colour=gtk_color_selection_new();
		gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour), TRUE);
		gtk_widget_show(colour);
		gtk_box_pack_start(GTK_BOX(hbox), colour, FALSE, FALSE, 2);
		gtk_container_add(GTK_CONTAINER(content), hbox);
		if (gtk_dialog_run(GTK_DIALOG(helpwin))==GTK_RESPONSE_APPLY)
		{
			g_free(plt2->rlab);
			g_free(plt2->thlab);
			pango_font_description_free(plt2->lfont);
			pango_font_description_free(plt2->afont);
			(plt2->rlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry1)));
			(plt2->thlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry2)));
			str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt1)));
			(plt2->lfont)=pango_font_description_from_string(str);
			g_free(str);
			str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt2)));
			(plt2->afont)=pango_font_description_from_string(str);
			g_free(str);
			g_object_get(G_OBJECT(plot3), "thmin", &xi, "thmax", &xf, "rmin", &mny, "rmax", &mxy, "rcnt", &rcn, "thcnt", &thc, NULL);
			bl1=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2));
			bl2=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3));
			bl3=gtk_combo_box_get_active(GTK_COMBO_BOX(ck4));
			if ((bl1==((flagd&2)==0))||(bl2==((flagd&4)==0))||((bl3*8)!=(flagd&24)))
			{
				g_array_free(bxr, TRUE);
				g_array_free(byr, TRUE);
				g_array_free(bsz, TRUE);
				g_array_free(bnx, TRUE);
				dx2=0;
				if (bl3==0)
				{
					if (bl1)
					{
						if (bl2)
						{
							bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf*jdimxf);
							byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf*jdimxf);
							bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf*jdimxf);
							bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf*jdimxf);
							iv=g_array_index(msr, gdouble, 0);
							g_array_append_val(bxr, iv);
							mny=g_array_index(vis, gdouble, 0);
							g_array_append_val(byr, mny);
							mxy=mny;
							for (j=1; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(vis, gdouble, (j*jdimxf*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
							for (k=1; k<kdimxf; k++)
							{
								for (j=0; j<mx; j++)
								{
									iv=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, iv);
									iv=g_array_index(vis, gdouble, k+(j*jdimxf*kdimxf));
									g_array_append_val(byr, iv);
									if (iv>mxy) mxy=iv;
									else if (iv<mny) mny=iv;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
							}
							for (l=1; l<jdimxf; l++)
							{
								for (k=0; k<kdimxf; k++)
								{
									for (j=0; j<mx; j++)
									{
										iv=g_array_index(msr, gdouble, j);
										g_array_append_val(bxr, iv);
										iv=g_array_index(vis, gdouble, k+((l+(j*jdimxf))*kdimxf));
										g_array_append_val(byr, iv);
										if (iv>mxy) mxy=iv;
										else if (iv<mny) mny=iv;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									dx2+=mx;
								}
							}
							flagd&=1;
							flagd|=6;
						}
						else
						{
							bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimxf);
							byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimxf);
							bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
							bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
							iv=g_array_index(msr, gdouble, 0);
							g_array_append_val(bxr, iv);
							mny=g_array_index(vis, gdouble, kdim);
							g_array_append_val(byr, mny);
							mxy=mny;
							for (j=1; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(vis, gdouble, kdim+(j*jdimxf*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
							for (k=1; k<jdimxf; k++)
							{
								for (j=0; j<mx; j++)
								{
									iv=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, iv);
									iv=g_array_index(vis, gdouble, kdim+((k+(j*jdimxf))*kdimxf));
									g_array_append_val(byr, iv);
									if (iv>mxy) mxy=iv;
									else if (iv<mny) mny=iv;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
							}
							flagd&=1;
							flagd|=2;
						}
					}
					else if (bl2)
					{
						bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf);
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf);
						bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
						iv=g_array_index(msr, gdouble, 0);
						g_array_append_val(bxr, iv);
						mny=g_array_index(vis, gdouble, (jdim*kdimxf));
						g_array_append_val(byr, mny);
						mxy=mny;
						for (j=1; j<mx; j++)
						{
							iv=g_array_index(msr, gdouble, j);
							g_array_append_val(bxr, iv);
							iv=g_array_index(vis, gdouble, ((jdim+(j*jdimxf))*kdimxf));
							g_array_append_val(byr, iv);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						g_array_append_val(bsz, mx);
						g_array_append_val(bnx, dx2);
						dx2+=mx;
						for (k=1; k<kdimxf; k++)
						{
							for (j=0; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(vis, gdouble, k+((jdim+(j*jdimxf))*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
						}
						flagd&=1;
						flagd|=4;
					}
					else
					{
						bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
						bsz=g_array_new(FALSE, FALSE, sizeof(gint));
						bnx=g_array_new(FALSE, FALSE, sizeof(gint));
						iv=g_array_index(msr, gdouble, 0);
						g_array_append_val(bxr, iv);
						mny=g_array_index(vis, gdouble, kdim+(jdim*kdimxf));
						g_array_append_val(byr, mny);
						mxy=mny;
						for (j=1; j<mx; j++)
						{
							iv=g_array_index(msr, gdouble, j);
							g_array_append_val(bxr, iv);
							iv=g_array_index(vis, gdouble, kdim+((jdim+(j*jdimxf))*kdimxf));
							g_array_append_val(byr, iv);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						g_array_append_val(bsz, mx);
						g_array_append_val(bnx, dx2);
						flagd&=1;
					}
				}
				else if (bl3==1)
				{
					if (bl1)
					{
						if (bl2)
						{
							bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf*jdimxf);
							byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf*jdimxf);
							bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf*jdimxf);
							bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf*jdimxf);
							iv=g_array_index(msr, gdouble, 0);
							g_array_append_val(bxr, iv);
							mny=g_array_index(doms, gdouble, 0);
							g_array_append_val(byr, mny);
							mxy=mny;
							for (j=1; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(doms, gdouble, (j*jdimxf*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
							for (k=1; k<kdimxf; k++)
							{
								for (j=0; j<mx; j++)
								{
									iv=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, iv);
									iv=g_array_index(doms, gdouble, k+(j*jdimxf*kdimxf));
									g_array_append_val(byr, iv);
									if (iv>mxy) mxy=iv;
									else if (iv<mny) mny=iv;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
							}
							for (l=1; l<jdimxf; l++)
							{
								for (k=0; k<kdimxf; k++)
								{
									for (j=0; j<mx; j++)
									{
										iv=g_array_index(msr, gdouble, j);
										g_array_append_val(bxr, iv);
										iv=g_array_index(doms, gdouble, k+((l+(j*jdimxf))*kdimxf));
										g_array_append_val(byr, iv);
										if (iv>mxy) mxy=iv;
										else if (iv<mny) mny=iv;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									dx2+=mx;
								}
							}
							flagd&=1;
							flagd|=14;
						}
						else
						{
							bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimxf);
							byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimxf);
							bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
							bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
							iv=g_array_index(msr, gdouble, 0);
							g_array_append_val(bxr, iv);
							mny=g_array_index(doms, gdouble, kdim);
							g_array_append_val(byr, mny);
							mxy=mny;
							for (j=1; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(doms, gdouble, kdim+(j*jdimxf*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
							for (k=1; k<jdimxf; k++)
							{
								for (j=0; j<mx; j++)
								{
									iv=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, iv);
									iv=g_array_index(doms, gdouble, kdim+((k+(j*jdimxf))*kdimxf));
									g_array_append_val(byr, iv);
									if (iv>mxy) mxy=iv;
									else if (iv<mny) mny=iv;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
							}
							flagd&=1;
							flagd|=10;
						}
					}
					else if (bl2)
					{
						bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf);
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf);
						bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
						iv=g_array_index(msr, gdouble, 0);
						g_array_append_val(bxr, iv);
						mny=g_array_index(doms, gdouble, (jdim*kdimxf));
						g_array_append_val(byr, mny);
						mxy=mny;
						for (j=1; j<mx; j++)
						{
							iv=g_array_index(msr, gdouble, j);
							g_array_append_val(bxr, iv);
							iv=g_array_index(doms, gdouble, ((jdim+(j*jdimxf))*kdimxf));
							g_array_append_val(byr, iv);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						g_array_append_val(bsz, mx);
						g_array_append_val(bnx, dx2);
						dx2+=mx;
						for (k=1; k<kdimxf; k++)
						{
							for (j=0; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(doms, gdouble, k+((jdim+(j*jdimxf))*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
						}
						flagd&=1;
						flagd|=12;
					}
					else
					{
						bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
						bsz=g_array_new(FALSE, FALSE, sizeof(gint));
						bnx=g_array_new(FALSE, FALSE, sizeof(gint));
						iv=g_array_index(msr, gdouble, 0);
						g_array_append_val(bxr, iv);
						mny=g_array_index(doms, gdouble, kdim+(jdim*kdimxf));
						g_array_append_val(byr, mny);
						mxy=mny;
						for (j=1; j<mx; j++)
						{
							iv=g_array_index(msr, gdouble, j);
							g_array_append_val(bxr, iv);
							iv=g_array_index(doms, gdouble, kdim+((jdim+(j*jdimxf))*kdimxf));
							g_array_append_val(byr, iv);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						g_array_append_val(bsz, mx);
						g_array_append_val(bnx, dx2);
						flagd&=1;
						flagd|=8;
					}
				}
				else if (bl1)
				{
					if (bl2)
					{
						bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf*jdimxf);
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf*jdimxf);
						bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf*jdimxf);
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf*jdimxf);
						iv=g_array_index(msr, gdouble, 0);
						g_array_append_val(bxr, iv);
						mny=g_array_index(chp, gdouble, 0);
						g_array_append_val(byr, mny);
						mxy=mny;
						for (j=1; j<mx; j++)
						{
							iv=g_array_index(msr, gdouble, j);
							g_array_append_val(bxr, iv);
							iv=g_array_index(chp, gdouble, (j*jdimxf*kdimxf));
							g_array_append_val(byr, iv);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						g_array_append_val(bsz, mx);
						g_array_append_val(bnx, dx2);
						dx2+=mx;
						for (k=1; k<kdimxf; k++)
						{
							for (j=0; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(chp, gdouble, k+(j*jdimxf*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
						}
						for (l=1; l<jdimxf; l++)
						{
							for (k=0; k<kdimxf; k++)
							{
								for (j=0; j<mx; j++)
								{
									iv=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, iv);
									iv=g_array_index(chp, gdouble, k+((l+(j*jdimxf))*kdimxf));
									g_array_append_val(byr, iv);
									if (iv>mxy) mxy=iv;
									else if (iv<mny) mny=iv;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
							}
						}
						flagd&=1;
						flagd|=22;
					}
					else
					{
						bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimxf);
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimxf);
						bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
						iv=g_array_index(msr, gdouble, 0);
						g_array_append_val(bxr, iv);
						mny=g_array_index(chp, gdouble, kdim);
						g_array_append_val(byr, mny);
						mxy=mny;
						for (j=1; j<mx; j++)
						{
							iv=g_array_index(msr, gdouble, j);
							g_array_append_val(bxr, iv);
							iv=g_array_index(chp, gdouble, kdim+(j*jdimxf*kdimxf));
							g_array_append_val(byr, iv);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						g_array_append_val(bsz, mx);
						g_array_append_val(bnx, dx2);
						dx2+=mx;
						for (k=1; k<jdimxf; k++)
						{
							for (j=0; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(chp, gdouble, kdim+((k+(j*jdimxf))*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
						}
						flagd&=1;
						flagd|=18;
					}
				}
				else if (bl2)
				{
					bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf);
					byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf);
					bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
					bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
					iv=g_array_index(msr, gdouble, 0);
					g_array_append_val(bxr, iv);
					mny=g_array_index(chp, gdouble, (jdim*kdimxf));
					g_array_append_val(byr, mny);
					mxy=mny;
					for (j=1; j<mx; j++)
					{
						iv=g_array_index(msr, gdouble, j);
						g_array_append_val(bxr, iv);
						iv=g_array_index(chp, gdouble, ((jdim+(j*jdimxf))*kdimxf));
						g_array_append_val(byr, iv);
						if (iv>mxy) mxy=iv;
						else if (iv<mny) mny=iv;
					}
					g_array_append_val(bsz, mx);
					g_array_append_val(bnx, dx2);
					dx2+=mx;
					for (k=1; k<kdimxf; k++)
					{
						for (j=0; j<mx; j++)
						{
							iv=g_array_index(msr, gdouble, j);
							g_array_append_val(bxr, iv);
							iv=g_array_index(chp, gdouble, k+((jdim+(j*jdimxf))*kdimxf));
							g_array_append_val(byr, iv);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						g_array_append_val(bsz, mx);
						g_array_append_val(bnx, dx2);
						dx2+=mx;
					}
					flagd&=1;
					flagd|=20;
				}
				else
				{
					bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
					byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
					bsz=g_array_new(FALSE, FALSE, sizeof(gint));
					bnx=g_array_new(FALSE, FALSE, sizeof(gint));
					iv=g_array_index(msr, gdouble, 0);
					g_array_append_val(bxr, iv);
					mny=g_array_index(chp, gdouble, kdim+(jdim*kdimxf));
					g_array_append_val(byr, mny);
					mxy=mny;
					for (j=1; j<mx; j++)
					{
						iv=g_array_index(msr, gdouble, j);
						g_array_append_val(bxr, iv);
						iv=g_array_index(chp, gdouble, kdim+((jdim+(j*jdimxf))*kdimxf));
						g_array_append_val(byr, iv);
						if (iv>mxy) mxy=iv;
						else if (iv<mny) mny=iv;
					}
					g_array_append_val(bsz, mx);
					g_array_append_val(bnx, dx2);
					flagd&=1;
					flagd|=16;
				}
				(plt2->thdata)=bxr;
				(plt2->rdata)=byr;
				(plt2->sizes)=bsz;
				(plt2->ind)=bnx;
			}
			plot_polar_update_scale(plot3, mny, mxy, xi, xf, rcn, thc);
		}
	}
	else if ((flags&8)!=0)
	{
		table=gtk_table_new(4, 2, FALSE);
		gtk_widget_show(table);
		label=gtk_label_new(_("Axis label font:"));
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		label=gtk_label_new(_("Tick label font:"));
		gtk_widget_show(label);
		gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		entry1=gtk_entry_new();
		entry2=gtk_entry_new();
		spr=gtk_hseparator_new();
		gtk_widget_show(spr);
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
		str=pango_font_description_to_string(plt->lfont);
		butt1=gtk_font_button_new_with_font(str);
		g_free(str);
		gtk_font_button_set_show_style(GTK_FONT_BUTTON(butt1), TRUE);
		gtk_font_button_set_show_size(GTK_FONT_BUTTON(butt1), TRUE);
		gtk_font_button_set_use_font(GTK_FONT_BUTTON(butt1), TRUE);
		gtk_font_button_set_use_size(GTK_FONT_BUTTON(butt1), FALSE);
		gtk_font_button_set_title(GTK_FONT_BUTTON(butt1), _("Font Selection for Axis Labels"));
		str=pango_font_description_to_string(plt->afont);
		butt2=gtk_font_button_new_with_font(str);
		g_free(str);
		gtk_font_button_set_show_style(GTK_FONT_BUTTON(butt2), TRUE);
		gtk_font_button_set_show_size(GTK_FONT_BUTTON(butt2), TRUE);
		gtk_font_button_set_use_font(GTK_FONT_BUTTON(butt2), TRUE);
		gtk_font_button_set_use_size(GTK_FONT_BUTTON(butt2), FALSE);
		gtk_font_button_set_title(GTK_FONT_BUTTON(butt2), _("Font Selection for Tick Mark Labels"));
		gtk_widget_show(entry1);
		gtk_widget_show(entry2);
		gtk_widget_show(butt1);
		gtk_widget_show(butt2);
		gtk_table_attach(GTK_TABLE(table), entry1, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_table_attach(GTK_TABLE(table), entry2, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_table_attach(GTK_TABLE(table), butt1, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_table_attach(GTK_TABLE(table), butt2, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), spr, FALSE, FALSE, 2);
		ck4=gtk_combo_box_new_text();
		gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Visibility"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Domain Shift"));
		if ((flags&16)!=0)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(ck4), _("Chirp"));
			gtk_combo_box_set_active(GTK_COMBO_BOX(ck4), (flagd&24)/8);
		}
		else gtk_combo_box_set_active(GTK_COMBO_BOX(ck4), (flagd&8)/8);
		gtk_widget_show(ck4);
		gtk_box_pack_start(GTK_BOX(vbox), ck4, FALSE, FALSE, 2);
		table=gtk_table_new(2, 2, FALSE);
		gtk_widget_show(table);
		ck2=gtk_check_button_new_with_label(_("Multiple plots for Results over index j"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck2), (flagd&2));
		gtk_widget_show(ck2);
		gtk_table_attach(GTK_TABLE(table), ck2, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		adj=(GtkAdjustment *) gtk_adjustment_new(jdim, 0, jdimx-1, 1.0, 5.0, 0.0);
		jind3=gtk_spin_button_new(adj, 0, 0);
		g_signal_connect(G_OBJECT(jind3), "value-changed", G_CALLBACK(upj), NULL);
		gtk_widget_show(jind3);
		gtk_table_attach(GTK_TABLE(table), jind3, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		ck3=gtk_check_button_new_with_label(_("Multiple plots for Results over index k"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck3), (flagd&4));
		gtk_widget_show(ck3);
		gtk_table_attach(GTK_TABLE(table), ck3, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		adj=(GtkAdjustment *) gtk_adjustment_new(kdim, 0, kdimx-1, 1.0, 5.0, 0.0);
		kind2=gtk_spin_button_new(adj, 0, 0);
		g_signal_connect(G_OBJECT(kind2), "value-changed", G_CALLBACK(upk), NULL);
		gtk_widget_show(kind2);
		gtk_table_attach(GTK_TABLE(table), kind2, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
		gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 2);
		spr=gtk_vseparator_new();
		gtk_widget_show(spr);
		gtk_box_pack_start(GTK_BOX(hbox), spr, FALSE, FALSE, 2);
		colour=gtk_color_selection_new();
		gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour), TRUE);
		gtk_widget_show(colour);
		gtk_box_pack_start(GTK_BOX(hbox), colour, FALSE, FALSE, 2);
		gtk_container_add(GTK_CONTAINER(content), hbox);
		if (gtk_dialog_run(GTK_DIALOG(helpwin))==GTK_RESPONSE_APPLY)
		{
			g_free(plt->xlab);
			g_free(plt->ylab);
			pango_font_description_free(plt->lfont);
			pango_font_description_free(plt->afont);
			(plt->xlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry1)));
			(plt->ylab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry2)));
			str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt1)));
			(plt->lfont)=pango_font_description_from_string(str);
			g_free(str);
			str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt2)));
			(plt->afont)=pango_font_description_from_string(str);
			g_free(str);
			g_object_get(G_OBJECT(plot3), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
			bl1=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2));
			bl2=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3));
			bl3=gtk_combo_box_get_active(GTK_COMBO_BOX(ck4));
			if ((bl1==((flagd&2)==0))||(bl2==((flagd&4)==0))||((bl3*8)!=(flagd&24)))
			{
				g_array_free(bxr, TRUE);
				g_array_free(byr, TRUE);
				g_array_free(bsz, TRUE);
				g_array_free(bnx, TRUE);
				dx2=0;
				if (bl3==0)
				{
					if (bl1)
					{
						if (bl2)
						{
							bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf*jdimxf);
							byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf*jdimxf);
							bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf*jdimxf);
							bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf*jdimxf);
							iv=g_array_index(msr, gdouble, 0);
							g_array_append_val(bxr, iv);
							mny=g_array_index(vis, gdouble, 0);
							g_array_append_val(byr, mny);
							mxy=mny;
							for (j=1; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(vis, gdouble, (j*jdimxf*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
							for (k=1; k<kdimxf; k++)
							{
								for (j=0; j<mx; j++)
								{
									iv=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, iv);
									iv=g_array_index(vis, gdouble, k+(j*jdimxf*kdimxf));
									g_array_append_val(byr, iv);
									if (iv>mxy) mxy=iv;
									else if (iv<mny) mny=iv;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
							}
							for (l=1; l<jdimxf; l++)
							{
								for (k=0; k<kdimxf; k++)
								{
									for (j=0; j<mx; j++)
									{
										iv=g_array_index(msr, gdouble, j);
										g_array_append_val(bxr, iv);
										iv=g_array_index(vis, gdouble, k+((l+(j*jdimxf))*kdimxf));
										g_array_append_val(byr, iv);
										if (iv>mxy) mxy=iv;
										else if (iv<mny) mny=iv;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									dx2+=mx;
								}
							}
							flagd&=1;
							flagd|=6;
						}
						else
						{
							bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimxf);
							byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimxf);
							bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
							bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
							iv=g_array_index(msr, gdouble, 0);
							g_array_append_val(bxr, iv);
							mny=g_array_index(vis, gdouble, kdim);
							g_array_append_val(byr, mny);
							mxy=mny;
							for (j=1; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(vis, gdouble, kdim+(j*jdimxf*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
							for (k=1; k<jdimxf; k++)
							{
								for (j=0; j<mx; j++)
								{
									iv=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, iv);
									iv=g_array_index(vis, gdouble, kdim+((k+(j*jdimxf))*kdimxf));
									g_array_append_val(byr, iv);
									if (iv>mxy) mxy=iv;
									else if (iv<mny) mny=iv;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
							}
							flagd&=1;
							flagd|=2;
						}
					}
					else if (bl2)
					{
						bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf);
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf);
						bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
						iv=g_array_index(msr, gdouble, 0);
						g_array_append_val(bxr, iv);
						mny=g_array_index(vis, gdouble, (jdim*kdimxf));
						g_array_append_val(byr, mny);
						mxy=mny;
						for (j=1; j<mx; j++)
						{
							iv=g_array_index(msr, gdouble, j);
							g_array_append_val(bxr, iv);
							iv=g_array_index(vis, gdouble, ((jdim+(j*jdimxf))*kdimxf));
							g_array_append_val(byr, iv);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						g_array_append_val(bsz, mx);
						g_array_append_val(bnx, dx2);
						dx2+=mx;
						for (k=1; k<kdimxf; k++)
						{
							for (j=0; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(vis, gdouble, k+((jdim+(j*jdimxf))*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
						}
						flagd&=1;
						flagd|=4;
					}
					else
					{
						bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
						bsz=g_array_new(FALSE, FALSE, sizeof(gint));
						bnx=g_array_new(FALSE, FALSE, sizeof(gint));
						iv=g_array_index(msr, gdouble, 0);
						g_array_append_val(bxr, iv);
						mny=g_array_index(vis, gdouble, kdim+(jdim*kdimxf));
						g_array_append_val(byr, mny);
						mxy=mny;
						for (j=1; j<mx; j++)
						{
							iv=g_array_index(msr, gdouble, j);
							g_array_append_val(bxr, iv);
							iv=g_array_index(vis, gdouble, kdim+((jdim+(j*jdimxf))*kdimxf));
							g_array_append_val(byr, iv);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						g_array_append_val(bsz, mx);
						g_array_append_val(bnx, dx2);
						flagd&=1;
					}
				}
				else if (bl3==1)
				{
					if (bl1)
					{
						if (bl2)
						{
							bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf*jdimxf);
							byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf*jdimxf);
							bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf*jdimxf);
							bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf*jdimxf);
							iv=g_array_index(msr, gdouble, 0);
							g_array_append_val(bxr, iv);
							mny=g_array_index(doms, gdouble, 0);
							g_array_append_val(byr, mny);
							mxy=mny;
							for (j=1; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(doms, gdouble, (j*jdimxf*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
							for (k=1; k<kdimxf; k++)
							{
								for (j=0; j<mx; j++)
								{
									iv=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, iv);
									iv=g_array_index(doms, gdouble, k+(j*jdimxf*kdimxf));
									g_array_append_val(byr, iv);
									if (iv>mxy) mxy=iv;
									else if (iv<mny) mny=iv;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
							}
							for (l=1; l<jdimxf; l++)
							{
								for (k=0; k<kdimxf; k++)
								{
									for (j=0; j<mx; j++)
									{
										iv=g_array_index(msr, gdouble, j);
										g_array_append_val(bxr, iv);
										iv=g_array_index(doms, gdouble, k+((l+(j*jdimxf))*kdimxf));
										g_array_append_val(byr, iv);
										if (iv>mxy) mxy=iv;
										else if (iv<mny) mny=iv;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									dx2+=mx;
								}
							}
							flagd&=1;
							flagd|=14;
						}
						else
						{
							bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimxf);
							byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimxf);
							bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
							bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
							iv=g_array_index(msr, gdouble, 0);
							g_array_append_val(bxr, iv);
							mny=g_array_index(doms, gdouble, kdim);
							g_array_append_val(byr, mny);
							mxy=mny;
							for (j=1; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(doms, gdouble, kdim+(j*jdimxf*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
							for (k=1; k<jdimxf; k++)
							{
								for (j=0; j<mx; j++)
								{
									iv=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, iv);
									iv=g_array_index(doms, gdouble, kdim+((k+(j*jdimxf))*kdimxf));
									g_array_append_val(byr, iv);
									if (iv>mxy) mxy=iv;
									else if (iv<mny) mny=iv;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
							}
							flagd&=1;
							flagd|=10;
						}
					}
					else if (bl2)
					{
						bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf);
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf);
						bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
						iv=g_array_index(msr, gdouble, 0);
						g_array_append_val(bxr, iv);
						mny=g_array_index(doms, gdouble, (jdim*kdimxf));
						g_array_append_val(byr, mny);
						mxy=mny;
						for (j=1; j<mx; j++)
						{
							iv=g_array_index(msr, gdouble, j);
							g_array_append_val(bxr, iv);
							iv=g_array_index(doms, gdouble, ((jdim+(j*jdimxf))*kdimxf));
							g_array_append_val(byr, iv);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						g_array_append_val(bsz, mx);
						g_array_append_val(bnx, dx2);
						dx2+=mx;
						for (k=1; k<kdimxf; k++)
						{
							for (j=0; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(doms, gdouble, k+((jdim+(j*jdimxf))*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
						}
						flagd&=1;
						flagd|=12;
					}
					else
					{
						bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
						bsz=g_array_new(FALSE, FALSE, sizeof(gint));
						bnx=g_array_new(FALSE, FALSE, sizeof(gint));
						iv=g_array_index(msr, gdouble, 0);
						g_array_append_val(bxr, iv);
						mny=g_array_index(doms, gdouble, kdim+(jdim*kdimxf));
						g_array_append_val(byr, mny);
						mxy=mny;
						for (j=1; j<mx; j++)
						{
							iv=g_array_index(msr, gdouble, j);
							g_array_append_val(bxr, iv);
							iv=g_array_index(doms, gdouble, kdim+((jdim+(j*jdimxf))*kdimxf));
							g_array_append_val(byr, iv);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						g_array_append_val(bsz, mx);
						g_array_append_val(bnx, dx2);
						flagd&=1;
						flagd|=8;
					}
				}
				else if (bl1)
				{
					if (bl2)
					{
						bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf*jdimxf);
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf*jdimxf);
						bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf*jdimxf);
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf*jdimxf);
						iv=g_array_index(msr, gdouble, 0);
						g_array_append_val(bxr, iv);
						mny=g_array_index(chp, gdouble, 0);
						g_array_append_val(byr, mny);
						mxy=mny;
						for (j=1; j<mx; j++)
						{
							iv=g_array_index(msr, gdouble, j);
							g_array_append_val(bxr, iv);
							iv=g_array_index(chp, gdouble, (j*jdimxf*kdimxf));
							g_array_append_val(byr, iv);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						g_array_append_val(bsz, mx);
						g_array_append_val(bnx, dx2);
						dx2+=mx;
						for (k=1; k<kdimxf; k++)
						{
							for (j=0; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(chp, gdouble, k+(j*jdimxf*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
						}
						for (l=1; l<jdimxf; l++)
						{
							for (k=0; k<kdimxf; k++)
							{
								for (j=0; j<mx; j++)
								{
									iv=g_array_index(msr, gdouble, j);
									g_array_append_val(bxr, iv);
									iv=g_array_index(chp, gdouble, k+((l+(j*jdimxf))*kdimxf));
									g_array_append_val(byr, iv);
									if (iv>mxy) mxy=iv;
									else if (iv<mny) mny=iv;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								dx2+=mx;
							}
						}
						flagd&=1;
						flagd|=22;
					}
					else
					{
						bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimxf);
						byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimxf);
						bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
						iv=g_array_index(msr, gdouble, 0);
						g_array_append_val(bxr, iv);
						mny=g_array_index(chp, gdouble, kdim);
						g_array_append_val(byr, mny);
						mxy=mny;
						for (j=1; j<mx; j++)
						{
							iv=g_array_index(msr, gdouble, j);
							g_array_append_val(bxr, iv);
							iv=g_array_index(chp, gdouble, kdim+(j*jdimxf*kdimxf));
							g_array_append_val(byr, iv);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						g_array_append_val(bsz, mx);
						g_array_append_val(bnx, dx2);
						dx2+=mx;
						for (k=1; k<jdimxf; k++)
						{
							for (j=0; j<mx; j++)
							{
								iv=g_array_index(msr, gdouble, j);
								g_array_append_val(bxr, iv);
								iv=g_array_index(chp, gdouble, kdim+((k+(j*jdimxf))*kdimxf));
								g_array_append_val(byr, iv);
								if (iv>mxy) mxy=iv;
								else if (iv<mny) mny=iv;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							dx2+=mx;
						}
						flagd&=1;
						flagd|=18;
					}
				}
				else if (bl2)
				{
					bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf);
					byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*kdimxf);
					bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
					bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);
					iv=g_array_index(msr, gdouble, 0);
					g_array_append_val(bxr, iv);
					mny=g_array_index(chp, gdouble, (jdim*kdimxf));
					g_array_append_val(byr, mny);
					mxy=mny;
					for (j=1; j<mx; j++)
					{
						iv=g_array_index(msr, gdouble, j);
						g_array_append_val(bxr, iv);
						iv=g_array_index(chp, gdouble, ((jdim+(j*jdimxf))*kdimxf));
						g_array_append_val(byr, iv);
						if (iv>mxy) mxy=iv;
						else if (iv<mny) mny=iv;
					}
					g_array_append_val(bsz, mx);
					g_array_append_val(bnx, dx2);
					dx2+=mx;
					for (k=1; k<kdimxf; k++)
					{
						for (j=0; j<mx; j++)
						{
							iv=g_array_index(msr, gdouble, j);
							g_array_append_val(bxr, iv);
							iv=g_array_index(chp, gdouble, k+((jdim+(j*jdimxf))*kdimxf));
							g_array_append_val(byr, iv);
							if (iv>mxy) mxy=iv;
							else if (iv<mny) mny=iv;
						}
						g_array_append_val(bsz, mx);
						g_array_append_val(bnx, dx2);
						dx2+=mx;
					}
					flagd&=1;
					flagd|=20;
				}
				else
				{
					bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
					byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
					bsz=g_array_new(FALSE, FALSE, sizeof(gint));
					bnx=g_array_new(FALSE, FALSE, sizeof(gint));
					iv=g_array_index(msr, gdouble, 0);
					g_array_append_val(bxr, iv);
					mny=g_array_index(chp, gdouble, kdim+(jdim*kdimxf));
					g_array_append_val(byr, mny);
					mxy=mny;
					for (j=1; j<mx; j++)
					{
						iv=g_array_index(msr, gdouble, j);
						g_array_append_val(bxr, iv);
						iv=g_array_index(chp, gdouble, kdim+((jdim+(j*jdimxf))*kdimxf));
						g_array_append_val(byr, iv);
						if (iv>mxy) mxy=iv;
						else if (iv<mny) mny=iv;
					}
					g_array_append_val(bsz, mx);
					g_array_append_val(bnx, dx2);
					flagd&=1;
					flagd|=16;
				}
				(plt->xdata)=bxr;
				(plt->ydata)=byr;
				(plt->sizes)=bsz;
				(plt->ind)=bnx;
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
				gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 2);
				gtk_container_add(GTK_CONTAINER(content), hbox);
				if (gtk_dialog_run(GTK_DIALOG(helpwin))==GTK_RESPONSE_APPLY)
				{
					flagd&=1;
					j=gtk_combo_box_get_active(GTK_COMBO_BOX(ck4));
					flagd|=j*8;
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2))) flagd|=2;
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3))) flagd|=4;
				}
			}
			break;
			case 1:
			table=gtk_table_new(4, 2, FALSE);
			gtk_widget_show(table);
			label=gtk_label_new(_("Axis label font:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			label=gtk_label_new(_("Tick label font:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			entry1=gtk_entry_new();
			entry2=gtk_entry_new();
			spr=gtk_hseparator_new();
			gtk_widget_show(spr);
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
			str=pango_font_description_to_string(plt->lfont);
			butt1=gtk_font_button_new_with_font(str);
			g_free(str);
			gtk_font_button_set_show_style(GTK_FONT_BUTTON(butt1), TRUE);
			gtk_font_button_set_show_size(GTK_FONT_BUTTON(butt1), TRUE);
			gtk_font_button_set_use_font(GTK_FONT_BUTTON(butt1), TRUE);
			gtk_font_button_set_use_size(GTK_FONT_BUTTON(butt1), FALSE);
			gtk_font_button_set_title(GTK_FONT_BUTTON(butt1), _("Font Selection for Axis Labels"));
			str=pango_font_description_to_string(plt->afont);
			butt2=gtk_font_button_new_with_font(str);
			g_free(str);
			gtk_font_button_set_show_style(GTK_FONT_BUTTON(butt2), TRUE);
			gtk_font_button_set_show_size(GTK_FONT_BUTTON(butt2), TRUE);
			gtk_font_button_set_use_font(GTK_FONT_BUTTON(butt2), TRUE);
			gtk_font_button_set_use_size(GTK_FONT_BUTTON(butt2), FALSE);
			gtk_font_button_set_title(GTK_FONT_BUTTON(butt2), _("Font Selection for Tick Mark Labels"));
			gtk_widget_show(entry1);
			gtk_widget_show(entry2);
			gtk_widget_show(butt1);
			gtk_widget_show(butt2);
			gtk_table_attach(GTK_TABLE(table), entry1, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_table_attach(GTK_TABLE(table), entry2, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_table_attach(GTK_TABLE(table), butt1, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_table_attach(GTK_TABLE(table), butt2, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(vbox), spr, FALSE, FALSE, 2);
			hbox2=gtk_hbox_new(FALSE, 0);
			gtk_widget_show(hbox2);
			ck=gtk_check_button_new_with_label(_("Multiple plots for Inverse Domain"));
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck), (flagd&1));
			gtk_widget_show(ck);
			gtk_box_pack_start(GTK_BOX(hbox2), ck, FALSE, FALSE, 2);
			adj=(GtkAdjustment *) gtk_adjustment_new(jdim, 0, jdimx-1, 1.0, 5.0, 0.0);
			jind3=gtk_spin_button_new(adj, 0, 0);
			g_signal_connect(G_OBJECT(jind3), "value-changed", G_CALLBACK(upj), NULL);
			gtk_widget_show(jind3);
			gtk_box_pack_start(GTK_BOX(hbox2), jind3, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 2);
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bat)))
			{
				ck2=gtk_check_button_new_with_label(_("Multiple plots for Results over index j"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck2), (flagd&2));
				gtk_widget_show(ck2);
				gtk_box_pack_start(GTK_BOX(vbox), ck2, FALSE, FALSE, 2);
				ck3=gtk_check_button_new_with_label(_("Multiple plots for Results over index k"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck3), (flagd&4));
				gtk_widget_show(ck3);
				gtk_box_pack_start(GTK_BOX(vbox), ck3, FALSE, FALSE, 2);
				gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 2);
				spr=gtk_vseparator_new();
				gtk_widget_show(spr);
				gtk_box_pack_start(GTK_BOX(hbox), spr, FALSE, FALSE, 2);
				colour=gtk_color_selection_new();
				gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour), TRUE);
				gtk_widget_show(colour);
				gtk_box_pack_start(GTK_BOX(hbox), colour, FALSE, FALSE, 2);
				gtk_container_add(GTK_CONTAINER(content), hbox);
				if (gtk_dialog_run(GTK_DIALOG(helpwin))==GTK_RESPONSE_APPLY)
				{
					g_free(plt->xlab);
					g_free(plt->ylab);
					pango_font_description_free(plt->lfont);
					pango_font_description_free(plt->afont);
					(plt->xlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry1)));
					(plt->ylab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry2)));
					str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt1)));
					(plt->lfont)=pango_font_description_from_string(str);
					g_free(str);
					str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt2)));
					(plt->afont)=pango_font_description_from_string(str);
					g_free(str);
					g_object_get(G_OBJECT(plot2), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck)))
					{
						if ((flagd&1)==0)
						{
							flagd^=1;
							if ((flags&2)!=0)/* merge into the one array */
							{
								{xi=0; mny=0;}
								dx2=0;
								dx=g_array_index(sz, gint, 0);
								{g_array_free(nx, TRUE); g_array_free(sz, TRUE);}
								nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
								sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
								xf=g_array_index(xsb, gdouble, dx-1);
								mxy=g_array_index(ysb, gdouble, 0);
								for (j=1; j<dx; j++)
								{
									iv=g_array_index(ysb, gdouble, j);
									if (mxy<iv) mxy=iv;
								}
								g_array_append_val(sz, dx);
								g_array_append_val(nx, dx2);
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
									g_array_append_val(sz, dx);
									dx2+=dx;
									g_array_append_val(nx, dx2);
									k++;
								}
								(plt->sizes)=sz;
								(plt->ind)=nx;
							}
						}
					}
					else if ((flagd&1)!=0)
					{
						flagd^=1;
						if ((flags&2)!=0)/* narrow array to the one plot */
						{
							{xi=0; mny=0;}
							xf=g_array_index(xsb, gdouble, dx-1);
							dx=g_array_index(sz, gint, 0);
							g_array_free(sz, TRUE);
							sz=g_array_new(FALSE, FALSE, sizeof(gint));
							g_array_append_val(sz, dx);
							(plt->sizes)=sz;
							mxy=g_array_index(ysb, gdouble, jdim*dx);
							for (j=1+(jdim*dx); j<((jdim+1)*dx); j++)
							{
								iv=g_array_index(ysb, gdouble, j);
								if (mxy<iv) mxy=iv;
							}
							g_array_free(nx, TRUE);
							nx=g_array_new(FALSE, FALSE, sizeof(gint));
							dx=0;
							g_array_append_val(nx, dx);
							(plt->ind)=nx;
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
				gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 2);
				spr=gtk_vseparator_new();
				gtk_widget_show(spr);
				gtk_box_pack_start(GTK_BOX(hbox), spr, FALSE, FALSE, 2);
				colour=gtk_color_selection_new();
				gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour), TRUE);
				gtk_widget_show(colour);
				gtk_box_pack_start(GTK_BOX(hbox), colour, FALSE, FALSE, 2);
				gtk_container_add(GTK_CONTAINER(content), hbox);
				if (gtk_dialog_run(GTK_DIALOG(helpwin))==GTK_RESPONSE_APPLY)
				{
					g_free(plt->xlab);
					g_free(plt->ylab);
					pango_font_description_free(plt->lfont);
					pango_font_description_free(plt->afont);
					(plt->xlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry1)));
					(plt->ylab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry2)));
					str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt1)));
					(plt->lfont)=pango_font_description_from_string(str);
					g_free(str);
					str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt2)));
					(plt->afont)=pango_font_description_from_string(str);
					g_free(str);
					g_object_get(G_OBJECT(plot2), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck)))
					{
						if ((flagd&1)==0)
						{
							flagd^=1;
							if ((flags&2)!=0)/* merge into the one array */
							{
								{xi=0; mny=0;}
								dx2=0;
								dx=g_array_index(sz, gint, 0);
								{g_array_free(nx, TRUE); g_array_free(sz, TRUE);}
								nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
								sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
								xf=g_array_index(xsb, gdouble, dx-1);
								mxy=g_array_index(ysb, gdouble, 0);
								for (j=1; j<dx; j++)
								{
									iv=g_array_index(ysb, gdouble, j);
									if (mxy<iv) mxy=iv;
								}
								g_array_append_val(sz, dx);
								g_array_append_val(nx, dx2);
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
									g_array_append_val(sz, dx);
									dx2+=dx;
									g_array_append_val(nx, dx2);
									k++;
								}
								(plt->sizes)=sz;
								(plt->ind)=nx;
							}
						}
					}
					else if ((flagd&1)!=0)
					{
						flagd^=1;
						if ((flags&2)!=0)/* narrow array to the one plot */
						{
							{xi=0; mny=0;}
							xf=g_array_index(xsb, gdouble, dx-1);
							dx=g_array_index(sz, gint, 0);
							g_array_free(sz, TRUE);
							sz=g_array_new(FALSE, FALSE, sizeof(gint));
							g_array_append_val(sz, dx);
							(plt->sizes)=sz;
							mxy=g_array_index(ysb, gdouble, jdim*dx);
							for (j=1+(jdim*dx); j<((jdim+1)*dx); j++)
							{
								iv=g_array_index(ysb, gdouble, j);
								if (mxy<iv) mxy=iv;
							}
							g_array_free(nx, TRUE);
							nx=g_array_new(FALSE, FALSE, sizeof(gint));
							dx=0;
							g_array_append_val(nx, dx);
							(plt->ind)=nx;
						}
					}
					plot_linear_update_scale(plot2, xi, xf, mny, mxy);
				}
			}
			break;
			default:
			table=gtk_table_new(4, 2, FALSE);
			gtk_widget_show(table);
			label=gtk_label_new(_("Axis label font:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			label=gtk_label_new(_("Tick label font:"));
			gtk_widget_show(label);
			gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			entry1=gtk_entry_new();
			entry2=gtk_entry_new();
			spr=gtk_hseparator_new();
			gtk_widget_show(spr);
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
			str=pango_font_description_to_string(plt->lfont);
			butt1=gtk_font_button_new_with_font(str);
			g_free(str);
			gtk_font_button_set_show_style(GTK_FONT_BUTTON(butt1), TRUE);
			gtk_font_button_set_show_size(GTK_FONT_BUTTON(butt1), TRUE);
			gtk_font_button_set_use_font(GTK_FONT_BUTTON(butt1), TRUE);
			gtk_font_button_set_use_size(GTK_FONT_BUTTON(butt1), FALSE);
			gtk_font_button_set_title(GTK_FONT_BUTTON(butt1), _("Font Selection for Axis Labels"));
			str=pango_font_description_to_string(plt->afont);
			butt2=gtk_font_button_new_with_font(str);
			g_free(str);
			gtk_font_button_set_show_style(GTK_FONT_BUTTON(butt2), TRUE);
			gtk_font_button_set_show_size(GTK_FONT_BUTTON(butt2), TRUE);
			gtk_font_button_set_use_font(GTK_FONT_BUTTON(butt2), TRUE);
			gtk_font_button_set_use_size(GTK_FONT_BUTTON(butt2), FALSE);
			gtk_font_button_set_title(GTK_FONT_BUTTON(butt2), _("Font Selection for Tick Mark Labels"));
			gtk_widget_show(entry1);
			gtk_widget_show(entry2);
			gtk_widget_show(butt1);
			gtk_widget_show(butt2);
			gtk_table_attach(GTK_TABLE(table), entry1, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_table_attach(GTK_TABLE(table), entry2, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_table_attach(GTK_TABLE(table), butt1, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_table_attach(GTK_TABLE(table), butt2, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(vbox), spr, FALSE, FALSE, 2);
			ck=gtk_check_button_new_with_label(_("Multiple plots for Inverse Domain"));
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck), (flagd&1));
			gtk_widget_show(ck);
			gtk_box_pack_start(GTK_BOX(vbox), ck, FALSE, FALSE, 2);
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bat)))
			{
				ck2=gtk_check_button_new_with_label(_("Multiple plots for Results over index j"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck2), (flagd&2));
				gtk_widget_show(ck2);
				gtk_box_pack_start(GTK_BOX(vbox), ck2, FALSE, FALSE, 2);
				ck3=gtk_check_button_new_with_label(_("Multiple plots for Results over index k"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ck3), (flagd&4));
				gtk_widget_show(ck3);
				gtk_box_pack_start(GTK_BOX(vbox), ck3, FALSE, FALSE, 2);
				gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 2);
				spr=gtk_vseparator_new();
				gtk_widget_show(spr);
				gtk_box_pack_start(GTK_BOX(hbox), spr, FALSE, FALSE, 2);
				colour=gtk_color_selection_new();
				gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour), TRUE);
				gtk_widget_show(colour);
				gtk_box_pack_start(GTK_BOX(hbox), colour, FALSE, FALSE, 2);
				gtk_container_add(GTK_CONTAINER(content), hbox);
				if (gtk_dialog_run(GTK_DIALOG(helpwin))==GTK_RESPONSE_APPLY)
				{
					g_free(plt->xlab);
					g_free(plt->ylab);
					pango_font_description_free(plt->lfont);
					pango_font_description_free(plt->afont);
					(plt->xlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry1)));
					(plt->ylab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry2)));
					str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt1)));
					(plt->lfont)=pango_font_description_from_string(str);
					g_free(str);
					str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt2)));
					(plt->afont)=pango_font_description_from_string(str);
					g_free(str);
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck)))
					{
						if ((flagd&1)==0)
						{
							flagd^=1;
							if ((flags&2)!=0)/* merge into the one array */
							{
								plt=PLOT_LINEAR(plot2);
								{xi=0; mny=0;}
								dx2=0;
								dx=g_array_index(sz, gint, 0);
								{g_array_free(nx, TRUE); g_array_free(sz, TRUE);}
								nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
								sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
								xf=g_array_index(xsb, gdouble, dx-1);
								mxy=g_array_index(ysb, gdouble, 0);
								for (j=1; j<dx; j++)
								{
									iv=g_array_index(ysb, gdouble, j);
									if (mxy<iv) mxy=iv;
								}
								g_array_append_val(sz, dx);
								g_array_append_val(nx, dx2);
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
									g_array_append_val(sz, dx);
									dx2+=dx;
									g_array_append_val(nx, dx2);
									k++;
								}
								(plt->sizes)=sz;
								(plt->ind)=nx;
							}
						}
					}
					else if ((flagd&1)!=0)/* narrow array to one plot */
					{
						flagd^=1;
						if ((flags&2)!=0)
						{
							plt=PLOT_LINEAR(plot2);
							{xi=0; mny=0;}
							xf=g_array_index(xsb, gdouble, dx-1);
							dx=g_array_index(sz, gint, 0);
							g_array_free(sz, TRUE);
							sz=g_array_new(FALSE, FALSE, sizeof(gint));
							g_array_append_val(sz, dx);
							(plt->sizes)=sz;
							mxy=g_array_index(ysb, gdouble, jdim*dx);
							for (j=1+(jdim*dx); j<((jdim+1)*dx); j++)
							{
								iv=g_array_index(ysb, gdouble, j);
								if (mxy<iv) mxy=iv;
							}
							g_array_free(nx, TRUE);
							nx=g_array_new(FALSE, FALSE, sizeof(gint));
							dx=0;
							g_array_append_val(nx, dx);
							(plt->ind)=nx;
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
				gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 2);
				spr=gtk_vseparator_new();
				gtk_widget_show(spr);
				gtk_box_pack_start(GTK_BOX(hbox), spr, FALSE, FALSE, 2);
				colour=gtk_color_selection_new();
				gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(colour), TRUE);
				gtk_widget_show(colour);
				gtk_box_pack_start(GTK_BOX(hbox), colour, FALSE, FALSE, 2);
				gtk_container_add(GTK_CONTAINER(content), hbox);
				if (gtk_dialog_run(GTK_DIALOG(helpwin))==GTK_RESPONSE_APPLY)
				{
					g_free(plt->xlab);
					g_free(plt->ylab);
					pango_font_description_free(plt->lfont);
					pango_font_description_free(plt->afont);
					(plt->xlab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry1)));
					(plt->ylab)=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry2)));
					str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt1)));
					(plt->lfont)=pango_font_description_from_string(str);
					g_free(str);
					str=g_strdup(gtk_font_button_get_font_name(GTK_FONT_BUTTON(butt2)));
					(plt->afont)=pango_font_description_from_string(str);
					g_free(str);
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck)))
					{
						if ((flagd&1)==0)
						{
							flagd^=1;
							if ((flags&2)!=0)/* merge into the one array */
							{
								plt=PLOT_LINEAR(plot2);
								{xi=0; mny=0;}
								dx2=0;
								dx=g_array_index(sz, gint, 0);
								{g_array_free(nx, TRUE); g_array_free(sz, TRUE);}
								nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
								sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimxf);
								xf=g_array_index(xsb, gdouble, dx-1);
								mxy=g_array_index(ysb, gdouble, 0);
								for (j=1; j<dx; j++)
								{
									iv=g_array_index(ysb, gdouble, j);
									if (mxy<iv) mxy=iv;
								}
								g_array_append_val(sz, dx);
								g_array_append_val(nx, dx2);
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
									g_array_append_val(sz, dx);
									dx2+=dx;
									g_array_append_val(nx, dx2);
									k++;
								}
								(plt->sizes)=sz;
								(plt->ind)=nx;
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
							{xi=0; mny=0;}
							xf=g_array_index(xsb, gdouble, dx-1);
							dx=g_array_index(sz, gint, 0);
							g_array_free(sz, TRUE);
							sz=g_array_new(FALSE, FALSE, sizeof(gint));
							g_array_append_val(sz, dx);
							(plt->sizes)=sz;
							mxy=g_array_index(ysb, gdouble, jdim*dx);
							for (j=1+(jdim*dx); j<((jdim+1)*dx); j++)
							{
								iv=g_array_index(ysb, gdouble, j);
								if (mxy<iv) mxy=iv;
							}
							g_array_free(nx, TRUE);
							nx=g_array_new(FALSE, FALSE, sizeof(gint));
							dx=0;
							g_array_append_val(nx, dx);
							(plt->ind)=nx;
							plot_linear_update_scale_pretty(plot2, 0, xf, 0, mxy);
						}
					}
					g_object_get(G_OBJECT(plot1), "xmin", &xi, "xmax", &xf, "ymin", &mny, "ymax", &mxy, NULL);
					plot_linear_update_scale(plot1, xi, xf, mny, mxy);
				}
			}
			break;
		}
	}
	gtk_widget_destroy(helpwin);
}

