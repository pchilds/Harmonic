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

void opd(GtkWidget *widget, gpointer data)
{
	PlotLinear *plt;
	PlotPolar *plt2;
	GtkWidget *wfile, *dialog, *content, *vbox, *table, *spin, *butt, *label, *cont, *trace;
	GtkAdjustment *adj;
	gdouble xi, xf, lcl, mny, mxy, idelf, iv, vzt, vt, ivd, ivdt, tcn, twd, phi, phio, phia, dst, ddp, pn, cn, tp, ct, ofs, clc, dx2, xx, ce;
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
				dialog=gtk_dialog_new_with_buttons(_("Configuration file generation"), GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_APPLY, NULL);
				g_signal_connect_swapped(G_OBJECT(dialog), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(dialog));
				gtk_widget_show(dialog);
				content=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
				vbox=gtk_vbox_new(FALSE, 0);
				gtk_widget_show(vbox);
				table=gtk_table_new(2, 2, FALSE);
				gtk_widget_show(table);
				label=gtk_label_new(_("Measurand\nValue:"));
				gtk_widget_show(label);
				gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
				label=gtk_label_new(_("File:"));
				gtk_widget_show(label);
				gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
				adj=(GtkAdjustment *) gtk_adjustment_new(0, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
				spin=gtk_spin_button_new(adj, 0, 0);
				gtk_widget_show(spin);
				gtk_table_attach(GTK_TABLE(table), spin, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
				butt=gtk_button_new(); /* thread to control updating table? */
				gtk_widget_show(butt);
				gtk_table_attach(GTK_TABLE(table), butt, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
				gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);
				gtk_container_add(GTK_CONTAINER(content), vbox);
				if (gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_APPLY)
				{
					dr=GTK_RESPONSE_ACCEPT;
				}
				gtk_widget_destroy(dialog);
			}
			if (dr!=GTK_RESPONSE_ACCEPT) gtk_widget_destroy(wfile);
			else
			{
				fin=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
				gtk_widget_destroy(wfile);
				if (g_file_get_contents(fin, &contents2, NULL, &Err))
				{
					strary2=g_strsplit_set(contents2, DLMT, 0);
					mx=g_strv_length(strary2);
					zp=1<<(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(zpd)));
					n=zp*jdimx;
					ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
					y=fftw_malloc(sizeof(double)*n);
					star=fftw_malloc(sizeof(double)*n);
					p=fftw_plan_many_r2r(1, &zp, jdimx, y, NULL, 1, zp, star, NULL, 1, zp, &type, FFTW_ESTIMATE);
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
											else /* +TdBss- */
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
										else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss- */
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
										else /* +RdBss- */
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
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss- +Tlss- */
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
									else /* -Rlss- +Rlss- */
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
										if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0- */
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
										else /* +TdB0- */
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
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0- */
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
									else /* +RdB0- */
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
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0- +Tl0- */
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
								else /* -Rl0- +Rl0- */
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
										if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss+ */
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
										else /* +TdBss+ */
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
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss+ */
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
									else /* +RdBss+ */
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
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss+ +Tlss+ */
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
								else /* -Rlss+ +Rlss+ */
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
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
								{
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0+ */
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
									else /* +TdB0+ */
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
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0+ */
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
								else /* +RdB0+ */
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
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0+ +Tl0+ */
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
							else /* -Rl0+ +Rl0+ */
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
										else /* +TdBss- */
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
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss- */
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
									else /* +RdBss- */
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
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss- +Tlss- */
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
								else /* -Rlss- +Rlss- */
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
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0- */
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
									else /* +TdB0- */
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
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0- */
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
								else /* +RdB0- */
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
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0- +Tl0- */
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
							else /* -Rl0- +Rl0- */
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
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss+ */
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
									else /* +TdBss+ */
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
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss+ */
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
								else /* +RdBss+ */
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
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss+ +Tlss+ */
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
							else /* -Rlss+ +Rlss+ */
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
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
						{
							flags&=47;
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0+ */
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
								else /* +TdB0+ */
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
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0+ */
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
							else /* +RdB0+ */
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
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0+ +Tl0+ */
						{
							flags&=47;
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
						else /* -Rl0+ +Rl0+ */
						{
							flags&=47;
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
											else /* +TdBss- */
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
										else /* +RdBss- */
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
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss- +Tlss- */
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
									else /* -Rlss- +Rlss- */
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
										else /* +TdB0- */
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
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0- */
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
									else /* +RdB0- */
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
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0- +Tl0- */
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
								else /* -Rl0- +Rl0- */
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
										if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss+ */
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
										else /* +TdBss+ */
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
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss+ */
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
									else /* +RdBss+ */
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
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss+ +Tlss+ */
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
								else /* -Rlss+ +Rlss+ */
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
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0+ */
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
									else /* +TdB0+ */
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
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0+ */
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
								else /* +RdB0+ */
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
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0+ +Tl0+ */
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
							else /* -Rl0+ +Rl0+ */
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
										else /* +TdBss- */
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
									else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss- */
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
									else /* +RdBss- */
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
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss- +Tlss- */
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
								else /* -Rlss- +Rlss- */
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
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0- */
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
									else /* +TdB0- */
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
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0- */
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
								else /* +RdB0- */
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
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0- +Tl0- */
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
							else /* -Rl0- +Rl0- */
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
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBss+ */
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
									else /* +TdBss+ */
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
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss+ */
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
								else /* +RdBss+ */
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
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss+ +Tlss+ */
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
							else /* -Rlss+ +Rlss+ */
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
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0+ */
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
								else /* +TdB0+ */
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
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0+ */
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
							else /* +RdB0+ */
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
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0+ +Tl0+ */
						{
							flags&=47;
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
						else /* -Rl0+ +Rl0+ */
						{
							flags&=47;
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
				str=g_strdup_printf(_("File: %s successfully loaded."), fin);
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

