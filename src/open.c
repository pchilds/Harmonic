/***************************************************************************
 *            open.c
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

#include "open.h"

GtkWidget *dialog, *spin;
GArray *fls, *vls;

void upt(GtkWidget *widget, gpointer dta)
{
	GtkWidget *wwfile, *lbl;
	GtkAdjustment *adj;
	GList *chld;
	guint rw;
	gdouble val;
	gchar *fin=NULL;

	wwfile=gtk_file_chooser_dialog_new(_("Select Data File"), GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	g_signal_connect(G_OBJECT(wwfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wwfile));
	if (gtk_dialog_run(GTK_DIALOG(wwfile))==GTK_RESPONSE_ACCEPT)
	{
		g_object_get(G_OBJECT(dta), "n-rows", &rw, NULL);
		gtk_table_resize(GTK_TABLE(dta), ++rw, 2);
		g_object_ref(G_OBJECT(widget));
		gtk_container_remove(GTK_CONTAINER(dta), widget);
		fin=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wwfile));
		lbl=gtk_label_new(fin);
		g_array_append_val(fls, fin);
		g_free(fin);
		gtk_widget_show(lbl);
		gtk_table_attach(GTK_TABLE(dta), lbl, 0, 1, rw-2, rw-1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		gtk_table_attach(GTK_TABLE(dta), widget, 0, 1, rw-1, rw, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		g_object_unref(G_OBJECT(widget));
		val=gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin));
		g_array_append_val(vls, val);
		adj=(GtkAdjustment*) gtk_adjustment_new(val, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
		spin=gtk_spin_button_new(adj, 0, 0);
		gtk_widget_show(spin);
		gtk_table_attach(GTK_TABLE(dta), spin, 1, 2, rw-1, rw, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	}
	gtk_widget_destroy(wwfile);
}

void opd(GtkWidget *widget, gpointer data)
{
	PlotLinear *plt;
	PlotPolar *plt2;
	GtkWidget *wfile, *content, *hbox, *vsc, *table, *scroll, *butt, *label, *cont, *trace;
	GtkAdjustment *adj, *adj2;
	gdouble xi, xf, lcl, mny, mxy, idelf, iv, vzt, vt, ivd, ivdt, tcn, twd, phi, phio, phia, dst, ddp, pn, cn, tp, ct, ofs, ofe, clc, dx2, xx, ce;
	guint j, k, l, m, sal, st, sp, kib;
	gint n, zp, lcib, dr;
	gchar s[5];
	gchar *contents, *contents2, *str, *s2, *fin=NULL;
	gchar **strary, **strary2, **strat, **strat2;
	gchar s1[10];
	GSList *list;
	GList *chld;
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
				fin=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				if (fin)/* also overwrite confirmation? */
				{
					dialog=gtk_dialog_new_with_buttons(_("Configuration file generation"), GTK_WINDOW(wfile), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_APPLY, NULL);
					g_signal_connect_swapped(G_OBJECT(dialog), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(dialog));
					gtk_widget_show(dialog);
					fls=g_array_new(FALSE, FALSE, sizeof(gchar*));
					vls=g_array_new(FALSE, FALSE, sizeof(gdouble));
					content=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
					table=gtk_table_new(2, 2, FALSE);
					gtk_widget_show(table);
					label=gtk_label_new(_("File:"));
					gtk_widget_show(label);
					gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
					label=gtk_label_new(_("Measurand\nValue:"));
					gtk_widget_show(label);
					gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
					butt=gtk_button_new_with_label(_("Add another file"));
					gtk_widget_show(butt);
					gtk_table_attach(GTK_TABLE(table), butt, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
					g_signal_connect(G_OBJECT(butt), "clicked", G_CALLBACK(upt), (gpointer) table);
					adj=(GtkAdjustment*) gtk_adjustment_new(0, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
					spin=gtk_spin_button_new(adj, 0, 0);
					gtk_widget_show(spin);
					gtk_table_attach(GTK_TABLE(table), spin, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
					adj=(GtkAdjustment*) gtk_adjustment_new(0, 0, 0, 0.0, 0.0, 0.0);
					adj2=(GtkAdjustment*) gtk_adjustment_new(0, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
					scroll=gtk_scrolled_window_new(adj, adj2);
					gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
					gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll), table);
					gtk_widget_show(scroll);
					gtk_box_pack_start(GTK_BOX(content), scroll, TRUE, TRUE, 2);
					if (gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_APPLY)
					{
						mx=(fls->len);
						g_snprintf(s1, 10, "%f", g_array_index(vls, gdouble, 0));
						s2=g_strdup(g_array_index(fls, gchar*, 0));
						contents2=g_strjoin("\t", s1, s2, NULL);
						g_free(s2);
						j=1;
						while (j<mx)
						{
							g_snprintf(s1, 10, "%f", g_array_index(vls, gdouble, j));
							s2=g_strdup(g_array_index(fls, gchar*, j));
							str=g_strjoin("\t", s1, s2, NULL);
							g_free(s2);
							contents=g_strdup(contents2);
							g_free(contents2);
							contents2=g_strjoin(DLMT, contents, str, NULL);
							g_free(contents);
							g_free(str);
							j++;
						}
						g_file_set_contents(fin, contents2, -1, &Err);
						g_free(contents2);
						if (Err)
						{
							str=g_strdup_printf(_("Error Saving file: %s."), (gchar*) Err);
							gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
							g_free(str);
							g_error_free(Err);
						}
						dr=GTK_RESPONSE_ACCEPT;
					}
					g_array_free(fls, TRUE);
					g_array_free(vls, TRUE);
				}
				g_free(fin);
				gtk_widget_destroy(dialog);
			}
			if (dr!=GTK_RESPONSE_ACCEPT) gtk_widget_destroy(wfile);
			else
			{
				fin=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				gtk_widget_destroy(wfile);
				if (g_file_get_contents(fin, &contents2, NULL, &Err))
				{
					{g_array_free(vis, TRUE); g_array_free(doms, TRUE); g_array_free(msr, TRUE); g_array_free(bxr, TRUE); g_array_free(byr, TRUE); g_array_free(bnx, TRUE); g_array_free(bsz, TRUE);}
					strary2=g_strsplit_set(contents2, DLMT, 0);
					mx=g_strv_length(strary2);
					zp=1<<(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(zpd)));
					n=zp*jdimx;
					y=fftw_malloc(sizeof(double)*n);
					star=fftw_malloc(sizeof(double)*n);
					p=fftw_plan_many_r2r(1, &zp, jdimx, y, NULL, 1, zp, star, NULL, 1, zp, &type, FFTW_ESTIMATE);
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(anosa))) {kib=2; lcib=-1;}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(sws))) {kib=0; lcib=-1;}
					else {kib=0; lcib=0;}
					vis=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx*kdimx*mx);
					doms=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx*kdimx*mx);
					msr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx);
					strat2=g_strsplit_set(strary2[0], "\t,", 0);
					xi=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
					xf=xi;
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(wll)))
					{
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
											if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssw- */
											{
												for (m=0; m<mx; m++)
												{
													strat2=g_strsplit_set(strary2[m], "\t,", 0);
													if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
													{
														{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
														lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
														g_array_append_val(msr, lcl);
														if (lcl<xi) xi=lcl;
														else if (lcl>xf) xf=lcl;
														strary=g_strsplit_set(contents, "\r\n", 0);
														sal=g_strv_length(strary);
														{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
														delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
														for (j=0; j<n; j++) y[j]=0;
														for (j=0; j<jdimx; j++)
														{
														}
														fftw_execute(p);
														stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
														for (j=0; j<n; j++)
														{
															iv=star[j];
															g_array_append_val(stars, iv);
														}
														for (j=0; j<jdimx; j++)
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
															for (k=0; k<kdimx; k++)
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
														str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
														gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
														g_free(str);
														g_error_free(Err);
													}
													g_free(contents);
													g_strfreev(strat2);
												}
											}
											else /* +TdBssw- */
											{
												for (m=0; m<mx; m++)
												{
													strat2=g_strsplit_set(strary2[m], "\t,", 0);
													if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
													{
														{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
														lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
														g_array_append_val(msr, lcl);
														if (lcl<xi) xi=lcl;
														else if (lcl>xf) xf=lcl;
														strary=g_strsplit_set(contents, "\r\n", 0);
														sal=g_strv_length(strary);
														{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
														delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
														for (j=0; j<n; j++) y[j]=0;
														for (j=0; j<jdimx; j++)
														{
														}
														fftw_execute(p);
														stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
														for (j=0; j<n; j++)
														{
															iv=star[j];
															g_array_append_val(stars, iv);
														}
														for (j=0; j<jdimx; j++)
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
															for (k=0; k<kdimx; k++)
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
														str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
														gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
														g_free(str);
														g_error_free(Err);
													}
													g_free(contents);
													g_strfreev(strat2);
												}
											}
										}
										else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssw- */
										{
											ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													for (j=0; j<jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
										else /* +RdBssw- */
										{
											ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													for (j=0; j<jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
									}
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssw- +Tlssw- */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* -Rlssw- +Rlssw- */
									{
										ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
										if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0w- */
										{
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													for (j=0; j<jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
										else /* +TdB0w- */
										{
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													for (j=0; j<jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
									}
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0w- */
									{
										ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +RdB0w- */
									{
										ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0w- +Tl0w- */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* -Rl0w- +Rl0w- */
								{
									ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
										if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssw+ */
										{
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													for (j=0; j<jdimx; j++)
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
															sp=zp;
															{ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
															while (k<8)
															{
																ofs+=g_array_index(yb, gdouble, st+k);
																k++;
																ofe+=g_array_index(yb, gdouble, sp+st-k);
															}
															{ofs/=8; ofe/=8;}
														}
														else if (sp<16) {ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1);}
														else
														{
															{ofs=g_array_index(yb, gdouble, st*satl); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
															while (k<8)
															{
																ofs+=g_array_index(yb, gdouble, st+k);
																k++;
																ofe+=g_array_index(yb, gdouble, sp+st-k);
															}
															{ofs/=8; ofe/=8;}
														}
														iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
														g_array_append_val(delf, iv);
														for (k=0; k<sp; k++)
														{
															clc=ofs+(k*(ofe-ofs)/(sp-1))-g_array_index(yb, gdouble, k+st);
															if (clc<NZE)
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
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
										else /* +TdBssw+ */
										{
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													for (j=0; j<jdimx; j++)
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
															sp=zp;
															{ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
															while (k<8)
															{
																ofs+=g_array_index(yb, gdouble, st+k);
																k++;
																ofe+=g_array_index(yb, gdouble, sp+st-k);
															}
															{ofs/=8; ofe/=8;}
														}
														else if (sp<16) {ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1);}
														else
														{
															{ofs=g_array_index(yb, gdouble, st*satl); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
															while (k<8)
															{
																ofs+=g_array_index(yb, gdouble, st+k);
																k++;
																ofe+=g_array_index(yb, gdouble, sp+st-k);
															}
															{ofs/=8; ofe/=8;}
														}
														iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
														g_array_append_val(delf, iv);
														for (k=0; k<sp; k++)
														{
															clc=g_array_index(yb, gdouble, k+st)-ofs-(k*(ofe-ofs)/(sp-1));
															if (clc<NZE)
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
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
									}
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssw+ */
									{
										ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
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
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +RdBssw+ */
									{
										ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
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
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssw+ +Tlssw+ */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
													sp=zp;
													{ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
													while (k<8)
													{
														ofs+=g_array_index(yb, gdouble, st+k);
														k++;
														ofe+=g_array_index(yb, gdouble, sp+st-k);
													}
													{ofs/=8; ofe/=8;}
												}
												else if (sp<16) {ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1);}
												else
												{
													{ofs=g_array_index(yb, gdouble, st*satl); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
													while (k<8)
													{
														ofs+=g_array_index(yb, gdouble, st+k);
														k++;
														ofe+=g_array_index(yb, gdouble, sp+st-k);
													}
													{ofs/=8; ofe/=8;}
												}
												iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
												g_array_append_val(delf, iv);
												for (k=0; k<sp; k++)
												{
													clc=ofs+(k*(ofe-ofs)/(sp-1));
													if (clc<DZE && clc>NZE) y[k+(j*zp)]=-G_MAXDOUBLE;
													else
													{
														clc=-g_array_index(yb, gdouble, k+st)/clc;
														clc++;
														if (clc>DZE) y[k+(j*zp)]=log(clc);
														else y[k+(j*zp)]=-G_MAXDOUBLE;
													}
												}
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* -Rlssw+ +Rlssw+ */
								{
									ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
													if (clc>DZE) y[k+(j*zp)]=log(clc);
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0w+ */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
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
														sp=zp;
														{ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
														while (k<8)
														{
															ofs+=g_array_index(yb, gdouble, st+k);
															k++;
															ofe+=g_array_index(yb, gdouble, sp+st-k);
														}
														{ofs/=8; ofe/=8;}
													}
													else if (sp<16) {ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1);}
													else
													{
														{ofs=g_array_index(yb, gdouble, st*satl); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
														while (k<8)
														{
															ofs+=g_array_index(yb, gdouble, st+k);
															k++;
															ofe+=g_array_index(yb, gdouble, sp+st-k);
														}
														{ofs/=8; ofe/=8;}
													}
													iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
													g_array_append_val(delf, iv);
													for (k=0; k<sp; k++)
													{
														clc=ofs+(k*(ofe-ofs)/(sp-1))-g_array_index(yb, gdouble, k+st);
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
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +TdB0w+ */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
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
														sp=zp;
														{ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
														while (k<8)
														{
															ofs+=g_array_index(yb, gdouble, st+k);
															k++;
															ofe+=g_array_index(yb, gdouble, sp+st-k);
														}
														{ofs/=8; ofe/=8;}
													}
													else if (sp<16) {ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1);}
													else
													{
														{ofs=g_array_index(yb, gdouble, st*satl); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
														while (k<8)
														{
															ofs+=g_array_index(yb, gdouble, st+k);
															k++;
															ofe+=g_array_index(yb, gdouble, sp+st-k);
														}
														{ofs/=8; ofe/=8;}
													}
													iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
													g_array_append_val(delf, iv);
													for (k=0; k<sp; k++)
													{
														clc=g_array_index(yb, gdouble, k+st)-ofs-(k*(ofe-ofs)/(sp-1));
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
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0w+ */
								{
									ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +RdB0w+ */
								{
									ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0w+ +Tl0w+ */
							{
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
												sp=zp;
												{ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
												while (k<8)
												{
													ofs+=g_array_index(yb, gdouble, st+k);
													k++;
													ofe+=g_array_index(yb, gdouble, sp+st-k);
												}
												{ofs/=8; ofe/=8;}
											}
											else if (sp<16) {ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1);}
											else
											{
												{ofs=g_array_index(yb, gdouble, st*satl); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
												while (k<8)
												{
													ofs+=g_array_index(yb, gdouble, st+k);
													k++;
													ofe+=g_array_index(yb, gdouble, sp+st-k);
												}
												{ofs/=8; ofe/=8;}
											}
											iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
											g_array_append_val(delf, iv);
											for (k=0; k<sp; k++)
											{
												clc=ofs+(k*(ofe-ofs)/(sp-1));
												if (clc<DZE && clc>NZE) y[k+(j*zp)]=-G_MAXDOUBLE;
												else
												{
													clc=-g_array_index(yb, gdouble, k+st)/clc;
													y[k+(j*zp)]=++clc;
												}
											}
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* -Rl0w+ +Rl0w+ */
							{
								ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
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
										if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssw- */
										{
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													for (j=0; j<jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
										else /* +TdBssw- */
										{
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													for (j=0; j<jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
									}
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssw- */
									{
										ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +RdBssw- */
									{
										ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssw- +Tlssw- */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* -Rlssw- +Rlssw- */
								{
									ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0w- */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +TdB0w- */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0w- */
								{
									ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +RdB0w- */
								{
									ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0w- +Tl0w- */
							{
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* -Rl0w- +Rl0w- */
							{
								ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssw+ */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
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
														sp=zp;
														{ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
														while (k<8)
														{
															ofs+=g_array_index(yb, gdouble, st+k);
															k++;
															ofe+=g_array_index(yb, gdouble, sp+st-k);
														}
														{ofs/=8; ofe/=8;}
													}
													else if (sp<16) {ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1);}
													else
													{
														{ofs=g_array_index(yb, gdouble, st*satl); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
														while (k<8)
														{
															ofs+=g_array_index(yb, gdouble, st+k);
															k++;
															ofe+=g_array_index(yb, gdouble, sp+st-k);
														}
														{ofs/=8; ofe/=8;}
													}
													iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
													g_array_append_val(delf, iv);
													for (k=0; k<sp; k++)
													{
														clc=ofs+(k*(ofe-ofs)/(sp-1))-g_array_index(yb, gdouble, k+st);
														if (clc<NZE)
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
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +TdBssw+ */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
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
														sp=zp;
														{ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
														while (k<8)
														{
															ofs+=g_array_index(yb, gdouble, st+k);
															k++;
															ofe+=g_array_index(yb, gdouble, sp+st-k);
														}
														{ofs/=8; ofe/=8;}
													}
													else if (sp<16) {ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1);}
													else
													{
														{ofs=g_array_index(yb, gdouble, st*satl); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
														while (k<8)
														{
															ofs+=g_array_index(yb, gdouble, st+k);
															k++;
															ofe+=g_array_index(yb, gdouble, sp+st-k);
														}
														{ofs/=8; ofe/=8;}
													}
													iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
													g_array_append_val(delf, iv);
													for (k=0; k<sp; k++)
													{
														clc=g_array_index(yb, gdouble, k+st)-ofs-(k*(ofe-ofs)/(sp-1));
														if (clc<NZE)
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
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssw+ */
								{
									ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +RdBssw+ */
								{
									ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssw+ +Tlssw+ */
							{
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
												sp=zp;
												{ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
												while (k<8)
												{
													ofs+=g_array_index(yb, gdouble, st+k);
													k++;
													ofe+=g_array_index(yb, gdouble, sp+st-k);
												}
												{ofs/=8; ofe/=8;}
											}
											else if (sp<16) {ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1);}
											else
											{
												{ofs=g_array_index(yb, gdouble, st*satl); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
												while (k<8)
												{
													ofs+=g_array_index(yb, gdouble, st+k);
													k++;
													ofe+=g_array_index(yb, gdouble, sp+st-k);
												}
												{ofs/=8; ofe/=8;}
											}
											iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
											g_array_append_val(delf, iv);
											for (k=0; k<sp; k++)
											{
												clc=ofs+(k*(ofe-ofs)/(sp-1));
												if (clc<DZE && clc>NZE) y[k+(j*zp)]=-G_MAXDOUBLE;
												else
												{
													clc=-g_array_index(yb, gdouble, k+st)/ofs;
													clc++;
													if (clc>DZE) y[k+(j*zp)]=log(clc);
													else y[k+(j*zp)]=-G_MAXDOUBLE;
												}
											}
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* -Rlssw+ +Rlssw+ */
							{
								ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
												if (clc>DZE) y[k+(j*zp)]=log(clc);
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
							flags&=47;
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0w+ */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
													sp=zp;
													{ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
													while (k<8)
													{
														ofs+=g_array_index(yb, gdouble, st+k);
														k++;
														ofe+=g_array_index(yb, gdouble, sp+st-k);
													}
													{ofs/=8; ofe/=8;}
												}
												else if (sp<16) {ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1);}
												else
												{
													{ofs=g_array_index(yb, gdouble, st*satl); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
													while (k<8)
													{
														ofs+=g_array_index(yb, gdouble, st+k);
														k++;
														ofe+=g_array_index(yb, gdouble, sp+st-k);
													}
													{ofs/=8; ofe/=8;}
												}
												iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
												g_array_append_val(delf, iv);
												for (k=0; k<sp; k++)
												{
													clc=ofs+(k*(ofe-ofs)/(sp-1))-g_array_index(yb, gdouble, k+st);
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +TdB0w+ */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
													sp=zp;
													{ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
													while (k<8)
													{
														ofs+=g_array_index(yb, gdouble, st+k);
														k++;
														ofe+=g_array_index(yb, gdouble, sp+st-k);
													}
													{ofs/=8; ofe/=8;}
												}
												else if (sp<16) {ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1);}
												else
												{
													{ofs=g_array_index(yb, gdouble, st*satl); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
													while (k<8)
													{
														ofs+=g_array_index(yb, gdouble, st+k);
														k++;
														ofe+=g_array_index(yb, gdouble, sp+st-k);
													}
													{ofs/=8; ofe/=8;}
												}
												iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
												g_array_append_val(delf, iv);
												for (k=0; k<sp; k++)
												{
													clc=g_array_index(yb, gdouble, k+st)-ofs-(k*(ofe-ofs)/(sp-1));
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0w+ */
							{
								ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* +RdB0w+ */
							{
								ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0w+ +Tl0w+ */
						{
							flags&=47;
							for (m=0; m<mx; m++)
							{
								strat2=g_strsplit_set(strary2[m], "\t,", 0);
								if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
								{
									{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
									for (j=0; j<n; j++) y[j]=0;
									for (j=0; j<jdimx; j++)
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
											sp=zp;
											{ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
											while (k<8)
											{
												ofs+=g_array_index(yb, gdouble, st+k);
												k++;
												ofe+=g_array_index(yb, gdouble, sp+st-k);
											}
											{ofs/=8; ofe/=8;}
										}
										else if (sp<16) {ofs=g_array_index(yb, gdouble, st); ofe=g_array_index(yb, gdouble, sp+st-1);}
										else
										{
											{ofs=g_array_index(yb, gdouble, st*satl); ofe=g_array_index(yb, gdouble, sp+st-1); k=1;}
											while (k<8)
											{
												ofs+=g_array_index(yb, gdouble, st+k);
												k++;
												ofe+=g_array_index(yb, gdouble, sp+st-k);
											}
											{ofs/=8; ofe/=8;}
										}
										iv=(sp-1)/(zp*(g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st)));
										g_array_append_val(delf, iv);
										for (k=0; k<sp; k++)
										{
											clc=ofs+(k*(ofe-ofs)/(sp-1));
											if (clc<DZE && clc>NZE) y[k+(j*zp)]=-G_MAXDOUBLE;
											else
											{
												clc=-g_array_index(yb, gdouble, k+st)/clc;
												y[k+(j*zp)]=++clc;
											}
										}
									}
									fftw_execute(p);
									stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
									for (j=0; j<n; j++)
									{
										iv=star[j];
										g_array_append_val(stars, iv);
									}
									for (j=0; j<jdimx; j++)
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
										for (k=0; k<kdimx; k++)
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
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									g_error_free(Err);
								}
								g_free(contents);
								g_strfreev(strat2);
							}
						}
						else /* -Rl0w+ +Rl0w+ */
						{
							ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
							flags&=47;
							for (m=0; m<mx; m++)
							{
								strat2=g_strsplit_set(strary2[m], "\t,", 0);
								if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
								{
									{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
									for (j=0; j<n; j++) y[j]=0;
									for (j=0; j<jdimx; j++)
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
									for (j=0; j<jdimx; j++)
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
										for (k=0; k<kdimx; k++)
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
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									g_error_free(Err);
								}
								g_free(contents);
								g_strfreev(strat2);
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(oft)))
					{
						ofs-=oe;
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
											if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssh- */
											{
												for (m=0; m<mx; m++)
												{
													strat2=g_strsplit_set(strary2[m], "\t,", 0);
													if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
													{
														{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
														lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
														g_array_append_val(msr, lcl);
														if (lcl<xi) xi=lcl;
														else if (lcl>xf) xf=lcl;
														strary=g_strsplit_set(contents, "\r\n", 0);
														sal=g_strv_length(strary);
														{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
														delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
														for (j=0; j<n; j++) y[j]=0;
														ce=0;
														for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
														ce/=16;
														for (j=0; j<jdimx; j++)
														{
														}
														fftw_execute(p);
														stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
														for (j=0; j<n; j++)
														{
															iv=star[j];
															g_array_append_val(stars, iv);
														}
														for (j=0; j<jdimx; j++)
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
															for (k=0; k<kdimx; k++)
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
														str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
														gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
														g_free(str);
														g_error_free(Err);
													}
													g_free(contents);
													g_strfreev(strat2);
												}
											}
											else /* +TdBssh- */
											{
												for (m=0; m<mx; m++)
												{
													strat2=g_strsplit_set(strary2[m], "\t,", 0);
													if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
													{
														{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
														lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
														g_array_append_val(msr, lcl);
														if (lcl<xi) xi=lcl;
														else if (lcl>xf) xf=lcl;
														strary=g_strsplit_set(contents, "\r\n", 0);
														sal=g_strv_length(strary);
														{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
														delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
														for (j=0; j<n; j++) y[j]=0;
														ce=0;
														for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
														ce/=16;
														for (j=0; j<jdimx; j++)
														{
														}
														fftw_execute(p);
														stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
														for (j=0; j<n; j++)
														{
															iv=star[j];
															g_array_append_val(stars, iv);
														}
														for (j=0; j<jdimx; j++)
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
															for (k=0; k<kdimx; k++)
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
														str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													ce=0;
													for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
													ce/=16;
													for (j=0; j<jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
										else /* +RdBssh- */
										{
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													ce=0;
													for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
													ce/=16;
													for (j=0; j<jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
									}
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssh- +Tlssh- */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												ce=0;
												for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
												ce/=16;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* -Rlssh- +Rlssh- */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												ce=0;
												for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
												ce/=16;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													ce=0;
													for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
													ce/=16;
													for (j=0; j<jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
										else /* +TdB0h- */
										{
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													ce=0;
													for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
													ce/=16;
													for (j=0; j<jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
									}
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0h- */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												ce=0;
												for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
												ce/=16;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +RdB0h- */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												ce=0;
												for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
												ce/=16;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0h- +Tl0h- */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											ce=0;
											for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
											ce/=16;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* -Rl0h- +Rl0h- */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											ce=0;
											for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
											ce/=16;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
										if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssh+ */
										{
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													ce=0;
													for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
													ce/=16;
													for (j=0; j<jdimx; j++)
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
															clc=ofs+ce-g_array_index(yb, gdouble, k+st);
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
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
										else /* +TdBssh+ */
										{
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													ce=0;
													for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
													ce/=16;
													for (j=0; j<jdimx; j++)
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
															clc=g_array_index(yb, gdouble, k+st)-ofs-ce;
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
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
									}
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssh+ */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												ce=0;
												for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
												ce/=16;
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(ofs+ce-g_array_index(yb, gdouble, k+st));
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +RdBssh+ */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												ce=0;
												for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
												ce/=16;
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(g_array_index(yb, gdouble, k+st)-ofs-ce);
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssh+ +Tlssh+ */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											ce=0;
											for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
											ce/=16;
											for (j=0; j<jdimx; j++)
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
													clc=-g_array_index(yb, gdouble, k+st)/(ofs+ce);
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* -Rlssh+ +Rlssh+ */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											ce=0;
											for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
											ce/=16;
											for (j=0; j<jdimx; j++)
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
													clc=g_array_index(yb, gdouble, k+st)/(ofs+ce);
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0h+ */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												ce=0;
												for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
												ce/=16;
												for (j=0; j<jdimx; j++)
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
														clc=ofs+ce-g_array_index(yb, gdouble, k+st);
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
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +TdB0h+ */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												ce=0;
												for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
												ce/=16;
												for (j=0; j<jdimx; j++)
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
														clc=g_array_index(yb, gdouble, k+st)-ofs-ce;
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
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0h+ */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											ce=0;
											for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
											ce/=16;
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(ofs+ce-g_array_index(yb, gdouble, k+st)));
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +RdB0h+ */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											ce=0;
											for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
											ce/=16;
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(g_array_index(yb, gdouble, k+st)-ofs-ce));
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0h+ +Tl0h+ */
							{
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										ce=0;
										for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
										ce/=16;
										for (j=0; j<jdimx; j++)
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
												clc=-g_array_index(yb, gdouble, k+st)/(ofs+ce);
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* -Rl0h+ +Rl0h+ */
							{
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										ce=0;
										for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
										ce/=16;
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<sp; k++) y[k+(j*zp)]=g_array_index(yb, gdouble, k+st)/(ofs+ce);
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
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
										if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssh- */
										{
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													ce=0;
													for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
													ce/=16;
													for (j=0; j<jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
										else /* +TdBssh- */
										{
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													ce=0;
													for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
													ce/=16;
													for (j=0; j<jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
									}
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssh- */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												ce=0;
												for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
												ce/=16;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +RdBssh- */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												ce=0;
												for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
												ce/=16;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssh- +Tlssh- */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											ce=0;
											for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
											ce/=16;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* -Rlssh- +Rlssh- */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											ce=0;
											for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
											ce/=16;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0h- */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												ce=0;
												for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
												ce/=16;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +TdB0h- */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												ce=0;
												for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
												ce/=16;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0h- */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											ce=0;
											for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
											ce/=16;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +RdB0h- */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											ce=0;
											for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
											ce/=16;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0h- +Tl0h- */
							{
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										ce=0;
										for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
										ce/=16;
										for (j=0; j<jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* -Rl0h- +Rl0h- */
							{
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										ce=0;
										for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
										ce/=16;
										for (j=0; j<jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssh+ */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												ce=0;
												for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
												ce/=16;
												for (j=0; j<jdimx; j++)
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
														clc=ofs+ce-g_array_index(yb, gdouble, k+st);
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
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +TdBssh+ */
									{
										for (m=0; m<mx; m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												ce=0;
												for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
												ce/=16;
												for (j=0; j<jdimx; j++)
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
														clc=g_array_index(yb, gdouble, k+st)-ofs-ce;
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
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssh+ */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											ce=0;
											for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
											ce/=16;
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(ofs+ce-g_array_index(yb, gdouble, k+st));
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +RdBssh+ */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											ce=0;
											for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
											ce/=16;
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<sp; k++) y[k+(j*zp)]=0.1*(g_array_index(yb, gdouble, k+st)-ofs-ce);
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssh+ +Tlssh+ */
							{
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										ce=0;
										for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
										ce/=16;
										for (j=0; j<jdimx; j++)
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
												clc=-g_array_index(yb, gdouble, k+st)/(ofs+ce);
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* -Rlssh+ +Rlssh+ */
							{
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										ce=0;
										for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
										ce/=16;
										for (j=0; j<jdimx; j++)
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
												clc=g_array_index(yb, gdouble, k+st)/(ofs+ce);
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
							flags&=47;
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0h+ */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											ce=0;
											for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
											ce/=16;
											for (j=0; j<jdimx; j++)
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
													clc=ofs+ce-g_array_index(yb, gdouble, k+st);
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +TdB0h+ */
								{
									for (m=0; m<mx; m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											ce=0;
											for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
											ce/=16;
											for (j=0; j<jdimx; j++)
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
													clc=g_array_index(yb, gdouble, k+st)-ofs-ce;
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0h+ */
							{
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										ce=0;
										for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
										ce/=16;
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(ofs+ce-g_array_index(yb, gdouble, k+st)));
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* +RdB0h+ */
							{
								for (m=0; m<mx; m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										ce=0;
										for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
										ce/=16;
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<sp; k++) y[k+(j*zp)]=exp(LNTOT*(g_array_index(yb, gdouble, k+st)-ofs-ce));
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0h+ +Tl0h+ */
						{
							flags&=47;
							for (m=0; m<mx; m++)
							{
								strat2=g_strsplit_set(strary2[m], "\t,", 0);
								if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
								{
									{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
									for (j=0; j<n; j++) y[j]=0;
									ce=0;
									for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
									ce/=16;
									for (j=0; j<jdimx; j++)
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
											clc=-g_array_index(yb, gdouble, k+st)/(ofs+ce);
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
									for (j=0; j<jdimx; j++)
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
										for (k=0; k<kdimx; k++)
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
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									g_error_free(Err);
								}
								g_free(contents);
								g_strfreev(strat2);
							}
						}
						else /* -Rl0h+ +Rl0h+ */
						{
							flags&=47;
							for (m=0; m<mx; m++)
							{
								strat2=g_strsplit_set(strary2[m], "\t,", 0);
								if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
								{
									{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
									for (j=0; j<n; j++) y[j]=0;
									ce=0;
									for (j=1; j<=16; j++) ce+=g_array_index(specs, gdouble, trc-1+((lc-j)*satl));
									ce/=16;
									for (j=0; j<jdimx; j++)
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
										for (k=0; k<sp; k++) y[k+(j*zp)]=g_array_index(yb, gdouble, k+st)/(ofs+ce);
									}
									fftw_execute(p);
									stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
									for (j=0; j<n; j++)
									{
										iv=star[j];
										g_array_append_val(stars, iv);
									}
									for (j=0; j<jdimx; j++)
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
										for (k=0; k<kdimx; k++)
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
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									g_error_free(Err);
								}
								g_free(contents);
								g_strfreev(strat2);
							}
						}
						oe=ce;
						ofs+=oe;
						gtk_spin_button_set_value(GTK_SPIN_BUTTON(fst), ofs);
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(chi)))
					{
						ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
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
										if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBsso-c */
										{
											for (m=0; m<mx; m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													for (j=0; j<jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
										else /* +TdBsso-c */
										{
											for (m=0; m<g_strv_length(strary2); m++)
											{
												strat2=g_strsplit_set(strary2[m], "\t,", 0);
												if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
												{
													{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
													lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
													g_array_append_val(msr, lcl);
													if (lcl<xi) xi=lcl;
													else if (lcl>xf) xf=lcl;
													strary=g_strsplit_set(contents, "\r\n", 0);
													sal=g_strv_length(strary);
													{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
													delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
													for (j=0; j<n; j++) y[j]=0;
													for (j=0; j<jdimx; j++)
													{
													}
													fftw_execute(p);
													stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
													for (j=0; j<n; j++)
													{
														iv=star[j];
														g_array_append_val(stars, iv);
													}
													for (j=0; j<jdimx; j++)
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
														for (k=0; k<kdimx; k++)
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
													str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
													gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
													g_free(str);
													g_error_free(Err);
												}
												g_free(contents);
												g_strfreev(strat2);
											}
										}
									}
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBsso-c */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +RdBsso-c */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlsso-c +Tlsso-c */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* -Rlsso-c +Rlsso-c */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0o-c */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +TdB0o-c */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0o-c */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +RdB0o-c */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0o-c +Tl0o-c */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* -Rl0o-c +Rl0o-c */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBsso+c */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
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
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +TdBsso+c */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
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
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBsso+c */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +RdBsso+c */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlsso+c +Tlsso+c */
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
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else /* -Rlsso+c +Rlsso+c */
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
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0o+c */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +TdB0o+c */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0o+c */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* +RdB0o+c */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0o+c +Tl0o+c */
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
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
						}
						else /* -Rl0o+c +Rl0o+c */
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
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
						ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
						flags&=47;
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
								{
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBsso- */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
									else /* +TdBsso- */
									{
										for (m=0; m<g_strv_length(strary2); m++)
										{
											strat2=g_strsplit_set(strary2[m], "\t,", 0);
											if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
											{
												{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
												lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
												g_array_append_val(msr, lcl);
												if (lcl<xi) xi=lcl;
												else if (lcl>xf) xf=lcl;
												strary=g_strsplit_set(contents, "\r\n", 0);
												sal=g_strv_length(strary);
												{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
												delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
												for (j=0; j<n; j++) y[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
												fftw_execute(p);
												stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
												for (j=0; j<n; j++)
												{
													iv=star[j];
													g_array_append_val(stars, iv);
												}
												for (j=0; j<jdimx; j++)
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
													for (k=0; k<kdimx; k++)
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
												str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
												gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
												g_free(str);
												g_error_free(Err);
											}
											g_free(contents);
											g_strfreev(strat2);
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBsso- */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +RdBsso- */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlsso- +Tlsso- */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* -Rlsso- +Rlsso- */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0o- */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +TdB0o- */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
											fftw_execute(p);
											stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
											for (j=0; j<n; j++)
											{
												iv=star[j];
												g_array_append_val(stars, iv);
											}
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0o- */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* +RdB0o- */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
										fftw_execute(p);
										stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
										for (j=0; j<n; j++)
										{
											iv=star[j];
											g_array_append_val(stars, iv);
										}
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0o- +Tl0o- */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
								strat2=g_strsplit_set(strary2[m], "\t,", 0);
								if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
								{
									{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
									for (j=0; j<n; j++) y[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
									fftw_execute(p);
									stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
									for (j=0; j<n; j++)
									{
										iv=star[j];
										g_array_append_val(stars, iv);
									}
									for (j=0; j<jdimx; j++)
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
										for (k=0; k<kdimx; k++)
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
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									g_error_free(Err);
								}
								g_free(contents);
								g_strfreev(strat2);
							}
						}
						else /* -Rl0o- +Rl0o- */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
								strat2=g_strsplit_set(strary2[m], "\t,", 0);
								if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
								{
									{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
									for (j=0; j<n; j++) y[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
									fftw_execute(p);
									stars=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
									for (j=0; j<n; j++)
									{
										iv=star[j];
										g_array_append_val(stars, iv);
									}
									for (j=0; j<jdimx; j++)
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
										for (k=0; k<kdimx; k++)
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
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
						ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
						flags&=47;
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBsso+ */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
								else /* +TdBsso+ */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
										strat2=g_strsplit_set(strary2[m], "\t,", 0);
										if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
										{
											{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
											lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
											g_array_append_val(msr, lcl);
											if (lcl<xi) xi=lcl;
											else if (lcl>xf) xf=lcl;
											strary=g_strsplit_set(contents, "\r\n", 0);
											sal=g_strv_length(strary);
											{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
											delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
											for (j=0; j<n; j++) y[j]=0;
											for (j=0; j<jdimx; j++)
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
											for (j=0; j<jdimx; j++)
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
												for (k=0; k<kdimx; k++)
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
											str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
											gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
											g_free(str);
											g_error_free(Err);
										}
										g_free(contents);
										g_strfreev(strat2);
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBsso+ */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* +RdBsso+ */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
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
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
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
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
						ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
						flags&=47;
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0o+ */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
							else /* +TdB0o+ */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
									strat2=g_strsplit_set(strary2[m], "\t,", 0);
									if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
									{
										{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
										lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
										g_array_append_val(msr, lcl);
										if (lcl<xi) xi=lcl;
										else if (lcl>xf) xf=lcl;
										strary=g_strsplit_set(contents, "\r\n", 0);
										sal=g_strv_length(strary);
										{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
										delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
										for (j=0; j<n; j++) y[j]=0;
										for (j=0; j<jdimx; j++)
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
										for (j=0; j<jdimx; j++)
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
											for (k=0; k<kdimx; k++)
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
										str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
										gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
										g_free(str);
										g_error_free(Err);
									}
									g_free(contents);
									g_strfreev(strat2);
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0o+ */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
								strat2=g_strsplit_set(strary2[m], "\t,", 0);
								if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
								{
									{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
									for (j=0; j<n; j++) y[j]=0;
									for (j=0; j<jdimx; j++)
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
									for (j=0; j<jdimx; j++)
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
										for (k=0; k<kdimx; k++)
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
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									g_error_free(Err);
								}
								g_free(contents);
								g_strfreev(strat2);
							}
						}
						else /* +RdB0o+ */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
								strat2=g_strsplit_set(strary2[m], "\t,", 0);
								if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
								{
									{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
									for (j=0; j<n; j++) y[j]=0;
									for (j=0; j<jdimx; j++)
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
									for (j=0; j<jdimx; j++)
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
										for (k=0; k<kdimx; k++)
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
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									g_error_free(Err);
								}
								g_free(contents);
								g_strfreev(strat2);
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0o+ +Tl0o+ */
					{
						ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
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
									{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
									for (j=0; j<n; j++) y[j]=0;
									for (j=0; j<jdimx; j++)
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
									for (j=0; j<jdimx; j++)
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
										for (k=0; k<kdimx; k++)
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
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
									gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
									g_free(str);
									g_error_free(Err);
								}
								g_free(contents);
								g_strfreev(strat2);
							}
						}
					}
					else /* -Rl0o+ +Rl0o+ */
					{
						ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
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
									{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(delf, TRUE); g_array_free(stars, TRUE);}
									lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
									g_array_append_val(msr, lcl);
									if (lcl<xi) xi=lcl;
									else if (lcl>xf) xf=lcl;
									strary=g_strsplit_set(contents, "\r\n", 0);
									sal=g_strv_length(strary);
									{x=g_array_new(FALSE, FALSE, sizeof(gdouble)); yb=g_array_new(FALSE, FALSE, sizeof(gdouble));}
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
									delf=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
									for (j=0; j<n; j++) y[j]=0;
									for (j=0; j<jdimx; j++)
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
									for (j=0; j<jdimx; j++)
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
										for (k=0; k<kdimx; k++)
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
									str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
					bxr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimx*kdimx);
					byr=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), mx*jdimx*kdimx);
					if ((flagd&2)==0)
					{
						if ((flagd&4)==0)/* single plot mode */
						{
							bsz=g_array_new(FALSE, FALSE, sizeof(gint));
							g_array_append_val(bsz, mx);
							bnx=g_array_new(FALSE, FALSE, sizeof(gint));
							{dx2=mx*((jdim*kdimx)+kdim); l=0;}
							g_array_append_val(bnx, dx2);
							if ((flagd&16)==0)
							{
								if ((flagd&8)==0)
								{
									while (j<jdim)
									{
										for (k=0; k<kdimx; k++)
										{
											for (l=0; l<mx; l++)
											{
												xx=g_array_index(msr, gdouble, l);
												g_array_append_val(bxr, xx);
												xx=g_array_index(vis, gdouble, k+((j+(l*jdimx))*kdimx));
												g_array_append_val(byr, xx);
											}
										}
										j++;
									}
									k=0;
									while (k<kdim)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(vis, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
										}
										k++;
									}
									xx=g_array_index(msr, gdouble, 0);
									g_array_append_val(bxr, xx);
									mny=g_array_index(vis, gdouble, k+(j*kdimx));
									g_array_append_val(byr, xx);
									{mxy=mny; l=1;}
									while (l<mx)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(vis, gdouble, k+((j+(l*jdimx))*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
										l++;
									}
									k++;
									while (k<kdimx)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(vis, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
										}
										k++;
									}
									j++;
									while (j<jdimx)
									{
										for (k=0; k<kdimx; k++)
										{
											for (l=0; l<mx; l++)
											{
												xx=g_array_index(msr, gdouble, l);
												g_array_append_val(bxr, xx);
												xx=g_array_index(vis, gdouble, k+((j+(l*jdimx))*kdimx));
												g_array_append_val(byr, xx);
											}
										}
										j++;
									}
								}
								else
								{
									while (j<jdim)
									{
										for (k=0; k<kdimx; k++)
										{
											for (l=0; l<mx; l++)
											{
												xx=g_array_index(msr, gdouble, l);
												g_array_append_val(bxr, xx);
												xx=g_array_index(doms, gdouble, k+((j+(l*jdimx))*kdimx));
												g_array_append_val(byr, xx);
											}
										}
										j++;
									}
									k=0;
									while (k<kdim)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(doms, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
										}
										k++;
									}
									xx=g_array_index(msr, gdouble, 0);
									g_array_append_val(bxr, xx);
									mny=g_array_index(doms, gdouble, k+(j*kdimx));
									g_array_append_val(byr, xx);
									{mxy=mny; l=1;}
									while (l<mx)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(doms, gdouble, k+((j+(l*jdimx))*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
										l++;
									}
									k++;
									while (k<kdimx)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(doms, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
										}
										k++;
									}
									j++;
									while (j<jdimx)
									{
										for (k=0; k<kdimx; k++)
										{
											for (l=0; l<mx; l++)
											{
												xx=g_array_index(msr, gdouble, l);
												g_array_append_val(bxr, xx);
												xx=g_array_index(doms, gdouble, k+((j+(l*jdimx))*kdimx));
												g_array_append_val(byr, xx);
											}
										}
										j++;
									}
								}
							}
							else
							{
								while (j<jdim)
								{
									for (k=0; k<kdimx; k++)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(chp, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
										}
									}
									j++;
								}
								k=0;
								while (k<kdim)
								{
									for (l=0; l<mx; l++)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(chp, gdouble, k+((j+(l*jdimx))*kdimx));
										g_array_append_val(byr, xx);
									}
									k++;
								}
								xx=g_array_index(msr, gdouble, 0);
								g_array_append_val(bxr, xx);
								mny=g_array_index(chp, gdouble, k+(l*kdimx));
								g_array_append_val(byr, xx);
								{mxy=mny; l=1;}
								while (l<mx)
								{
									xx=g_array_index(msr, gdouble, l);
									g_array_append_val(bxr, xx);
									xx=g_array_index(chp, gdouble, k+((j+(l*jdimx))*kdimx));
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
									l++;
								}
								k++;
								while (k<kdimx)
								{
									for (l=0; l<mx; l++)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(chp, gdouble, k+((j+(l*jdimx))*kdimx));
										g_array_append_val(byr, xx);
									}
									k++;
								}
								j++;
								while (j<jdimx)
								{
									for (k=0; k<kdimx; k++)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(chp, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
										}
									}
									j++;
								}
							}
						}
						else/* multiplot over k */
						{
							bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimx);
							bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimx);
							{dx2=mx*kdimx*jdim; l=0;}
							if ((flagd&16)==0)
							{
								if ((flagd&8)==0)
								{
									while (j<jdim)
									{
										for (k=0; k<kdimx; k++)
										{
											for (l=0; l<mx; l++)
											{
												xx=g_array_index(msr, gdouble, l);
												g_array_append_val(bxr, xx);
												xx=g_array_index(vis, gdouble, k+((j+(l*jdimx))*kdimx));
												g_array_append_val(byr, xx);
											}
										}
										j++;
									}
									xx=g_array_index(msr, gdouble, 0);
									g_array_append_val(bxr, xx);
									mny=g_array_index(vis, gdouble, l*kdimx);
									g_array_append_val(byr, xx);
									{mxy=mny; l=1;}
									while (l<mx)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(vis, gdouble, (j+(l*jdimx))*kdimx);
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
										l++;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									{dx2+=mx; k=1;}
									while (k<kdimx)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(vis, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
											if (xx>mxy) mxy=xx;
											else if (xx<mny) mny=xx;
										}
										g_array_append_val(bsz, mx);
										g_array_append_val(bnx, dx2);
										{dx2+=mx; k++;}
									}
									j++;
									while (j<jdimx)
									{
										for (k=0; k<kdimx; k++)
										{
											for (l=0; l<mx; l++)
											{
												xx=g_array_index(msr, gdouble, l);
												g_array_append_val(bxr, xx);
												xx=g_array_index(vis, gdouble, k+((j+(l*jdimx))*kdimx));
												g_array_append_val(byr, xx);
											}
										}
										j++;
									}
								}
								else
								{
									while (j<jdim)
									{
										for (k=0; k<kdimx; k++)
										{
											for (l=0; l<mx; l++)
											{
												xx=g_array_index(msr, gdouble, l);
												g_array_append_val(bxr, xx);
												xx=g_array_index(doms, gdouble, k+((j+(l*jdimx))*kdimx));
												g_array_append_val(byr, xx);
											}
										}
										j++;
									}
									xx=g_array_index(msr, gdouble, 0);
									g_array_append_val(bxr, xx);
									mny=g_array_index(doms, gdouble, j*kdimx);
									g_array_append_val(byr, xx);
									{mxy=mny; l=1;}
									while (l<mx)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(doms, gdouble, (j+(l*jdimx))*kdimx);
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
										l++;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									{dx2+=mx; k=1;}
									while (k<kdimx)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(doms, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
											if (xx>mxy) mxy=xx;
											else if (xx<mny) mny=xx;
										}
										g_array_append_val(bsz, mx);
										g_array_append_val(bnx, dx2);
										{dx2+=mx; k++;}
									}
									j++;
									while (j<jdimx)
									{
										for (k=0; k<kdimx; k++)
										{
											for (l=0; l<mx; l++)
											{
												xx=g_array_index(msr, gdouble, l);
												g_array_append_val(bxr, xx);
												xx=g_array_index(doms, gdouble, k+((j+(l*jdimx))*kdimx));
												g_array_append_val(byr, xx);
											}
										}
										j++;
									}
								}
							}
							else
							{
								while (j<jdim)
								{
									for (k=0; k<kdimx; k++)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(vis, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
										}
									}
									j++;
								}
								xx=g_array_index(msr, gdouble, 0);
								g_array_append_val(bxr, xx);
								mny=g_array_index(chp, gdouble, j*kdimx);
								g_array_append_val(byr, xx);
								{mxy=mny; l=1;}
								while (l<mx)
								{
									xx=g_array_index(msr, gdouble, l);
									g_array_append_val(bxr, xx);
									xx=g_array_index(chp, gdouble, (j+(l*jdimx))*kdimx);
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
									l++;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								{dx2+=mx; k=1;}
								while (k<kdimx)
								{
									for (l=0; l<mx; l++)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(chp, gdouble, k+((j+(l*jdimx))*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									{dx2+=mx; k++;}
								}
								j++;
								while (j<jdimx)
								{
									for (k=0; k<kdimx; k++)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(chp, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
										}
									}
									j++;
								}
							}
						}
					}
					else if ((flagd&4)==0)/* multiplot over j */
					{
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx);
						bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx);
						{dx2=mx*kdim; k=0;}
						if ((flagd&16)==0)
						{
							if ((flagd&8)==0)
							{
								while (k<kdim)
								{
									for (l=0; l<mx; l++)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(vis, gdouble, k+(l*jdimx*kdimx));
										g_array_append_val(byr, xx);
									}
									k++;
								}
								xx=g_array_index(msr, gdouble, 0);
								g_array_append_val(bxr, xx);
								mny=g_array_index(vis, gdouble, k);
								g_array_append_val(byr, xx);
								{mxy=mny; l=1;}
								while (l<mx)
								{
									xx=g_array_index(msr, gdouble, l);
									g_array_append_val(bxr, xx);
									xx=g_array_index(vis, gdouble, k+(l*jdimx*kdimx));
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
									l++;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								{dx2+=mx; k++;}
								while (k<kdimx)
								{
									for (l=0; l<mx; l++)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(vis, gdouble, k+(l*jdimx*kdimx));
										g_array_append_val(byr, xx);
									}
									{dx2+=mx; k++;}
								}
								j=1;
								while (j<jdimx)
								{
									k=0;
									while (k<kdim)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(vis, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
										}
										{dx2+=mx; k++;}
									}
									for (l=0; l<mx; l++)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(vis, gdouble, k+((j+(l*jdimx))*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									{dx2+=mx; k++;}
									while (k<kdimx)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(vis, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
										}
										{dx2+=mx; k++;}
									}
									j++;
								}
							}
							else
							{
								while (k<kdim)
								{
									for (l=0; l<mx; l++)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(doms, gdouble, k+(l*jdimx*kdimx));
										g_array_append_val(byr, xx);
									}
									k++;
								}
								xx=g_array_index(msr, gdouble, 0);
								g_array_append_val(bxr, xx);
								mny=g_array_index(doms, gdouble, k);
								g_array_append_val(byr, xx);
								{mxy=mny; l=1;}
								while (l<mx)
								{
									xx=g_array_index(msr, gdouble, l);
									g_array_append_val(bxr, xx);
									xx=g_array_index(doms, gdouble, k+(l*jdimx*kdimx));
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
									l++;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								{dx2+=mx; k++;}
								while (k<kdimx)
								{
									for (l=0; l<mx; l++)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(doms, gdouble, k+((l*jdimx)*kdimx));
										g_array_append_val(byr, xx);
									}
									{dx2+=mx; k++;}
								}
								j=1;
								while (j<jdimx)
								{
									k=0;
									while (k<kdim)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(doms, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
										}
										{dx2+=mx; k++;}
									}
									for (l=0; l<mx; l++)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(doms, gdouble, k+((j+(l*jdimx))*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									{dx2+=mx; k++;}
									while (k<kdimx)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(doms, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
										}
										{dx2+=mx; k++;}
									}
									j++;
								}
							}
						}
						else
						{
							while (k<kdim)
							{
								for (l=0; l<mx; l++)
								{
									xx=g_array_index(msr, gdouble, l);
									g_array_append_val(bxr, xx);
									xx=g_array_index(chp, gdouble, k+((l*jdimx)*kdimx));
									g_array_append_val(byr, xx);
								}
								k++;
							}
							xx=g_array_index(msr, gdouble, 0);
							g_array_append_val(bxr, xx);
							mny=g_array_index(chp, gdouble, k);
							g_array_append_val(byr, xx);
							{mxy=mny; l=1;}
							while (l<mx)
							{
								xx=g_array_index(msr, gdouble, l);
								g_array_append_val(bxr, xx);
								xx=g_array_index(chp, gdouble, k+((l*jdimx)*kdimx));
								g_array_append_val(byr, xx);
								if (xx>mxy) mxy=xx;
								else if (xx<mny) mny=xx;
								l++;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							{dx2+=mx; k++;}
							while (k<kdimx)
							{
								for (l=0; l<mx; l++)
								{
									xx=g_array_index(msr, gdouble, l);
									g_array_append_val(bxr, xx);
									xx=g_array_index(chp, gdouble, k+((l*jdimx)*kdimx));
									g_array_append_val(byr, xx);
								}
								{dx2+=mx; k++;}
							}
							j=1;
							while (j<jdimx)
							{
								k=0;
								while (k<kdim)
								{
									for (l=0; l<mx; l++)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(chp, gdouble, k+((j+(l*jdimx))*kdimx));
										g_array_append_val(byr, xx);
									}
									{dx2+=mx; k++;}
								}
								for (l=0; l<mx; l++)
								{
									xx=g_array_index(msr, gdouble, l);
									g_array_append_val(bxr, xx);
									xx=g_array_index(chp, gdouble, k+((j+(l*jdimx))*kdimx));
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								{dx2+=mx; k++;}
								while (k<kdimx)
								{
									for (l=0; l<mx; l++)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(chp, gdouble, k+((j+(l*jdimx))*kdimx));
										g_array_append_val(byr, xx);
									}
									dx2+=mx;
									k++;
								}
								j++;
							}
						}
					}
					else /* both indices */
					{
						bnx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx*kdimx);
						bsz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx*kdimx);
						dx2=0;
						if ((flagd&16)==0)
						{
							if ((flagd&8)==0)
							{
								xx=g_array_index(msr, gdouble, 0);
								g_array_append_val(bxr, xx);
								mny=g_array_index(vis, gdouble, 0);
								g_array_append_val(byr, mny);
								mxy=mny;
								for (l=1; l<mx; l++)
								{
									xx=g_array_index(msr, gdouble, l);
									g_array_append_val(bxr, xx);
									xx=g_array_index(vis, gdouble, l*jdimx*kdimx);
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								{dx2+=mx; k=1;}
								while (k<kdimx)
								{
									for (l=0; l<mx; l++)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(vis, gdouble, k+(l*jdimx*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									{dx2+=mx; k++;}
								}
								j=1;
								while (j<jdimx)
								{
									for (k=0; k<kdimx; k++)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(vis, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
											if (xx>mxy) mxy=xx;
											else if (xx<mny) mny=xx;
										}
										g_array_append_val(bsz, mx);
										g_array_append_val(bnx, dx2);
										dx2+=mx;
									}
									j++;
								}
							}
							else
							{
								xx=g_array_index(msr, gdouble, 0);
								g_array_append_val(bxr, xx);
								mny=g_array_index(doms, gdouble, 0);
								g_array_append_val(byr, mny);
								mxy=mny;
								for (l=1; l<mx; l++)
								{
									xx=g_array_index(msr, gdouble, l);
									g_array_append_val(bxr, xx);
									xx=g_array_index(doms, gdouble, l*jdimx*kdimx);
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								{dx2+=mx; k=1;}
								while (k<kdimx)
								{
									for (l=0; l<mx; l++)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(doms, gdouble, k+(l*jdimx*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									{dx2+=mx; k++;}
								}
								j=1;
								while (j<jdimx)
								{
									for (k=0; k<kdimx; k++)
									{
										for (l=0; l<mx; l++)
										{
											xx=g_array_index(msr, gdouble, l);
											g_array_append_val(bxr, xx);
											xx=g_array_index(doms, gdouble, k+((j+(l*jdimx))*kdimx));
											g_array_append_val(byr, xx);
											if (xx>mxy) mxy=xx;
											else if (xx<mny) mny=xx;
										}
										g_array_append_val(bsz, mx);
										g_array_append_val(bnx, dx2);
										dx2+=mx;
									}
									j++;
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
							for (l=1; l<mx; l++)
							{
								xx=g_array_index(msr, gdouble, l);
								g_array_append_val(bxr, xx);
								xx=g_array_index(chp, gdouble, l*jdimx*kdimx);
								g_array_append_val(byr, xx);
								if (xx>mxy) mxy=xx;
								else if (xx<mny) mny=xx;
							}
							g_array_append_val(bsz, mx);
							g_array_append_val(bnx, dx2);
							{dx2+=mx; k=1;}
							while (k<kdimx)
							{
								for (l=0; l<mx; l++)
								{
									xx=g_array_index(msr, gdouble, l);
									g_array_append_val(bxr, xx);
									xx=g_array_index(chp, gdouble, k+(l*jdimx*kdimx));
									g_array_append_val(byr, xx);
									if (xx>mxy) mxy=xx;
									else if (xx<mny) mny=xx;
								}
								g_array_append_val(bsz, mx);
								g_array_append_val(bnx, dx2);
								{dx2+=mx; k++;}
							}
							j=1;
							while (j<jdimx)
							{
								for (k=0; k<kdimx; k++)
								{
									for (l=0; l<mx; l++)
									{
										xx=g_array_index(msr, gdouble, l);
										g_array_append_val(bxr, xx);
										xx=g_array_index(chp, gdouble, k+((j+(l*jdimx))*kdimx));
										g_array_append_val(byr, xx);
										if (xx>mxy) mxy=xx;
										else if (xx<mny) mny=xx;
									}
									g_array_append_val(bsz, mx);
									g_array_append_val(bnx, dx2);
									dx2+=mx;
								}
								j++;
							}
						}
					}
					dialog=gtk_dialog_new_with_buttons(_("Measurand Variable Type"), GTK_WINDOW(window), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Linear"), 1, _("Polar\n(Degrees)"), 2, _("Polar\n(Radians)"), 3, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
					cont=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
					label=gtk_label_new(_("Select Measurand Type:"));
					gtk_container_add(GTK_CONTAINER(cont), label);
					switch (gtk_dialog_run(GTK_DIALOG(dialog)))
					{
						case 1:
						if (((flags&8)==0)||((flags&32)!=0))
						{
							gtk_notebook_remove_page(GTK_NOTEBOOK(notebook2), 2);
							rest=gtk_table_new(1, 1, FALSE);
							gtk_widget_show(rest);
							plot3=plot_linear_new();
							g_signal_connect(plot3, "moved", G_CALLBACK(pltmv), NULL);
							gtk_widget_show(plot3);
							gtk_table_attach(GTK_TABLE(rest), plot3, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
							label=gtk_label_new(_("Analysis Results"));
							gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), rest, label);
							{flags|=15; flags&=31;}
						}
						plt=PLOT_LINEAR(plot3);
						if (mx<8) {(plt->flagd)=3; (plt->ptsize)=4;}
						else if (mx<20){(plt->flagd)=3; (plt->ptsize)=3;}
						else if (mx<50){(plt->flagd)=3; (plt->ptsize)=2;}
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
							rest=gtk_table_new(1, 1, FALSE);
							gtk_widget_show(rest);
							plot3=plot_polar_new();
							g_signal_connect(plot3, "moved", G_CALLBACK(pltmvp), NULL);
							gtk_widget_show(plot3);
							gtk_table_attach(GTK_TABLE(rest), plot3, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
							label=gtk_label_new(_("Analysis Results"));
							gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), rest, label);
							flags|=47;
						}
						plt2=PLOT_POLAR(plot3);
						if (mx<8) (plt2->ptsize)=4;
						else if (mx<20)(plt2->ptsize)=3;
						else if (mx<50)(plt2->ptsize)=2;
						else (plt2->flagd)=2;
						(plt2->thdata)=bxr;
						(plt2->rdata)=byr;
						(plt2->sizes)=bsz;
						(plt2->ind)=bnx;
						plot_polar_update_scale_pretty(plot3, mny, mxy, xi, xf);
						gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
						gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 2);
						break;
						case 3:
						if (((flags&8)==0)||((flags&32)==0))
						{
							gtk_notebook_remove_page(GTK_NOTEBOOK(notebook2), 2);
							rest=gtk_table_new(1, 1, FALSE);
							gtk_widget_show(rest);
							plot3=plot_polar_new();
							g_signal_connect(plot3, "moved", G_CALLBACK(pltmvp), NULL);
							gtk_widget_show(plot3);
							gtk_table_attach(GTK_TABLE(rest), plot3, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
							label=gtk_label_new(_("Analysis Results"));
							gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), rest, label);
							flags|=47;
						}
						plt2=PLOT_POLAR(plot3);
						if (mx<8) {(plt2->flagd)=7; (plt2->ptsize)=4;}
						else if (mx<20){(plt2->flagd)=7; (plt2->ptsize)=3;}
						else if (mx<50){(plt2->flagd)=7; (plt2->ptsize)=2;}
						else (plt2->flagd)=3;
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
					gtk_widget_destroy(dialog);
				}
				else
				{
					str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar *) Err);
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
				{g_array_free(x, TRUE); g_array_free(yb, TRUE); g_array_free(sz, TRUE); g_array_free(nx, TRUE); g_array_free(specs, TRUE);}
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
					list=(group2->next);
					gtk_widget_destroy(group2->data);
					group2=list;
					j--;
				}
				gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);
				gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 0);
				g_signal_connect(G_OBJECT(tr), "clicked", G_CALLBACK(trs), NULL);
				if (g_signal_handler_is_connected(G_OBJECT(pr), pr_id)) g_signal_handler_disconnect(G_OBJECT(pr), pr_id);
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
}

