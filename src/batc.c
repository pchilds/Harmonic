/***************************************************************************
 *            batc.c
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

#include "batc.h"
GtkWidget *adb, *cft, *dialog, *lbl1, *lbl2, *spin;
GList *conff=NULL, *confv=NULL;
GSList *confb=NULL;

void tsw(GtkWidget* widget, gpointer dta)
{
	gchar *s1=NULL, *s2=NULL;
	gdouble val, val2;
	GList *iter=NULL;
	guint ps;

	ps=GPOINTER_TO_INT(dta);
	iter=g_list_nth(conff, ps);
	s2=g_strdup(gtk_label_get_label(GTK_LABEL(iter->data)));
	iter=(iter->prev);
	s1=g_strdup(gtk_label_get_label(GTK_LABEL(iter->data)));
	gtk_label_set_label(GTK_LABEL(iter->data), s2);
	iter=(iter->next);
	gtk_label_set_label(GTK_LABEL(iter->data), s1);
	{g_free(s1); g_free(s2);}
	iter=g_list_nth(confv, ps);
	val2=gtk_spin_button_get_value(GTK_SPIN_BUTTON(iter->data));
	iter=(iter->prev);
	val=gtk_spin_button_get_value(GTK_SPIN_BUTTON(iter->data));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(iter->data), val2);
	iter=(iter->next);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(iter->data), val);
}

void tdl(GtkWidget* widget, gpointer dta)
{
	GList *iter1=NULL, *iter2=NULL;
	gchar *s=NULL;
	gdouble val;
	guint ps;
	gint rw;

	g_object_get(G_OBJECT(cft), "n-rows", &rw, NULL);
	rw-=3;
	if (rw<=2)
	{
		gtk_container_remove(GTK_CONTAINER(cft), widget);
		gtk_container_remove(GTK_CONTAINER(cft), GTK_WIDGET(conff->data));
		gtk_container_remove(GTK_CONTAINER(cft), GTK_WIDGET(confv->data));
		{g_slist_free(confb); g_list_free(conff); g_list_free(confv);}
		{confb=NULL; conff=NULL; confv=NULL;}
	}
	else
	{
		ps=GPOINTER_TO_INT(dta);
		gtk_container_remove(GTK_CONTAINER(cft), GTK_WIDGET(confb->data));
		confb=(confb->next);
		gtk_container_remove(GTK_CONTAINER(cft), GTK_WIDGET(confb->data));
		confb=(confb->next);
		gtk_container_remove(GTK_CONTAINER(cft), GTK_WIDGET(confb->data));
		confb=(confb->next);
		{iter1=(g_list_nth(conff, ps))->next; iter2=(g_list_nth(confv, ps))->next;}
		while (iter1)
		{
			s=g_strdup(gtk_label_get_label(GTK_LABEL(iter1->data)));
			gtk_label_set_label(GTK_LABEL((iter1->prev)->data), s);
			val=gtk_spin_button_get_value(GTK_SPIN_BUTTON(iter2->data));
			gtk_spin_button_set_value(GTK_SPIN_BUTTON((iter2->prev)->data), val);
			{g_free(s); iter1=(iter1->next); iter2=(iter2->next);}
		}
		iter1=g_list_last(conff);
		iter2=(iter1->prev);
		{(iter2->next)=NULL; (iter1->prev)=NULL;}
		gtk_container_remove(GTK_CONTAINER(cft), GTK_WIDGET(iter1->data));
		iter1=g_list_last(confv);
		iter2=(iter1->prev);
		{(iter2->next)=NULL; (iter1->prev)=NULL;}
		gtk_container_remove(GTK_CONTAINER(cft), GTK_WIDGET(iter1->data));
	}
	g_object_ref(G_OBJECT(adb));
	gtk_container_remove(GTK_CONTAINER(cft), adb);
	gtk_table_attach(GTK_TABLE(cft), adb, 1, 2, rw-1, rw, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	{g_object_unref(G_OBJECT(adb)); g_object_ref(G_OBJECT(spin));}
	gtk_container_remove(GTK_CONTAINER(cft), spin);
	gtk_table_attach(GTK_TABLE(cft), spin, 2, 3, rw-1, rw, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
	g_object_unref(G_OBJECT(spin));
	gtk_table_resize(GTK_TABLE(cft), rw, 3);
}

void upt(GtkWidget* widget, gpointer dta)
{
	AtkObject *atk_lbl, *atk_ll, *atk_wgt;
	gdouble val;
	gint rw, j;
	gint ps;
	GList *chld;
	GSList *fls=NULL, *list;
	GtkAdjustment *adj;
	GtkWidget *btd, *btt, *btu, *lbl, *wwfile;

	wwfile=gtk_file_chooser_dialog_new(_("Select Data File(s)"), GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(wwfile), TRUE);
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(wwfile), FALSE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wwfile), fold);
	g_signal_connect(G_OBJECT(wwfile), "destroy", G_CALLBACK(gtk_widget_destroy), G_OBJECT(wwfile));
	if (gtk_dialog_run(GTK_DIALOG(wwfile))==GTK_RESPONSE_ACCEPT)
	{
		g_object_get(G_OBJECT(cft), "n-rows", &rw, NULL);
		ps=(rw-2)/3;
		fls=gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(wwfile));
		j=g_slist_length(fls);
		gtk_table_resize(GTK_TABLE(cft), rw+(3*j), 3);
		val=gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin));
		g_object_ref(G_OBJECT(widget));
		gtk_container_remove(GTK_CONTAINER(cft), widget);
		atk_lbl=gtk_widget_get_accessible(GTK_WIDGET(lbl2));
		if (ps==0)
		{
			lbl=gtk_label_new((gchar*) (fls->data));
			gtk_table_attach(GTK_TABLE(cft), lbl, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			conff=g_list_append(conff, (gpointer) lbl);
			g_free(fls->data);
			fls=(fls->next);
			btt=gtk_button_new_from_stock(GTK_STOCK_DELETE);
			g_signal_connect(G_OBJECT(btt), "clicked", G_CALLBACK(tdl), GINT_TO_POINTER(ps));
			gtk_table_attach(GTK_TABLE(cft), btt, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_widget_show(btt);
			confb=g_slist_prepend(confb, (gpointer) btt);
			atk_wgt=gtk_widget_get_accessible(lbl1);
			atk_ll=gtk_widget_get_accessible(GTK_WIDGET(lbl));
			atk_object_add_relationship(atk_wgt, ATK_RELATION_LABEL_FOR, atk_ll);
			atk_object_add_relationship(atk_ll, ATK_RELATION_LABELLED_BY, atk_wgt);
			atk_wgt=gtk_widget_get_accessible(spin);
			atk_object_add_relationship(atk_ll, ATK_RELATION_LABEL_FOR, atk_wgt);
			atk_object_add_relationship(atk_wgt, ATK_RELATION_LABELLED_BY, atk_ll);
			confv=g_list_append(confv, (gpointer) spin);
			{ps++; rw+=3;}
			adj=(GtkAdjustment*) gtk_adjustment_new(val, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
			spin=gtk_spin_button_new(adj, 0, 0);
			gtk_widget_show(spin);
			gtk_table_attach(GTK_TABLE(cft), spin, 2, 3, 4, 5, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			atk_wgt=gtk_widget_get_accessible(spin);
			atk_object_add_relationship(atk_lbl, ATK_RELATION_LABEL_FOR, atk_wgt);
			atk_object_add_relationship(atk_wgt, ATK_RELATION_LABELLED_BY, atk_lbl);
		}
		else atk_ll=gtk_widget_get_accessible(GTK_WIDGET(g_list_last(conff)->data));
		while (fls)
		{
			btd=gtk_button_new_from_stock(GTK_STOCK_GO_DOWN);
			g_signal_connect(G_OBJECT(btd), "clicked", G_CALLBACK(tsw), GINT_TO_POINTER(ps));
			gtk_table_attach(GTK_TABLE(cft), btd, 0, 1, rw-3, rw-2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_widget_show(btd);
			confb=g_slist_prepend(confb, (gpointer) btd);
			btu=gtk_button_new_from_stock(GTK_STOCK_GO_UP);
			g_signal_connect(G_OBJECT(btu), "clicked", G_CALLBACK(tsw), GINT_TO_POINTER(ps));
			gtk_table_attach(GTK_TABLE(cft), btu, 0, 1, rw-2, rw-1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_widget_show(btu);
			confb=g_slist_prepend(confb, (gpointer) btu);
			atk_wgt=gtk_widget_get_accessible(btd);
			atk_object_add_relationship(atk_ll, ATK_RELATION_CONTROLLED_BY, atk_wgt);
			atk_object_add_relationship(atk_wgt, ATK_RELATION_CONTROLLER_FOR, atk_ll);
			atk_wgt=gtk_widget_get_accessible(btu);
			atk_object_add_relationship(atk_ll, ATK_RELATION_CONTROLLED_BY, atk_wgt);
			atk_object_add_relationship(atk_wgt, ATK_RELATION_CONTROLLER_FOR, atk_ll);
			lbl=gtk_label_new((gchar*) (fls->data));
			gtk_table_attach(GTK_TABLE(cft), lbl, 1, 2, rw-1, rw, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			conff=g_list_append(conff, (gpointer) lbl);
			g_free(fls->data);
			fls=(fls->next);
			atk_wgt=gtk_widget_get_accessible(lbl1);
			atk_ll=gtk_widget_get_accessible(GTK_WIDGET(lbl));
			atk_object_add_relationship(atk_wgt, ATK_RELATION_LABEL_FOR, atk_ll);
			atk_object_add_relationship(atk_ll, ATK_RELATION_LABELLED_BY, atk_wgt);
			atk_wgt=gtk_widget_get_accessible(btd);
			atk_object_add_relationship(atk_ll, ATK_RELATION_CONTROLLED_BY, atk_wgt);
			atk_object_add_relationship(atk_wgt, ATK_RELATION_CONTROLLER_FOR, atk_ll);
			atk_wgt=gtk_widget_get_accessible(btu);
			atk_object_add_relationship(atk_ll, ATK_RELATION_CONTROLLED_BY, atk_wgt);
			atk_object_add_relationship(atk_wgt, ATK_RELATION_CONTROLLER_FOR, atk_ll);
			btt=gtk_button_new_from_stock(GTK_STOCK_DELETE);
			g_signal_connect(G_OBJECT(btt), "clicked", G_CALLBACK(tdl), GINT_TO_POINTER(ps));
			gtk_table_attach(GTK_TABLE(cft), btt, 0, 1, rw-1, rw, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			gtk_widget_show(btt);
			confb=g_slist_prepend(confb, (gpointer) btt);
			atk_wgt=gtk_widget_get_accessible(btt);
			atk_object_add_relationship(atk_ll, ATK_RELATION_CONTROLLED_BY, atk_wgt);
			atk_object_add_relationship(atk_wgt, ATK_RELATION_CONTROLLER_FOR, atk_ll);
			atk_wgt=gtk_widget_get_accessible(spin);
			atk_object_add_relationship(atk_ll, ATK_RELATION_LABEL_FOR, atk_wgt);
			atk_object_add_relationship(atk_wgt, ATK_RELATION_LABELLED_BY, atk_ll);
			confv=g_list_append(confv, (gpointer) spin);
			{ps++; rw+=3;}
			adj=(GtkAdjustment*) gtk_adjustment_new(val, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
			spin=gtk_spin_button_new(adj, 0, 0);
			gtk_widget_show(spin);
			gtk_table_attach(GTK_TABLE(cft), spin, 2, 3, rw-1, rw, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			atk_wgt=gtk_widget_get_accessible(spin);
			atk_object_add_relationship(atk_lbl, ATK_RELATION_LABEL_FOR, atk_wgt);
			atk_object_add_relationship(atk_wgt, ATK_RELATION_LABELLED_BY, atk_lbl);
		}
		gtk_table_attach(GTK_TABLE(cft), widget, 1, 2, rw-1, rw, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
		g_object_unref(G_OBJECT(widget));
	}
	gtk_widget_destroy(wwfile);
}


void bat(GtkWidget *widget, gpointer data)
{
	AtkObject *atk_lbl, *atk_wgt;
	double *yt, *star;
	fftw_plan p;
	fftw_r2r_kind type=FFTW_R2HC;
	GArray *delp, *starp, *nx, *st, *sz, *x, *xp, *y, *yp;
	gchar *contents, *contents2, *fin=NULL, *str, *s2;
	gchar **strary, **strary2, **strat, **strat2;
	gchar s1[10];
	gdouble ce, clc, cn, ct, ddp, dst, idelf, iv, ivd, ivdt, lcl, mny, mxy, ofe, ofs, phi, phia, phio, pn, tcn, tp, twd, vt, vzt, xi, xf, xx;
	GError *Err=NULL;
	gint dr, dx, dx2, j, jdim, jdimx, k, kdim, kib, l, lc, lcib, m, mx, n, sal, sr, sp, trc, zp;
	GList *iter1, *iter2;
	GSList *list;
	GtkAdjustment *adj, *adj2;
	GtkPlotLinear *plt;
	GtkPlotPolar *plt2;
	GtkWidget *cont, *content, *hbox, *label, *scroll, *vsc, *wfile;
	struct confrow *row;

	wfile=gtk_file_chooser_dialog_new(_("Select Config File"), GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_NEW, GTK_RESPONSE_APPLY, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(wfile), FALSE);
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(wfile), FALSE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(wfile), folr);
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
			content=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
			cft=gtk_table_new(2, 3, FALSE);
			gtk_widget_show(cft);
			lbl1=gtk_label_new(_("File:"));
			gtk_widget_show(lbl1);
			gtk_table_attach(GTK_TABLE(cft), lbl1, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			lbl2=gtk_label_new(_("Measurand\nValue:"));
			gtk_widget_show(lbl2);
			gtk_table_attach(GTK_TABLE(cft), lbl2, 2, 3, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			adb=gtk_button_new_with_label(_("Add another file"));
			gtk_widget_show(adb);
			gtk_table_attach(GTK_TABLE(cft), adb, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			g_signal_connect(G_OBJECT(adb), "clicked", G_CALLBACK(upt), NULL);
			adj=(GtkAdjustment*) gtk_adjustment_new(0, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
			spin=gtk_spin_button_new(adj, 0, 0);
			gtk_widget_show(spin);
			gtk_table_attach(GTK_TABLE(cft), spin, 2, 3, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
			atk_wgt=gtk_widget_get_accessible(spin);
			atk_lbl=gtk_widget_get_accessible(GTK_WIDGET(lbl2));
			atk_object_add_relationship(atk_lbl, ATK_RELATION_LABEL_FOR, atk_wgt);
			atk_object_add_relationship(atk_wgt, ATK_RELATION_LABELLED_BY, atk_lbl);
			adj=(GtkAdjustment*) gtk_adjustment_new(0, 0, 0, 0.0, 0.0, 0.0);
			adj2=(GtkAdjustment*) gtk_adjustment_new(0, -G_MAXDOUBLE, G_MAXDOUBLE, 1.0, 5.0, 0.0);
			scroll=gtk_scrolled_window_new(adj, adj2);
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
			gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll), cft);
			gtk_widget_show(scroll);
			gtk_box_pack_start(GTK_BOX(content), scroll, TRUE, TRUE, 2);
			if (gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_APPLY)
			{
				mx=0;
				{iter1=g_list_first(conff); iter2=g_list_first(confv);}
				if (iter1)
				{
					s2=g_strdup(gtk_label_get_label(GTK_LABEL(iter1->data)));
					iv=gtk_spin_button_get_value(GTK_SPIN_BUTTON(iter2->data));
					{iter1=(iter1->next); iter2=(iter2->next);}
					g_snprintf(s1, 10, "%f", iv);
					contents2=g_strjoin("\t", s1, s2, NULL);
					g_free(s2);
					mx++;
					while (iter1)
					{
						s2=g_strdup(gtk_label_get_label(GTK_LABEL(iter1->data)));
						iter1=(iter1->next);
						iv=gtk_spin_button_get_value(GTK_SPIN_BUTTON(iter2->data));
						iter2=(iter2->next);
						g_snprintf(s1, 10, "%f", iv);
						str=g_strjoin("\t", s1, s2, NULL);
						contents=g_strdup(contents2);
						{g_free(s2); g_free(contents2);}
						contents2=g_strjoin(DLMT, contents, str, NULL);
						{g_free(contents); g_free(str);}
						mx++;
					}
					g_file_set_contents(fin, contents2, -1, &Err);
					g_free(contents2);
					if (Err)
					{
						str=g_strdup_printf(_("Error Saving file: %s."), (Err->message));
						gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
						{g_free(str); g_error_free(Err);}
					}
					else dr=GTK_RESPONSE_ACCEPT;
				}
				else
				{
					str=g_strdup(_("No filenames given."));
					gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
					g_free(str);
				}
			}
			{g_list_free(conff); g_list_free(confv); g_slist_free(confb); gtk_widget_destroy(dialog);}
		}
		g_free(fin);
	}
	if (dr!=GTK_RESPONSE_ACCEPT) gtk_widget_destroy(wfile);
	else
	{
		g_free(folr);
		folr=gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wfile));
		fin=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
		gtk_widget_destroy(wfile);
		if (g_file_get_contents(fin, &contents2, NULL, &Err))
		{
			{g_array_free(vis, FALSE); g_array_free(doms, FALSE); g_array_free(chp, FALSE);}
			strary2=g_strsplit_set(contents2, DLMT, 0);
			mx=g_strv_length(strary2);
			zp=1<<(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(zpd)));
			jdimx=(bsra->len);
			n=zp*jdimx;
			kdimxf=(isra->len)/kdimxf;
			yt=fftw_malloc(sizeof(double)*n);
			star=fftw_malloc(sizeof(double)*n);
			p=fftw_plan_many_r2r(1, &zp, jdimx, yt, NULL, 1, zp, star, NULL, 1, zp, &type, FFTW_ESTIMATE);
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(anosa))) {kib=2; lcib=-1;}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(sws))) {kib=0; lcib=-1;}
			else {kib=0; lcib=0;}
			trc=g_array_index(GTK_PLOT(plot1)->stride, gint, 0)-1;
			list=group2;
			while (list)
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(list->data))) break;
				{list=(list->next); trc--;}
			}
			vis=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx*kdimxf*mx);
			doms=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx*kdimxf*mx);
			chp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx*kdimxf*mx);
			strat2=g_strsplit_set(strary2[0], "\t", 0);
			xi=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
			xf=xi;
			flags&=(PROC_OPN|PROC_TRS|PROC_PRS|PROC_BAT|PROC_POL|PROC_COM|PROC_RI);
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(wll)))
			{
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(chi)))
				{
					flags|=PROC_CHP;
					x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 3*mx*jdimx*kdimxf);
					y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 3*mx*jdimx*kdimxf);
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(twopionx)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
								{
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssw-c */
									{
										for (m=0; m<mx; m++)
										{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
										}
									}
									else /* +TdBssw-c */
									{
										for (m=0; m<mx; m++)
										{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
												for (j=0; j<jdimx; j++)
												{
												}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssw-c */
								{
									ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
									for (m=0; m<mx; m++)
									{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
								else /* +RdBssw-c */
								{
									ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
									for (m=0; m<mx; m++)
									{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssw-c +Tlssw-c */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* -Rlssw-c +Rlssw-c */
							{
								ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0w-c */
								{
									for (m=0; m<mx; m++)
									{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
								else /* +TdB0w-c */
								{
									for (m=0; m<mx; m++)
									{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0w-c */
							{
								ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +RdB0w-c */
							{
								ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0w-c +Tl0w-c */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* -Rl0w-c +Rl0w-c */
						{
							ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssw+c */
								{
									for (m=0; m<mx; m++)
									{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
											for (j=0; j<jdimx; j++)
											{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp)
{
	sp=zp;
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
else if (sp<16) {ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1);}
else
{
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=ofs+(k*(ofe-ofs)/(sp-1))-g_array_index(yp, gdouble, k+sr);
	if (clc<NZE) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
								else /* +TdBssw+c */
								{
									for (m=0; m<mx; m++)
									{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
											for (j=0; j<jdimx; j++)
											{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp)
{
	sp=zp;
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
else if (sp<16) {ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1);}
else
{
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=g_array_index(yp, gdouble, k+sr)-ofs-(k*(ofe-ofs)/(sp-1));
	if (clc<NZE) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssw+c */
							{
								ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(ofs-g_array_index(yp, gdouble, k+sr));
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +RdBssw+c */
							{
								ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(g_array_index(yp, gdouble, k+sr)-ofs);
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssw+c +Tlssw+c */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp)
{
	sp=zp;
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
else if (sp<16) {ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1);}
else
{
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=ofs+(k*(ofe-ofs)/(sp-1));
	if (clc<DZE && clc>NZE) yt[k+(j*zp)]=-G_MAXDOUBLE;
	else
	{
		clc=1-(g_array_index(yp, gdouble, k+sr)/clc);
		if (clc>DZE) yt[k+(j*zp)]=log(clc);
		else yt[k+(j*zp)]=-G_MAXDOUBLE;
	}
}
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* -Rlssw+c +Rlssw+c */
						{
							ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=g_array_index(yp, gdouble, k+sr)/ofs;
	if (clc>DZE) yt[k+(j*zp)]=log(clc);
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0w+c */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp)
{
	sp=zp;
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
else if (sp<16) {ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1);}
else
{
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(ofs+(k*(ofe-ofs)/(sp-1))-g_array_index(yp, gdouble, k+sr)));
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +TdB0w+c */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp)
{
	sp=zp;
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
else if (sp<16) {ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1);}
else
{
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(g_array_index(yp, gdouble, k+sr)-ofs-(k*(ofe-ofs)/(sp-1))));
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0w+c */
						{
							ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(ofs-g_array_index(yp, gdouble, k+sr)));
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* +RdB0w+c */
						{
							ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(g_array_index(yp, gdouble, k+sr)-ofs));
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0w+c +Tl0w+c */
					{
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp)
{
	sp=zp;
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
else if (sp<16) {ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1);}
else
{
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=ofs+(k*(ofe-ofs)/(sp-1));
	if (clc<DZE && clc>NZE) yt[k+(j*zp)]=-G_MAXDOUBLE;
	else yt[k+(j*zp)]=1-(g_array_index(yp, gdouble, k+sr)/clc);
}
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
					else /* -Rl0w+c +Rl0w+c */
					{
						ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=g_array_index(yp, gdouble, k+sr)/ofs;;
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(twopionx)))
				{
					x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
								else /* +TdBssw- */
								{
									for (m=0; m<mx; m++)
									{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssw- */
							{
								ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +RdBssw- */
							{
								ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssw- +Tlssw- */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* -Rlssw- +Rlssw- */
						{
							ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +TdB0w- */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0w- */
						{
							ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* +RdB0w- */
						{
							ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0w- +Tl0w- */
					{
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
					else /* -Rl0w- +Rl0w- */
					{
						ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
				{
					x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssw+ */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp)
{
	sp=zp;
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
else if (sp<16) {ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1);}
else
{
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=ofs+(k*(ofe-ofs)/(sp-1))-g_array_index(yp, gdouble, k+sr);
	if (clc<NZE) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +TdBssw+ */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp)
{
	sp=zp;
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
else if (sp<16) {ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1);}
else
{
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
											for (k=0; k<sp; k++)
											{
for (k=0; k<sp; k++)
{
	clc=g_array_index(yp, gdouble, k+sr)-ofs-(k*(ofe-ofs)/(sp-1));
	if (clc<NZE) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
											}
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssw+ */
						{
							ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(ofs-g_array_index(yp, gdouble, k+sr));
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* +RdBssw+ */
						{
							ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(g_array_index(yp, gdouble, k+sr)-ofs);
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssw+ +Tlssw+ */
					{
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp)
{
	sp=zp;
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
else if (sp<16) {ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1);}
else
{
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=ofs+(k*(ofe-ofs)/(sp-1));
	if (clc<DZE && clc>NZE) yt[k+(j*zp)]=-G_MAXDOUBLE;
	else
	{
		clc=1-(g_array_index(yp, gdouble, k+sr)/clc);
		if (clc>DZE) yt[k+(j*zp)]=log(clc);
		else yt[k+(j*zp)]=-G_MAXDOUBLE;
	}
}
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
					else /* -Rlssw+ +Rlssw+ */
					{
						ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=g_array_index(yp, gdouble, k+sr)/ofs;
	if (clc>DZE) yt[k+(j*zp)]=log(clc);
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
				{
					x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0w+ */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp)
{
	sp=zp;
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
else if (sp<16) {ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1);}
else
{
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(ofs+(k*(ofe-ofs)/(sp-1))-g_array_index(yp, gdouble, k+sr)));
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* +TdB0w+ */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp)
{
	sp=zp;
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
else if (sp<16) {ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1);}
else
{
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(g_array_index(yp, gdouble, k+sr)-ofs-(k*(ofe-ofs)/(sp-1))));
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0w+ */
					{
						ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(ofs-g_array_index(yp, gdouble, k+sr)));
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
					else /* +RdB0w+ */
					{
						ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(g_array_index(yp, gdouble, k+sr)-ofs));
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0w+ +Tl0w+ */
				{
					x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					for (m=0; m<mx; m++)
					{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp)
{
	sp=zp;
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
else if (sp<16) {ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1);}
else
{
	{ofs=g_array_index(yp, gdouble, sr); ofe=g_array_index(yp, gdouble, sp+sr-1); k=1;}
	while (k<8) {ofs+=g_array_index(yp, gdouble, sr+k); ofe+=g_array_index(yp, gdouble, sp+sr-(++k));}
	{ofs/=8; ofe/=8;}
}
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=ofs+(k*(ofe-ofs)/(sp-1));
	if (clc<DZE && clc>NZE) yt[k+(j*zp)]=-G_MAXDOUBLE;
	else yt[k+(j*zp)]=1-(g_array_index(yp, gdouble, k+sr)/clc);
}
							}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
					}
				}
				else /* -Rl0w+ +Rl0w+ */
				{
					x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
					for (m=0; m<mx; m++)
					{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=g_array_index(yp, gdouble, k+sr)/ofs;
							}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(oft)))
			{
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst))-oe;
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(chi)))
				{
					x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 3*mx*jdimx*kdimxf);
					y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 3*mx*jdimx*kdimxf);
					flags|=(PROC_OFT|PROC_CHP);
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(twopionx)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
								{
									if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssh-c */
									{
										for (m=0; m<mx; m++)
										{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
												for (j=0; j<jdimx; j++)
												{
												}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
										}
									}
									else /* +TdBssh-c */
									{
										for (m=0; m<mx; m++)
										{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
												for (j=0; j<jdimx; j++)
												{
												}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
										}
									}
								}
								else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBss-c */
								{
									for (m=0; m<mx; m++)
									{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
											for (j=0; j<jdimx; j++)
											{
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
								else /* +RdBssh-c */
								{
									for (m=0; m<mx; m++)
									{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
											for (j=0; j<jdimx; j++)
											{
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssh-c +Tlssh-c */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* -Rlssh-c +Rlssh-c */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0-c */
								{
									for (m=0; m<mx; m++)
									{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
											for (j=0; j<jdimx; j++)
											{
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
								else /* +TdB0h-c */
								{
									for (m=0; m<mx; m++)
									{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
											for (j=0; j<jdimx; j++)
											{
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0h-c */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +RdB0h-c */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0h-c +Tl0h-c */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* -Rl0h-c +Rl0h-c */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
							{
								if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssh+c */
								{
									for (m=0; m<mx; m++)
									{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
											for (j=0; j<jdimx; j++)
											{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=ofs+ce-g_array_index(yp, gdouble, k+sr);
	if (clc<0) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
								else /* +TdBssh+c */
								{
									for (m=0; m<mx; m++)
									{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
											for (j=0; j<jdimx; j++)
											{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
	{
	clc=g_array_index(yp, gdouble, k+sr)-ofs-ce;
	if (clc<0) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssh+c */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
										for (j=0; j<jdimx; j++)
										{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(ofs+ce-g_array_index(yp, gdouble, k+sr));
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +RdBssh+c */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
										for (j=0; j<jdimx; j++)
										{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(g_array_index(yp, gdouble, k+sr)-ofs-ce);
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssh+c +Tlssh+c */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=1-(g_array_index(yp, gdouble, k+sr)/(ofs+ce));
	if (clc>0) yt[k+(j*zp)]=log(clc);
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* -Rlssh+c +Rlssh+c */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=g_array_index(yp, gdouble, k+sr)/(ofs+ce);
	if (clc>0) yt[k+(j*zp)]=log(clc);
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0h+c */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
										for (j=0; j<jdimx; j++)
										{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(ofs+ce-g_array_index(yp, gdouble, k+sr)));
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +TdB0h+c */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
										for (j=0; j<jdimx; j++)
										{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(g_array_index(yp, gdouble, k+sr)-ofs-ce));
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0h+c */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(ofs+ce-g_array_index(yp, gdouble, k+sr)));
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* +RdB0h+c */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(g_array_index(yp, gdouble, k+sr)-ofs-ce));
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0h+c +Tl0h+c */
					{
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=1-(g_array_index(yp, gdouble, k+sr)/(ofs+ce));
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
					else /* -Rl0h+c +Rl0h+c */
					{
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=g_array_index(yp, gdouble, k+sr)/(ofs+ce);
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(twopionx)))
				{
					x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					flags|=PROC_OFT;
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
											for (j=0; j<jdimx; j++)
											{
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
								else /* +TdBssh- */
								{
									for (m=0; m<mx; m++)
									{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
											for (j=0; j<jdimx; j++)
											{
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssh- */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +RdBssh- */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssh- +Tlssh- */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* -Rlssh- +Rlssh- */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +TdB0h- */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0h- */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* +RdB0h- */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0h- +Tl0h- */
					{
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
								for (j=0; j<jdimx; j++)
								{
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
					else /* -Rl0h- +Rl0h- */
					{
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
								for (j=0; j<jdimx; j++)
								{
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
				{
					x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					flags|=PROC_OFT;
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
						{
							if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBssh+ */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
										for (j=0; j<jdimx; j++)
										{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=ofs+ce-g_array_index(yp, gdouble, k+sr);
	if (clc<0) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +TdBssh+ */
							{
								for (m=0; m<mx; m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
										for (j=0; j<jdimx; j++)
										{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
	{
	clc=g_array_index(yp, gdouble, k+sr)-ofs-ce;
	if (clc<0) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBssh+ */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(ofs+ce-g_array_index(yp, gdouble, k+sr));
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* +RdBssh+ */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(g_array_index(yp, gdouble, k+sr)-ofs-ce);
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlssh+ +Tlssh+ */
					{
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=1-(g_array_index(yp, gdouble, k+sr)/(ofs+ce));
	if (clc>0) yt[k+(j*zp)]=log(clc);
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
					else /* -Rlssh+ +Rlssh+ */
					{
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=g_array_index(yp, gdouble, k+sr)/(ofs+ce);
	if (clc>0) yt[k+(j*zp)]=log(clc);
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
				{
					x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					flags|=PROC_OFT;
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0h+ */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(ofs+ce-g_array_index(yp, gdouble, k+sr)));
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* +TdB0h+ */
						{
							for (m=0; m<mx; m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(g_array_index(yp, gdouble, k+sr)-ofs-ce));
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0h+ */
					{
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(ofs+ce-g_array_index(yp, gdouble, k+sr)));
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
					else /* +RdB0h+ */
					{
						for (m=0; m<mx; m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(g_array_index(yp, gdouble, k+sr)-ofs-ce));
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0h+ +Tl0h+ */
				{
					x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					flags|=PROC_OFT;
					for (m=0; m<mx; m++)
					{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=1-(g_array_index(yp, gdouble, k+sr)/(ofs+ce));
							}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
					}
				}
				else /* -Rl0h+ +Rl0h+ */
				{
					x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
					flags|=PROC_OFT;
					for (m=0; m<mx; m++)
					{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
ce=0;
for (j=1; j<=16; j++) ce+=g_array_index(yp, gdouble, lc-j);
ce/=16;
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=g_array_index(yp, gdouble, k+sr)/(ofs+ce);
							}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
					}
				}
				{oe=ce; ofs+=ce;}
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(fst), ofs);
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(chi)))
			{
				x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 3*mx*jdimx*kdimxf);
				y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 3*mx*jdimx*kdimxf);
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
				flags|=PROC_CHP;
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
								else /* +TdBsso-c */
								{
									for (m=0; m<g_strv_length(strary2); m++)
									{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
											for (j=0; j<jdimx; j++)
											{
											}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
									}
								}
							}
							else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBsso-c */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +RdBsso-c */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlsso-c +Tlsso-c */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* -Rlsso-c +Rlsso-c */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +TdB0o-c */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0o-c */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* +RdB0o-c */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0o-c +Tl0o-c */
					{
						for (m=0; m<g_strv_length(strary2); m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
					else /* -Rl0o-c +Rl0o-c */
					{
						for (m=0; m<g_strv_length(strary2); m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=ofs-g_array_index(yp, gdouble, k+sr);
	if (clc<0) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +TdBsso+c */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=g_array_index(yp, gdouble, k+sr)-ofs;
	if (clc<NZE) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBsso+c */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(ofs-g_array_index(yp, gdouble, k+sr));
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* +RdBsso+c */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(g_array_index(yp, gdouble, k+sr)-ofs);
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=1-(g_array_index(yp, gdouble, k+sr)/ofs);
	if (clc>DZE) yt[k+(j*zp)]=log(clc);
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=g_array_index(yp, gdouble, k+sr)/ofs;
	if (clc>DZE) yt[k+(j*zp)]=log(clc);
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(ofs-g_array_index(yp, gdouble, k+sr)));
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* +TdB0o+c */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);

									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0o+c */
					{
						for (m=0; m<g_strv_length(strary2); m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(ofs-g_array_index(yp, gdouble, k+sr)));
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
					else /* +RdB0o+c */
					{
						for (m=0; m<g_strv_length(strary2); m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(g_array_index(yp, gdouble, k+sr)-ofs));
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=1-(g_array_index(yp, gdouble, k+sr)/ofs);
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=g_array_index(yp, gdouble, k+sr)/ofs;
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(twopionx)))
			{
				x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
				y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
							else /* +TdBsso- */
							{
								for (m=0; m<g_strv_length(strary2); m++)
								{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
										for (j=0; j<jdimx; j++)
										{
										}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
								}
							}
						}
						else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBsso- */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* +RdBsso- */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlsso- +Tlsso- */
					{
						for (m=0; m<g_strv_length(strary2); m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
					else /* -Rlsso- +Rlsso- */
					{
						for (m=0; m<g_strv_length(strary2); m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* +TdB0o- */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0o- */
					{
						for (m=0; m<g_strv_length(strary2); m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
					else /* +RdB0o- */
					{
						for (m=0; m<g_strv_length(strary2); m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0o- +Tl0o- */
				{
					for (m=0; m<g_strv_length(strary2); m++)
					{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
							for (j=0; j<jdimx; j++)
							{
							}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
					}
				}
				else /* -Rl0o- +Rl0o- */
				{
					for (m=0; m<g_strv_length(strary2); m++)
					{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
							for (j=0; j<jdimx; j++)
							{
							}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lcmp)))
			{
				x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
				y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
					{
						if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdBsso+ */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=ofs-g_array_index(yp, gdouble, k+sr);
	if (clc<0) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
						else /* +TdBsso+ */
						{
							for (m=0; m<g_strv_length(strary2); m++)
							{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
									for (j=0; j<jdimx; j++)
									{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=g_array_index(yp, gdouble, k+sr)-ofs;
	if (clc<NZE) yt[k+(j*zp)]=log(1-exp(LNTOT*clc));
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
									}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
							}
						}
					}
					else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdBsso+ */
					{
						for (m=0; m<g_strv_length(strary2); m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(ofs-g_array_index(yp, gdouble, k+sr));
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
					else /* +RdBsso+ */
					{
						for (m=0; m<g_strv_length(strary2); m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=0.1*(g_array_index(yp, gdouble, k+sr)-ofs);
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=1-(g_array_index(yp, gdouble, k+sr)/ofs);
	if (clc>DZE) yt[k+(j*zp)]=log(clc);
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++)
{
	clc=g_array_index(yp, gdouble, k+sr)/ofs;
	if (clc>DZE) yt[k+(j*zp)]=log(clc);
	else yt[k+(j*zp)]=-G_MAXDOUBLE;
}
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(dBs)))
			{
				x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
				y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
				if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans)))
				{
					if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -TdB0o+ */
					{
						for (m=0; m<g_strv_length(strary2); m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=1-exp(LNTOT*(ofs-g_array_index(yp, gdouble, k+sr)));
								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
					else /* +TdB0o+ */
					{
						for (m=0; m<g_strv_length(strary2); m++)
						{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
								for (j=0; j<jdimx; j++)
								{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);

								}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
						}
					}
				}
				else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(neg))) /* -RdB0o+ */
				{
					for (m=0; m<g_strv_length(strary2); m++)
					{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(ofs-g_array_index(yp, gdouble, k+sr)));
							}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
					}
				}
				else /* +RdB0o+ */
				{
					for (m=0; m<g_strv_length(strary2); m++)
					{
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=exp(LNTOT*(g_array_index(yp, gdouble, k+sr)-ofs));
							}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
					}
				}
			}
			else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0o+ +Tl0o+ */
			{
				x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
				y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=1-(g_array_index(yp, gdouble, k+sr)/ofs);
							}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp/2)-1))&&(sp<(zp/2))&&((sp-sr)>0))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phio=-atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				{dst=0; pn=0;}
				for (l=sr+1; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (gchar*) Err);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
					}
				}
			}
			else /* -Rl0o+ +Rl0o+ */
			{
				x=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
				y=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 2*mx*jdimx*kdimxf);
				ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
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
strat2=g_strsplit_set(strary2[m], "\t", 0);
if (g_file_get_contents(strat2[1], &contents, NULL, &Err))
{
	strary=g_strsplit_set(contents, "\r\n", 0);
	sal=g_strv_length(strary);
	{xp=g_array_new(FALSE, FALSE, sizeof(gdouble)); yp=g_array_new(FALSE, FALSE, sizeof(gdouble));}
	lc=lcib;
	for (k=kib; k<sal; k++)
	{
		if (!strary[k]) continue;
		g_strchug(strary[k]);
		if (!g_strcmp0("", strary[k])) continue;
		if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) continue;
		if ((lc++)<0) continue;
		strat=g_strsplit_set(strary[k], "\t", 0);
		lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
		g_array_append_val(xp, lcl);
		if (!strat[trc]) lcl=0;
		else lcl=g_ascii_strtod(g_strstrip(strat[trc]), NULL);
		g_array_append_val(yp, lcl);
		g_strfreev(strat);
	}
	g_strfreev(strary);
	lcl=g_ascii_strtod(g_strstrip(strat2[0]), NULL);
	if (lcl<xi) xi=lcl;
	else if (lcl>xf) xf=lcl;
	delp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), jdimx);
	for (j=0; j<n; j++) yt[j]=0;
							for (j=0; j<jdimx; j++)
							{
iv=g_array_index(bsra, gdouble, j);
k=0;
while ((k<lc)&&(iv>g_array_index(xp, gdouble, k))) k++;
sr=k;
iv=g_array_index(bspa, gdouble, j);
while ((k<lc)&&(iv>=g_array_index(xp, gdouble, k))) k++;
sp=k-sr;
if (sp>zp) sp=zp;
iv=(sp-1)/(zp*(g_array_index(xp, gdouble, sp+sr-1)-g_array_index(xp, gdouble, sr)));
g_array_append_val(delp, iv);
for (k=0; k<sp; k++) yt[k+(j*zp)]=g_array_index(yp, gdouble, k+sr)/ofs;
							}
	{g_array_free(xp, TRUE); g_array_free(yp, TRUE);}
	fftw_execute(p);
	starp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), n);
	for (j=0; j<n; j++) g_array_append_val(starp, star[j]);
	for (j=0; j<jdimx; j++)
	{
		vzt=g_array_index(starp, gdouble, j*zp);
		iv=g_array_index(delp, gdouble, j);
		if (iv<DZE) idelf=G_MAXDOUBLE;
		else idelf=1/iv;
		iv=g_array_index(zwa, gdouble, j)*idelf/2;
		for (l=1; l<iv; l++)
		{
			ivd=g_array_index(starp, gdouble, l+(j*zp));
			ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
			vzt+=sqrt((ivd*ivd)+(ivdt*ivdt));
		}
		if (vzt<DZE) vzt=G_MAXDOUBLE;
		else vzt=l/vzt;
		for (k=0; k<kdimxf; k++)
		{
			g_array_append_val(x, lcl);
			sr=(gint) ceil(g_array_index(isra, gdouble, j+(k*jdimx))*idelf);
			sp=(gint) floor(g_array_index(ispa, gdouble, j+(k*jdimx))*idelf);
			tcn=g_array_index(tca, gdouble, j+(k*jdimx))*idelf;
			twd=g_array_index(twa, gdouble, j+(k*jdimx))*idelf/2;
			if ((sr<((zp-1)/2))&&(sp<(zp/2))&&((sp-sr)>1))
			{
				vt=g_array_index(starp, gdouble, sr+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr);
				phia=atan2(ivdt, vt);
				vt=sqrt((vt*vt)+(ivdt*ivdt));
				ivd=g_array_index(starp, gdouble, sr+1+(j*zp));
				ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-sr-1);
				phio=-atan2(ivdt, ivd);
				phia+=phio;
				vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
				{pn=0; cn=0; dst=0; ddp=0;}
				for (l=sr+2; l<=sp; l++)
				{
					ivd=g_array_index(starp, gdouble, l+(j*zp));
					ivdt=g_array_index(starp, gdouble, ((j+1)*zp)-l);
					phi=atan2(ivdt, ivd);
					phio+=phi;
					vt+=sqrt((ivd*ivd)+(ivdt*ivdt));
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
				}
				pn*=NMY_2PI*g_array_index(delp, gdouble, j);
				if ((pn<DZE)&&(pn>NZE)) dst=G_MAXDOUBLE;
				else dst/=pn;
				cn*=NMY_2PI*g_array_index(delp, gdouble, j);
				cn*=G_PI*g_array_index(delp, gdouble, j);
				if ((cn<DZE)&&(cn>NZE)) ddp=G_MAXDOUBLE;
				else ddp=cn/ddp;
				vt*=vzt/(sp-sr+1);
			}
			else {vt=0; dst=0; ddp=0;}
			{g_array_append_val(vis, vt); g_array_append_val(doms, dst); g_array_append_val(chp, ddp);}
		}
	}
	{g_array_free(delp, TRUE); g_array_free(starp, TRUE);}
}
else
{
	str=g_strdup_printf(_("Loading failed for file: %s, Error: %s."), fin, (Err->message));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
	{g_free(str); g_error_free(Err);}
}
{g_free(contents); g_strfreev(strat2);}
					}
				}
			}
			{g_strfreev(strary2); fftw_destroy_plan(p); fftw_free(yt); fftw_free(star);}
			for (j=0; j<jdimx*kdimxf; j++)
			{
				iv=g_array_index(vis, gdouble, j);
				g_array_append_val(y, iv);
			}
			for (j=0; j<jdimx*kdimxf; j++)
			{
				iv=g_array_index(x, gdouble, j);
				g_array_append_val(x, iv);
				iv=g_array_index(doms, gdouble, j);
				g_array_append_val(y, iv);
			}
			if (flags&PROC_CHP)
			{
				for (j=0; j<jdimx*kdimxf; j++)
				{
					iv=g_array_index(x, gdouble, j);
					g_array_append_val(x, iv);
					iv=g_array_index(chp, gdouble, j);
					g_array_append_val(y, iv);
				}
			}
			dx=jdimx*kdimxf;
			if ((flagd&DISP_CHP)==0)
			{
				if ((flagd&DISP_DMS)==0) dx2=0;
				else dx2=mx*dx;
			}
			else dx2=2*mx*dx;
			if ((flagd&DISP_MRJ)==0)
			{
				if ((flagd&DISP_MRK)==0)/* single plot mode */
				{
					{nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1); st=g_array_sized_new(FALSE, FALSE, sizeof(gint), 1);}
					dx2+=kdim+(jdim*kdimxf);
					{g_array_append_val(sz, mx); g_array_append_val(st, dx); g_array_append_val(nx, dx2);}
					mny=g_array_index(y, gdouble, dx2);
					{mxy=mny; l=1;}
					while (l<mx)
					{
						xx=g_array_index(y, gdouble, dx2+((l++)*dx));
						if (xx>mxy) mxy=xx;
						else if (xx<mny) mny=xx;
					}
				}
				else/* multiplot over k */
				{
					{nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf); st=g_array_sized_new(FALSE, FALSE, sizeof(gint), kdimxf);}
					dx2+=jdim*kdimxf;
					{g_array_append_val(sz, mx); g_array_append_val(st, dx); g_array_append_val(nx, dx2);}
					mny=g_array_index(y, gdouble, dx2);
					{mxy=mny; l=1;}
					while (l<mx)
					{
						xx=g_array_index(y, gdouble, dx2+((l++)*dx));
						if (xx>mxy) mxy=xx;
						else if (xx<mny) mny=xx;
					}
					k=dx2+1;
					while (k<kdimxf+dx2)
					{
						{g_array_append_val(sz, mx); g_array_append_val(st, dx); g_array_append_val(nx, k);}
						l=0;
						while (l<mx)
						{
							xx=g_array_index(y, gdouble, k+((l++)*dx));
							if (xx>mxy) mxy=xx;
							else if (xx<mny) mny=xx;
						}
						k++;
					}
				}
			}
			else if ((flagd&DISP_MRK)==0)/* multiplot over j */
			{
				{nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx); st=g_array_sized_new(FALSE, FALSE, sizeof(gint), jdimx);}
				dx2+=kdim;
				{g_array_append_val(sz, mx); g_array_append_val(st, dx); g_array_append_val(nx, dx2);}
				mny=g_array_index(y, gdouble, dx2);
				{mxy=mny; l=1;}
				while (l<mx)
				{
					xx=g_array_index(y, gdouble, dx2+((l++)*dx));
					if (xx>mxy) mxy=xx;
					else if (xx<mny) mny=xx;
				}
				k=dx2+kdimxf;
				while (k<dx+dx2)
				{
					{g_array_append_val(sz, mx); g_array_append_val(st, dx); g_array_append_val(nx, k);}
					l=0;
					while (l<mx)
					{
						xx=g_array_index(y, gdouble, k+((l++)*dx));
						if (xx>mxy) mxy=xx;
						else if (xx<mny) mny=xx;
					}
					k+=kdimxf;
				}
			}
			else /* both indices */
			{
				{nx=g_array_sized_new(FALSE, FALSE, sizeof(gint), dx); sz=g_array_sized_new(FALSE, FALSE, sizeof(gint), dx); st=g_array_sized_new(FALSE, FALSE, sizeof(gint), dx);}
				{g_array_append_val(sz, mx); g_array_append_val(st, dx); g_array_append_val(nx, dx2);}
				mny=g_array_index(y, gdouble, dx2);
				{mxy=mny; l=1;}
				while (l<mx)
				{
					xx=g_array_index(y, gdouble, dx2+((l++)*dx));
					if (xx>mxy) mxy=xx;
					else if (xx<mny) mny=xx;
				}
				k=dx2+1;
				while (k<dx+dx2)
				{
					{g_array_append_val(sz, mx); g_array_append_val(st, dx); g_array_append_val(nx, k);}
					l=0;
					while (l<mx)
					{
						xx=g_array_index(y, gdouble, k+((l++)*dx));
						if (xx>mxy) mxy=xx;
						else if (xx<mny) mny=xx;
					}
					k++;
				}
			}
			dialog=gtk_dialog_new_with_buttons(_("Measurand Variable Type"), GTK_WINDOW(window), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, _("Linear"), 1, _("Polar\n(Degrees)"), 2, _("Polar\n(Radians)"), 3, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
			cont=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
			label=gtk_label_new(_("Select Measurand Type:"));
			gtk_container_add(GTK_CONTAINER(cont), label);
			switch (gtk_dialog_run(GTK_DIALOG(dialog)))
			{
				case 1:
				if (((flags&PROC_BAT)==0)||((flags&PROC_POL)!=0))
				{
					gtk_notebook_remove_page(GTK_NOTEBOOK(notebook2), 2);
					rest=gtk_table_new(1, 1, FALSE);
					gtk_widget_show(rest);
					plot3=gtk_plot_linear_new();
					g_signal_connect(plot3, "moved", G_CALLBACK(pltmv), NULL);
					gtk_widget_show(plot3);
					gtk_table_attach(GTK_TABLE(rest), plot3, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
					label=gtk_label_new(_("Analysis Results"));
					gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), rest, label);
					{flags|=(PROC_BAT|PROC_PRS|PROC_TRS|PROC_OPN); flags&=(PROC_OFT|PROC_CHP|PROC_BAT|PROC_PRS|PROC_TRS|PROC_OPN);}
				}
				plt=GTK_PLOT_LINEAR(plot3);
				if (mx<8) {(plt->flagd)=3; (plt->ptsize)=4;}
				else if (mx<20){(plt->flagd)=3; (plt->ptsize)=3;}
				else if (mx<50){(plt->flagd)=3; (plt->ptsize)=2;}
				gtk_plot_linear_set_data(plt, x, y, nx, sz, st);
				gtk_plot_linear_update_scale_pretty(plot3, xi, xf, mny, mxy);
				{g_array_unref(x); g_array_unref(y); g_array_unref(nx); g_array_unref(sz); g_array_unref(st);}
				gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
				gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 2);
				break;
				case 2:
				if (((flags&PROC_BAT)==0)||((flags&PROC_POL)==0))
				{
					gtk_notebook_remove_page(GTK_NOTEBOOK(notebook2), 2);
					rest=gtk_table_new(1, 1, FALSE);
					gtk_widget_show(rest);
					plot3=gtk_plot_polar_new();
					g_signal_connect(plot3, "moved", G_CALLBACK(pltmvp), NULL);
					gtk_widget_show(plot3);
					gtk_table_attach(GTK_TABLE(rest), plot3, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
					label=gtk_label_new(_("Analysis Results"));
					gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), rest, label);
					flags|=(PROC_POL|PROC_BAT|PROC_PRS|PROC_TRS|PROC_OPN);
				}
				plt2=GTK_PLOT_POLAR(plot3);
				if (mx<8) (plt2->ptsize)=4;
				else if (mx<20) (plt2->ptsize)=3;
				else if (mx<50) (plt2->ptsize)=2;
				else (plt2->flagd)=2;
				gtk_plot_polar_set_data(plt2, y, x, nx, sz, st);
				gtk_plot_polar_update_scale_pretty(plot3, mny, mxy, xi, xf);
				{g_array_unref(x); g_array_unref(y); g_array_unref(nx); g_array_unref(sz); g_array_unref(st);}
				gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
				gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 2);
				break;
				case 3:
				if (((flags&PROC_BAT)==0)||((flags&PROC_POL)==0))
				{
					gtk_notebook_remove_page(GTK_NOTEBOOK(notebook2), 2);
					rest=gtk_table_new(1, 1, FALSE);
					gtk_widget_show(rest);
					plot3=gtk_plot_polar_new();
					g_signal_connect(plot3, "moved", G_CALLBACK(pltmvp), NULL);
					gtk_widget_show(plot3);
					gtk_table_attach(GTK_TABLE(rest), plot3, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
					label=gtk_label_new(_("Analysis Results"));
					gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), rest, label);
					flags|=(PROC_POL|PROC_BAT|PROC_PRS|PROC_TRS|PROC_OPN);
				}
				plt2=GTK_PLOT_POLAR(plot3);
				if (mx<8) {(plt2->flagd)=7; (plt2->ptsize)=4;}
				else if (mx<20) {(plt2->flagd)=7; (plt2->ptsize)=3;}
				else if (mx<50) {(plt2->flagd)=7; (plt2->ptsize)=2;}
				else (plt2->flagd)=3;
				gtk_plot_polar_set_data(plt2, y, x, nx, sz, st);
				gtk_plot_polar_update_scale_pretty(plot3, mny, mxy, xi, xf);
				{g_array_unref(x); g_array_unref(y); g_array_unref(nx); g_array_unref(sz); g_array_unref(st);}
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
			{g_free(str); g_error_free(Err);}
		}
		{g_free(contents2); g_free(fin);}
	}
}
