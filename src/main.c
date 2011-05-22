/***************************************************************************
 *            main.c
 *
 *  A GTK+ program for analysing data in the Fourier domain
 *  version 0.1.0
 *  Features:
 *            Multiple data format input
 *            Graphical display of data and transformed result
 *            Batch Processing
 *            Analysis for visibility, domain shifts and chirp
 *            Publication ready graphs of results in postscript format
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
 * FFT: implement invert to 2pi/x routine
 * PRC: triangle optimisation
 * OPD: improve config writing utility to reorder post entry
 * OPD: extra zero on the end (varies due to situation)
 * DISP/UPJ/UPK debug changing index/multiplot
 * SAV: session save/restore routine
 * TRS: wavelets
 */

#include <gdk/gdkkeysyms.h>
#include <fftw3.h>
#include "main.h"
#include "open.h"
#include "disp.h"
#include "data.h"
#include "proc.h"
#include "util.h"

GtkWidget *window, *tr, *zpd, *pr, *tracmenu, *trac, *fst, *notebook, *notebook2, *plot1, *plot2, *plot3, *statusbar, *rest, *visl, *dsl, *chil;
GtkWidget *agosa, *agtl, *anosa, *sws, *dlm, *ncmp, *lcmp, *frr, *db4, *db8, *myr, *mrl, *bat, *chi, *twopionx, *opttri, *trans, *dBs, *neg, *wll, *oft;
GtkWidget *bsr, *bsp, *isr, *isp, *tc, *tw, *zw, *jind, *jind2, *kind; /* widgets for windowing */
GArray *bsra, *bspa, *isra, *ispa, *tca, *twa, *zwa, *x, *specs, *yb, *stars, *xsb, *ysb, *delf, *vis, *doms, *chp, *msr, *bxr, *byr; /* arrays for windowing and data */
GArray *sz, *nx, *sz2, *nx2, *bsz, *bnx, *rd1, *gr1, *bl1, *al1, *rd2, *gr2, *bl2, *al2, *rd3, *gr3, *bl3, *al3;
GSList *group2=NULL; /* list for various traces available and Basis functions for Transformation*/
gint lc, mx; /* number of data points and number of files in batch routine */
guint jdim=0, kdim=0, jdimx=1, kdimx=1, jdimxf=1, kdimxf=1, satl=0, trc=1, flags=0, flagd=0; /* array indices, #of traces, trace number, and current processing state and display flags */
gulong pr_id; /* id for disabling/enabling post-transform processing */
gdouble oe; /* value to hold prior reference level for offset tracking */
gchar *fold=NULL, *folr=NULL;

