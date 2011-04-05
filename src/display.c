/***************************************************************************
 *            display.c
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

#include "display.h"

void dpr(GtkWidget *widget, gpointer data)
{
	GtkWidget *helpwin, *content, *table, *vbox, *entry1, *entry2, *label, *spin1, *spin2, *hsp, *ck, *ck2, *ck3, *ck4;
	GtkAdjustment *adj;
	PlotLinear *plt;
	PlotPolar *plt2;
	gdouble xi, xf, mny, mxy, iv, clc, rcn, thc;
	gint dx, dx2, j, k, l, bl1, bl2, bl3;
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
			bl1=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck2));
			bl2=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ck3));
			bl3=gtk_combo_box_get_active(GTK_COMBO_BOX(ck4));
			if ((bl1==((flagd&2)==0))||(bl2==((flagd&4)==0))||((bl3*8)!=(flagd&24)))
			{
				g_array_free(bxr, TRUE);
				g_array_free(byr, TRUE);
				g_array_free(bsz, TRUE);
				g_array_free(bnx, TRUE);
				if (bl3==0)
				{
					if (bl1)
					{
						if (bl2)
						{
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
			break;
		}
	}
	gtk_widget_destroy(helpwin);
}