int main( int argc, char *argv[])
{
	GtkAdjustment *adj;
	GtkWidget *vbox, *mnb, *mnu, *smnu, *mni, *hpane, *table, *label, *butt;
	GtkAccelGroup *accel_group=NULL;
	GSList *group=NULL, *group3=NULL, *group4=NULL;
	PlotLinear *plt, *plt2;
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
	mni=gtk_menu_item_new_with_label(_("Restore Session"));
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(sessres), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	mni=gtk_menu_item_new_with_label(_("Save Session"));
	g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(sesssav), NULL);
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
	mni=gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
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
	mni=gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
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
	smnu=gtk_menu_new();
	frr=gtk_radio_menu_item_new_with_label(group4, _("Fourier"));
	group4=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(frr));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), frr);
	gtk_widget_show(frr);
	db4=gtk_radio_menu_item_new_with_label(group4, _("Daubechies 4"));
	group4=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(db4));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), db4);
	gtk_widget_show(db4);
	db8=gtk_radio_menu_item_new_with_label(group4, _("Daubechies 8"));
	group4=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(db8));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), db8);
	gtk_widget_show(db8);
	myr=gtk_radio_menu_item_new_with_label(group4, _("Meyer"));
	group4=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(myr));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), myr);
	gtk_widget_show(myr);
	mrl=gtk_radio_menu_item_new_with_label(group4, _("Morlet"));
	group4=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mrl));
	gtk_menu_shell_append(GTK_MENU_SHELL(smnu), mrl);
	gtk_widget_show(mrl);
	mni=gtk_menu_item_new_with_label(_("Basis Function:"));
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), smnu);
	twopionx=gtk_check_menu_item_new_with_label(_("Invert domain?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(twopionx), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), twopionx);
	gtk_widget_show(twopionx);
	wll=gtk_check_menu_item_new_with_label(_("Apply correction\nto Window edges?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wll), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), wll);
	gtk_widget_show(wll);
	mni=gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	opttri=gtk_check_menu_item_new_with_label(_("Optimise Triangle fit?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(opttri), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), opttri);
	gtk_widget_show(opttri);
	chi=gtk_check_menu_item_new_with_label(_("Calculate Chirp?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(chi), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), chi);
	gtk_widget_show(chi);
	mni=gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
	gtk_widget_show(mni);
	bat=gtk_check_menu_item_new_with_label(_("Batch Process Data?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bat), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), bat);
	gtk_widget_show(bat);
	oft=gtk_check_menu_item_new_with_label(_("Autotrack Offset?"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(oft), FALSE);
	gtk_menu_shell_append(GTK_MENU_SHELL(mnu), oft);
	gtk_widget_show(oft);
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
	hpane=gtk_hpaned_new();
	gtk_widget_show(hpane);
	gtk_container_add(GTK_CONTAINER(vbox), hpane);
	bsra=g_array_new(FALSE, FALSE, sizeof(gdouble));
	bspa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	isra=g_array_new(FALSE, FALSE, sizeof(gdouble));
	ispa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	tca=g_array_new(FALSE, FALSE, sizeof(gdouble));
	twa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	zwa=g_array_new(FALSE, FALSE, sizeof(gdouble));
	g_array_append_val(bsra, fll);
	fll++;
	g_array_append_val(bspa, fll);
	g_array_append_val(isra, fll);
	fll++;
	g_array_append_val(tca, fll);
	g_array_append_val(twa, fll);
	g_array_append_val(zwa, fll);
	fll++;
	g_array_append_val(ispa, fll);
	notebook=gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
	table=gtk_table_new(5, 3, FALSE);
	gtk_widget_show(table);
	label=gtk_label_new(_("Spectrum Start:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(0, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	bsr=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(bsr), "value-changed", G_CALLBACK(upa1), (gpointer) bsra);
	gtk_table_attach(GTK_TABLE(table), bsr, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(bsr);
	label=gtk_label_new(_("Spectrum Stop:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(1, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	bsp=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(bsp), "value-changed", G_CALLBACK(upa2), (gpointer) bspa);
	gtk_table_attach(GTK_TABLE(table), bsp, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(bsp);
	label=gtk_label_new(_("Offset:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(1, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	fst=gtk_spin_button_new(adj, 0.5, 2);
	gtk_table_attach(GTK_TABLE(table), fst, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(fst);
	label=gtk_label_new(_("j index:"));
	gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(0, 0, G_MAXINT8, 1.0, 5.0, 0.0);
	jind=gtk_spin_button_new(adj, 0, 0);
	g_signal_connect(G_OBJECT(jind), "value-changed", G_CALLBACK(upj), NULL);
	gtk_table_attach(GTK_TABLE(table), jind, 2, 3, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(jind);
	label=gtk_label_new(_("Zero Padding 2^:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment*) gtk_adjustment_new(12, 4, 31, 1.0, 5.0, 0.0);
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
	adj=(GtkAdjustment *) gtk_adjustment_new(1, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	isr=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(isr), "value-changed", G_CALLBACK(upa2), (gpointer) isra);
	gtk_table_attach(GTK_TABLE(table), isr, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(isr);
	label=gtk_label_new(_("Inverse Spectrum Stop:"));
	gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(3, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
	isp=gtk_spin_button_new(adj, 0.5, 3);
	g_signal_connect(G_OBJECT(isp), "value-changed", G_CALLBACK(upa2), (gpointer) ispa);
	gtk_table_attach(GTK_TABLE(table), isp, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(isp);
	label=gtk_label_new(_("Triangle Centre:"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	gtk_widget_show(label);
	adj=(GtkAdjustment *) gtk_adjustment_new(2, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
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
	rd1=g_array_new(FALSE, FALSE, sizeof(gdouble));
	gr1=g_array_new(FALSE, FALSE, sizeof(gdouble));
	bl1=g_array_new(FALSE, FALSE, sizeof(gdouble));
	al1=g_array_new(FALSE, FALSE, sizeof(gdouble));
	fll=0;
	g_array_append_val(rd1, fll);
	g_array_append_val(gr1, fll);
	g_array_append_val(bl1, fll);
	fll=1;
	g_array_append_val(al1, fll);
	plt=PLOT_LINEAR(plot1);
	(plt->rd)=rd1;
	(plt->gr)=gr1;
	(plt->bl)=bl1;
	(plt->al)=al1;
	g_signal_connect(plot1, "moved", G_CALLBACK(pltmv), NULL);
	gtk_widget_show(plot1);
	gtk_table_attach(GTK_TABLE(table), plot1, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK |GTK_EXPAND, 2, 2);
	label=gtk_label_new(_("Spectrum"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), table, label);
	table=gtk_table_new(1, 1, FALSE);
	gtk_widget_show(table);
	plot2=plot_linear_new();
	plt2=PLOT_LINEAR(plot2);
	(plt2->xlab)=g_strdup(_("Inverse Domain"));
	rd2=g_array_new(FALSE, FALSE, sizeof(gdouble));
	gr2=g_array_new(FALSE, FALSE, sizeof(gdouble));
	bl2=g_array_new(FALSE, FALSE, sizeof(gdouble));
	al2=g_array_new(FALSE, FALSE, sizeof(gdouble));
	fll=0;
	g_array_append_val(rd2, fll);
	g_array_append_val(gr2, fll);
	g_array_append_val(bl2, fll);
	g_array_append_val(gr2, fll);
	g_array_append_val(bl2, fll);
	g_array_append_val(bl2, fll);
	fll=1;
	g_array_append_val(rd2, fll);
	g_array_append_val(gr2, fll);
	g_array_append_val(bl2, fll);
	fll=0;
	g_array_append_val(rd2, fll);
	g_array_append_val(gr2, fll);
	g_array_append_val(rd2, fll);
	fll=0.8;
	g_array_append_val(al2, fll);
	g_array_append_val(al2, fll);
	g_array_append_val(al2, fll);
	g_array_append_val(al2, fll);
	(plt2->rd)=rd2;
	(plt2->gr)=gr2;
	(plt2->bl)=bl2;
	(plt2->al)=al2;
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
	bxr=g_array_new(FALSE, FALSE, sizeof(gdouble));
	byr=g_array_new(FALSE, FALSE, sizeof(gdouble));
	sz=g_array_new(FALSE, FALSE, sizeof(gint));
	nx=g_array_new(FALSE, FALSE, sizeof(gint));
	sz2=g_array_new(FALSE, FALSE, sizeof(gint));
	nx2=g_array_new(FALSE, FALSE, sizeof(gint));
	bsz=g_array_new(FALSE, FALSE, sizeof(gint));
	bnx=g_array_new(FALSE, FALSE, sizeof(gint));
	rd3=g_array_new(FALSE, FALSE, sizeof(gdouble));
	gr3=g_array_new(FALSE, FALSE, sizeof(gdouble));
	bl3=g_array_new(FALSE, FALSE, sizeof(gdouble));
	al3=g_array_new(FALSE, FALSE, sizeof(gdouble));
	fold=g_strdup("/home");
	folr=g_strdup("/home");
	gtk_widget_show(window);
	gtk_main();
	gdk_threads_leave();
	return 0;
}
