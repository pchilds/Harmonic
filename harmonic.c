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
 * occasional crashing when reloading files [memory handling?]
 * HELP: build contents
 * ABOUT: build contents
 * PLOT: signal connect kindex for displaying plot3
 * PLOT: signal connect jindex for displaying plot3
 * PLOT: update trc in upg function
 * BATCH: read filename from config file
 * BATCH: config writer utility
 * FFT: implement invert to 2pi/x routine
 * PRC: differential phase routine
 * PRC: storage of results
 * PRC: chirp
 * SAVE: everything
 */

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <fftw3.h>
#include <math.h>
#include "plotlinear0-1-0.h"
/*#include "harmonicconf.h"*/

#define DZE 0.00001 /* divide by zero threshold */
#define MXD 16 /* maximum number of windows */
#define MXDS 256 /* the square of this */
#define MXDM 15 /* one less than MXD */
#define LNTOT 0.23025850929940456840179914546843642076011014886288 /* (ln10)/10 */

GtkWidget *svg, *tr, *zpd, *pr, *tracmenu, *trac, *fst, *notebook, *notebook2, *plot1, *plot2, *plot3, *statusbar;
GtkWidget *agosa, *agtl, *anosa, *sws, *dlm, *ncmp, *lcmp, *bat, *twopionx, *trac, *trans, *dBs, *neg;
GtkWidget *bsr, *bsp, *isr, *isp, *tc, *tw, *zw, *jind, *jind2, *kind; /* widgets for windowing */
GArray *bsra, *bspa, *isra, *ispa, *tca, *twa, *zwa, *x, *specs, *yb, *stars, *xsb, *ysb, *delx; /* arrays for windowing and data */
GtkAdjustment *adj;
GSList *group2=NULL;
gint lc;
guint jdim=0, kdim=0, jdimx=0, kdimx=0, satl=0, trc=1, jdimxf=0;
gchar *contents;
gulong pr_id, sv_id;
GError *Err;

void help(GtkWidget *widget, gpointer data)
  {
  GtkWidget *helpwin;

  helpwin=gtk_dialog_new_with_buttons("Instructions", GTK_WINDOW(widget), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
  g_signal_connect_swapped(helpwin, "destroy", G_CALLBACK(gtk_widget_destroy), helpwin);
  gtk_widget_show(helpwin);
  if (gtk_dialog_run(GTK_DIALOG(helpwin))==GTK_RESPONSE_CLOSE) gtk_widget_destroy(helpwin);
  }

void about(GtkWidget *widget, gpointer data)
  {
  GtkWidget *helpwin;

  helpwin=gtk_about_dialog_new();
  g_signal_connect_swapped(helpwin, "destroy", G_CALLBACK(gtk_widget_destroy), helpwin);
  gtk_widget_show(helpwin);
  }

void sav(GtkWidget *widget, gpointer data)
  {
  GtkWidget *wfile;
  gchar *fout=NULL;

  wfile=gtk_file_chooser_dialog_new("Select Data File", GTK_WINDOW(widget), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
  g_signal_connect(wfile, "destroy", G_CALLBACK(gtk_widget_destroy), wfile);
  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(wfile), TRUE);
  if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
    {
    fout=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (wfile));
    g_file_set_contents(fout, contents, -1, &Err);
    }
  gtk_widget_destroy(wfile);
  }

void prs(GtkWidget *widget, gpointer data)
  {
  PlotLinear *plt2;
  gint j, k, l, st, sp;
  gdouble ml, iv, vzt, vt, ivd, ivdt;

  plt2=PLOT_LINEAR(plot2);
  for (j=0; j<=jdimxf; j++)
    {
    ml=(g_array_index(delx, gdouble, j))*(plt2->size)/((2*(plt2->size))-1);
    iv=g_array_index(zwa, gdouble, j)*ml;
    vzt=0;
    for (l=0; l<iv; l++)
      {
      ivd=g_array_index(stars, gdouble, l+(2*j*(plt2->size)));
      ivd*=ivd;
      ivdt=g_array_index(stars, gdouble, (2*(j+1)*(plt2->size))-l);
      ivdt*=ivdt;
      ivd+=ivdt;
      ivd=sqrt(ivd);
      vzt+=ivd;
      }
    vzt=1/vzt;
    for (k=0; k<kdimx; k++)
      {
      st=ceil((g_array_index(isra, gdouble, j+(k*MXD)))*ml);
      if (st>(plt2->size)) st=(plt2->size);
      sp=floor((g_array_index(ispa, gdouble, j))*ml);
      if (sp>(plt2->size)) sp=(plt2->size);
      vt=0;
      for (l=st; l<sp; l++)
        {
        ivd=g_array_index(stars, gdouble, l+(2*j*(plt2->size)));
        ivdt=g_array_index(stars, gdouble, (2*(j+1)*(plt2->size))-l);
        /* differential phase routine */
        ivd*=ivd;
        ivdt*=ivdt;
        ivd+=ivdt;
        ivd=sqrt(ivd);
        vt+=ivd;
        }
      vt*=vzt;
      }
    }
  }
  
void trs(GtkWidget *widget, gpointer data) /* need to incorporate case for inversion to 2pi/x */
  {
  PlotLinear *plt2;
  guint j, k, st, sp;
  gint n, zp;
  gdouble iv, clc, ofs, xx, yx;
  gchar *str;
  double *y, *star;
  fftw_plan p;
  fftw_r2r_kind type=FFTW_R2HC;

  ofs=gtk_spin_button_get_value(GTK_SPIN_BUTTON(fst));
  zp=1<<(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(zpd)));
  n=zp*(jdimx+1);
  g_array_free(stars, TRUE);
  g_array_free(xsb, TRUE);
  g_array_free(ysb, TRUE);
  g_array_free(delx, TRUE);
  delx=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (jdimx+1));
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
                str=g_strdup("Some clipping occured. Increase zero padding.");
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
                str=g_strdup("Some clipping occured. Increase zero padding.");
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
              str=g_strdup("Some clipping occured. Increase zero padding.");
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
              str=g_strdup("Some clipping occured. Increase zero padding.");
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
            str=g_strdup("Some clipping occured. Increase zero padding.");
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
            str=g_strdup("Some clipping occured. Increase zero padding.");
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
            str=g_strdup("Some clipping occured. Increase zero padding.");
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
              str=g_strdup("Some clipping occured. Increase zero padding.");
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
              str=g_strdup("Some clipping occured. Increase zero padding.");
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
            str=g_strdup("Some clipping occured. Increase zero padding.");
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
            str=g_strdup("Some clipping occured. Increase zero padding.");
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
          str=g_strdup("Some clipping occured. Increase zero padding.");
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
          str=g_strdup("Some clipping occured. Increase zero padding.");
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
          str=g_strdup("Some clipping occured. Increase zero padding.");
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
              str=g_strdup("Some clipping occured. Increase zero padding.");
              gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
              g_free(str);
              sp=zp;
              }
            iv=g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st);
            g_array_append_val(delx, iv);
            for (k=0; k<sp; k++)
              {
              clc=ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl));
              if (clc<0)
                {
                clc=clc*LNTOT;
                clc=exp(clc);
                clc=-clc;
                clc++;
                y[k+(j*zp)]=log(clc);
                }
              else
                {
                y[k+(j*zp)]=G_MININT;
                }
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
              str=g_strdup("Some clipping occured. Increase zero padding.");
              gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
              g_free(str);
              sp=zp;
              }
            iv=g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st);
            g_array_append_val(delx, iv);
            for (k=0; k<sp; k++)
              {
              clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs;
              if (clc<0)
                {
                clc=clc*LNTOT;
                clc=exp(clc);
                clc=-clc;
                clc++;
                y[k+(j*zp)]=log(clc);
                }
              else
                {
                y[k+(j*zp)]=G_MININT;
                }
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
            str=g_strdup("Some clipping occured. Increase zero padding.");
            gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
            g_free(str);
            sp=zp;
            }
          iv=g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st);
          g_array_append_val(delx, iv);
          for (k=0; k<sp; k++)
            {
            clc=ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl));
            y[k+(j*zp)]=clc*0.1;
            }
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
            str=g_strdup("Some clipping occured. Increase zero padding.");
            gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
            g_free(str);
            sp=zp;
            }
          iv=g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st);
          g_array_append_val(delx, iv);
          for (k=0; k<sp; k++)
            {
            clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs;
            y[k+(j*zp)]=clc*0.1;
            }
          }
        }
      }
    else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tlss+ +Tlss+ */
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
          str=g_strdup("Some clipping occured. Increase zero padding.");
          gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
          g_free(str);
          sp=zp;
          }
        iv=g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st);
        g_array_append_val(delx, iv);
        for (k=0; k<sp; k++)
          {
          clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
          clc=-clc;
          clc++;
          if (clc>0)
            {
            y[k+(j*zp)]=log(clc);
            }
          else
            {
            y[k+(j*zp)]=G_MININT;
            }
          }
        }
      }
    else /* -Rlss+ +Rlss+ */
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
          str=g_strdup("Some clipping occured. Increase zero padding.");
          gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
          g_free(str);
          sp=zp;
          }
        iv=g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st);
        g_array_append_val(delx, iv);
        for (k=0; k<sp; k++)
          {
          clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
          if (clc>0)
            {
            y[k+(j*zp)]=log(clc);
            }
          else
            {
            y[k+(j*zp)]=G_MININT;
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
            str=g_strdup("Some clipping occured. Increase zero padding.");
            gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
            g_free(str);
            sp=zp;
            }
          iv=g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st);
          g_array_append_val(delx, iv);
          for (k=0; k<sp; k++)
            {
            clc=ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl));
            clc=clc*LNTOT;
            clc=exp(clc);
            clc=-clc;
            y[k+(j*zp)]=clc++;
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
            str=g_strdup("Some clipping occured. Increase zero padding.");
            gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
            g_free(str);
            sp=zp;
            }
          iv=g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st);
          g_array_append_val(delx, iv);
          for (k=0; k<sp; k++)
            {
            clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs;
            clc=clc*LNTOT;
            clc=exp(clc);
            clc=-clc;
            y[k+(j*zp)]=clc++;
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
          str=g_strdup("Some clipping occured. Increase zero padding.");
          gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
          g_free(str);
          sp=zp;
          }
        iv=g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st);
        g_array_append_val(delx, iv);
        for (k=0; k<sp; k++)
          {
          clc=ofs-g_array_index(specs, gdouble, trc-1+((k+st)*satl));
          clc=clc*LNTOT;
          y[k+(j*zp)]=exp(clc);
          }
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
          str=g_strdup("Some clipping occured. Increase zero padding.");
          gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
          g_free(str);
          sp=zp;
          }
        iv=g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st);
        g_array_append_val(delx, iv);
        for (k=0; k<sp; k++)
          {
          clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))-ofs;
          clc=clc*LNTOT;
          y[k+(j*zp)]=exp(clc);
          }
        }
      }
    }
  else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(trans))) /* -Tl0+ and +Tl0+ */ 
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
        str=g_strdup("Some clipping occured. Increase zero padding.");
        gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
        g_free(str);
        sp=zp;
        }
      iv=g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st);
      g_array_append_val(delx, iv);
      for (k=0; k<sp; k++)
        {
        clc=g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
        clc=-clc;
        y[k+(j*zp)]=clc++;
        }
      }
    }
  else /* -Rl0+ and +Rl0+ */ 
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
        str=g_strdup("Some clipping occured. Increase zero padding.");
        gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
        g_free(str);
        sp=zp;
        }
      iv=g_array_index(x, gdouble, sp+st-1)-g_array_index(x, gdouble, st);
      g_array_append_val(delx, iv);
      for (k=0; k<sp; k++) y[k+(j*zp)]=g_array_index(specs, gdouble, trc-1+((k+st)*satl))/ofs;
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
  for (j=0; j<zp/2; j++)
    {
    xx=j*(zp-1)/(zp*g_array_index(delx, gdouble, 0));
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
  fftw_free(star);
  plt2=PLOT_LINEAR(plot2);
  plt2->size=zp/2;
  plt2->xdata=xsb;
  plt2->ydata=ysb;
  plot_linear_update_scale_pretty(plot2, 0, xx, 0, yx);
  gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
  gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 1);
  adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, jdimx, 1.0, 5.0, 0.0);
  gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(jind), adj);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind), 0);
  gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(jind2), adj);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind2), 0);
  adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, (MXDM), 1.0, 5.0, 0.0);
  gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(kind), adj);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(kind), 0);
  jdimxf=jdimx;
  pr_id=g_signal_connect(G_OBJECT(pr), "activate", G_CALLBACK(prs), NULL);
  sv_id=g_signal_connect(G_OBJECT(svg), "activate", G_CALLBACK(sav), NULL);
  }

void upg(GtkWidget *widget, gpointer data)
  {
  PlotLinear *plt;
  gdouble dt, xi, xf, mny, mxy;
  gint j;

  /* determine which trace button was clicked and update trc */
  plt=PLOT_LINEAR(plot1);
  g_array_free(yb, TRUE);
  yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
  dt=g_array_index(specs, gdouble, trc-1);
  mny=dt;
  mxy=dt;
  g_array_append_val(yb, dt);
  for (j=1; j<(plt->size); j++)
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

void opd(GtkWidget *widget, gpointer data)
  {
  PlotLinear *plt;
  GtkWidget *wfile, *trace;
  gdouble xi, xf, lcl, mny, mxy;
  guint j, k, l, sal;
  gchar s[5];
  gchar *str, *fin=NULL;
  gchar **strary, **strat;

  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bat)))
    {
    if (!g_signal_handler_is_connected(G_OBJECT(pr), pr_id))
      {
      str=g_strdup("Perform an initial test of the parameters first");
      gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
      g_free(str);
      }
    else /* batch job processing mode */
      {
      wfile=gtk_file_chooser_dialog_new("Select Config File", GTK_WINDOW(widget), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
      g_signal_connect(wfile, "destroy", G_CALLBACK(gtk_widget_destroy), wfile);
      if (gtk_dialog_run(GTK_DIALOG(wfile))==GTK_RESPONSE_ACCEPT)
        {
        fin=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(wfile));
        gtk_widget_destroy (wfile);
        if (g_file_get_contents(fin, &contents, NULL, &Err))
          {
          str=g_strdup_printf("File: %s successfully loaded", fin);
          gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
          g_free(str);
          gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
          gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 2);
          for (j=0; j<1; j++) /* add read from config file and update j */
            {
            strary=g_strsplit_set(contents, "\r\n", 0);
            sal=g_strv_length(strary);
            g_array_free(x, TRUE);
            g_array_free(yb, TRUE);
            g_array_free(specs, TRUE);
            x=g_array_new(FALSE, FALSE, sizeof(gdouble));
            yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
            specs=g_array_new(FALSE, FALSE, sizeof(gdouble));
            if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(anosa))) {k=2; lc=-1;}
            else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(sws))) {k=0; lc=-1;}
            else {k=0; lc=0;}
            while (k<sal)
              {
              if (!strary[k]) {k++; continue;}
              if (!g_strcmp0("", strary[k])) {k++; continue;}
              if (!(g_ascii_isdigit(strary[k][0])|(g_str_has_prefix(strary[k],"-")))) {k++; continue;}
              if (lc<0) {lc++; k++; continue;}
              strat=g_strsplit_set(strary[k], "\t,", 0);
              lcl=g_ascii_strtod(g_strstrip(strat[0]), NULL);
              g_array_append_val(x, lcl);
              if (lc==0) satl=g_strv_length(strat);
              if (!strat[1]) lcl=0;
              else lcl=g_ascii_strtod(g_strstrip(strat[1]), NULL);
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
            trs(tr, NULL);
            prs(pr, NULL);
            }
          }
        else
          {
          str=g_strdup_printf("Loading failed for file: %s", fin);
          gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
          g_free(str);
          }
        }
      else
        {
        gtk_widget_destroy (wfile);
        }
      }
    }
  else /* single file mode */
    {
    wfile=gtk_file_chooser_dialog_new("Select Data File", GTK_WINDOW(widget), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
    g_signal_connect(wfile, "destroy", G_CALLBACK(gtk_widget_destroy), wfile);
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
        x=g_array_new(FALSE, FALSE, sizeof(gdouble));
        yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
        specs=g_array_new(FALSE, FALSE, sizeof(gdouble));
        if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(anosa))) {k=2; lc=-1;}
        else if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(sws))) {k=0; lc=-1;}
        else {k=0; lc=0;}
        while (k<sal)
          {
          if (!strary[k]) {k++; continue;}
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
        g_slist_free(group2);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(trac), NULL);
        gtk_widget_destroy(tracmenu);
        tracmenu=gtk_menu_new();
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(trac), tracmenu);
        group2=NULL;
        trace=gtk_radio_menu_item_new_with_label(group2, "1");
        group2=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(trace));
        gtk_menu_shell_append(GTK_MENU_SHELL(tracmenu), trace);
        gtk_widget_show(trace);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(trace), TRUE);
        for (j=2; j<=satl; j++)
          {
          g_snprintf(s, 4, "%d", j);
          trace=gtk_radio_menu_item_new_with_label(group2, s);
          g_signal_connect(G_OBJECT(trace), "clicked", G_CALLBACK(upg), NULL);
          group2=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(trace));
          gtk_menu_shell_append(GTK_MENU_SHELL(tracmenu), trace);
          gtk_widget_show(trace);
          }
        gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);
        gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook2), 0);
        g_signal_connect(G_OBJECT(tr), "clicked", G_CALLBACK(trs), NULL);
        if (g_signal_handler_is_connected(G_OBJECT(pr), pr_id))
          {
          g_signal_handler_disconnect(G_OBJECT(pr), pr_id);
          g_signal_handler_disconnect(G_OBJECT(svg), sv_id);
          }
        str=g_strdup_printf("File: %s successfully loaded", fin);
        gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
        g_free(str);
        plt=PLOT_LINEAR(plot1);
        plt->size=lc;
        plt->xdata=x;
        plt->ydata=yb;
        xi=g_array_index(x, gdouble, 0);
        xf=g_array_index(x, gdouble, (lc-1));
        plot_linear_update_scale_pretty(plot1, xi, xf, mny, mxy);
        }
      else
        {
        str=g_strdup_printf("Loading failed for file: %s", fin);
        gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
        g_free(str);
        }
      }
    gtk_widget_destroy (wfile);
    }
  }

void upj(GtkWidget *widget, gpointer data)
  {
  /*
   * Checks if j index spinner has increased to a new value and fills parameter array values accordingly
   * updates front panel to display parameters for new j,k values
   * If transform has been performed, changes the graph in plot 2   
   */
  PlotLinear *plt2, *plt3;
  guint j, k;
  gdouble num, num2, num3, num4, num5, num6, num7;
  gdouble *ptr;

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
    for (j=(jdimx+1); j<=jdim; j++)
      {
      for (k=0; k<kdimx; k++)
        {
        ptr=&g_array_index(isra, gdouble, j+(k*MXD));
        *ptr=num;
        ptr=&g_array_index(ispa, gdouble, j+(k*MXD));
        *ptr=num2;
        ptr=&g_array_index(tca, gdouble, j+(k*MXD));
        *ptr=num3;
        ptr=&g_array_index(twa, gdouble, j+(k*MXD));
        *ptr=num4;
        }
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
    num=g_array_index(isra, gdouble, (jdim+(kdim*MXD)));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(isr), num);
    num=g_array_index(ispa, gdouble, (jdim+(kdim*MXD)));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(isp), num);
    num=g_array_index(tca, gdouble, (jdim+(kdim*MXD)));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(tc), num);
    num=g_array_index(twa, gdouble, (jdim+(kdim*MXD)));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(tw), num);
    num=g_array_index(zwa, gdouble, jdim);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(zw), num);
    num6=g_array_index(bsra, gdouble, jdim);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(bsr), num6);
    num6=g_array_index(bspa, gdouble, jdim);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(bsp), num6);
    }
  if (g_signal_handler_is_connected(G_OBJECT(pr), pr_id))
    {
    plt2=PLOT_LINEAR(plot2);
    g_array_free(xsb, TRUE);
    g_array_free(ysb, TRUE);
    xsb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (plt2->size));
    ysb=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), (plt2->size));
    for (j=0; j<(plt2->size); j++)
      {
      num=j*((2*(plt2->size))-1)/(2*(plt2->size)*g_array_index(delx, gdouble, jdim));
      g_array_append_val(xsb, num);
      }
    j=(2*(jdim)*(plt2->size));
    num2=fabs(g_array_index(stars, gdouble, j));
    g_array_append_val(ysb, num2);
    while (j<(((2*jdim)+1)*(plt2->size)))
      {
      num6=g_array_index(stars, gdouble, j);
      num6*=num6;
      num7=g_array_index(stars, gdouble, (2*(plt2->size))-j);
      num7*=num7;
      num6+=num7;
      num6=sqrt(num6);
      if (num2<num6) num2=num6;
      g_array_append_val(ysb, num6);
      j++;
      }
    plt2->xdata=xsb;
    plt2->ydata=ysb;
    plot_linear_update_scale_pretty(plot2, 0, num, 0, num2);
    }
  }

void upk(GtkWidget *widget, gpointer data)
  {
  /*
   * Checks if k index spinner has increased to a new value and fills parameter array values accordingly
   * updates front panel to display parameters for new j,k values
   */
  PlotLinear *plt3;
  guint j;
  gdouble num, num2, num3, num4;

  kdim=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
  if (kdim>kdimx)
    {
    num=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isr));
    num2=gtk_spin_button_get_value(GTK_SPIN_BUTTON(isp));
    num3=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tc));
    num4=gtk_spin_button_get_value(GTK_SPIN_BUTTON(tw));
    for (j=0; j<(MXD*(kdim-kdimx)); j++)
      {
      g_array_append_val(isra, num);
      g_array_append_val(ispa, num2);
      g_array_append_val(tca, num3);
      g_array_append_val(twa, num4);
      }
    kdimx = kdim;
    }
  else
    {
    num=g_array_index(isra, gint, (jdim+(kdim*MXD)));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(isr), num);
    num=g_array_index(ispa, gint, (jdim+(kdim*MXD)));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(isp), num);
    num=g_array_index(tca, gint, (jdim+(kdim*MXD)));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(tc), num);
    num=g_array_index(twa, gint, (jdim+(kdim*MXD)));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(tw), num);
    }
  }

void pltmv(PlotLinear *plot, gpointer data)
  {
  gchar *str;

  str=g_strdup_printf("x: %f, y: %f", plot->xps, plot->yps);
  gtk_statusbar_push(GTK_STATUSBAR(statusbar), gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), str), str);
  g_free(str);
  }

/* Functions for updating analysis parameters - COMPLETISH - combine into two int/double functions? coersion? */

void uptwa(GtkWidget *widget, gpointer data)
  {
  gdouble *ptr;

  ptr=&g_array_index(twa, gdouble, jdim+(kdim*MXD));
  *ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  }

void uptca(GtkWidget *widget, gpointer data)
  {
  gdouble *ptr;

  ptr=&g_array_index(tca, gdouble, jdim+(kdim*MXD));
  *ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  }

void upispa(GtkWidget *widget, gpointer data)
  {
  gdouble *ptr;

  ptr=&g_array_index(ispa, gdouble, jdim+(kdim*MXD));
  *ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  /* coerce isr */
  }

void upisra(GtkWidget *widget, gpointer data)
  {
  gdouble *ptr;

  ptr=&g_array_index(isra, gdouble, jdim+(kdim*MXD));
  *ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  /* coerce isp */
  }

void upzwa(GtkWidget *widget, gpointer data)
  {
  gdouble *ptr;

  ptr=&g_array_index(zwa, gdouble, jdim);
  *ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  }

void upbspa(GtkWidget *widget, gpointer data)
  {
  gdouble *ptr;

  ptr=&g_array_index(bspa, gdouble, jdim);
  *ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  /* coerce bsr */
  }

void upbsra(GtkWidget *widget, gpointer data)
  {
  gdouble *ptr;

  ptr=&g_array_index(bsra, gdouble, jdim);
  *ptr=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  /* coerce bsp */
  }

void reset(GtkWidget *widget, gpointer data)
  {
  gdouble num;

  num=g_array_index(bsra, gdouble, 0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(bsr), num);
  num=g_array_index(bspa, gdouble, 0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(bsp), num);
  g_array_free(bsra, TRUE);
  g_array_free(bspa, TRUE);
  g_array_free(zwa, TRUE);
  g_array_free(isra, TRUE);
  g_array_free(ispa, TRUE);
  g_array_free(tca, TRUE);
  g_array_free(twa, TRUE);
  jdim=0;
  jdimx=0;
  kdim=0;
  kdimx=0;
  num=0;
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind), 0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(kind), 0);
  bsra=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXD);
  g_array_append_val(bspa, num);
  bspa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXD);
  g_array_append_val(bsra, num);
  zwa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXD);
  isra=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXDS);
  ispa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXDS);
  tca=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXDS);
  twa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXDS);
  /* deal with graphs? potential bugs */
  }

void reset2(GtkWidget *widget, gpointer data)
  {
  gdouble num, num2, num3, num4, num5;
  guint j;

  num=g_array_index(isra, gdouble, 0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(isr), num);
  num2=g_array_index(ispa, gdouble, 0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(isp), num2);
  num3=g_array_index(tca, gdouble, 0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(tc), num3);
  num4=g_array_index(twa, gdouble, 0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(tw), num4);
  num5=g_array_index(zwa, gdouble, 0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(zw), num5);
  g_array_free(zwa, TRUE);
  g_array_free(isra, TRUE);
  g_array_free(ispa, TRUE);
  g_array_free(tca, TRUE);
  g_array_free(twa, TRUE);
  jdim=0;
  kdim=0;
  kdimx=0;
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(jind2), 0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(kind), 0);
  zwa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXD);
  isra=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXDS);
  ispa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXDS);
  tca=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXDS);
  twa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXDS);
  for (j=0; j<=jdimx; j++)
  {
    g_array_append_val(isra, num);
    g_array_append_val(ispa, num2);
    g_array_append_val(tca, num3);
    g_array_append_val(twa, num4);
    g_array_append_val(zwa, num5);
    }
  /* deal with graphs? potential bugs */
  }

int main( int argc, char *argv[])
  {
  GtkWidget *window, *vbox, *mnb, *mnu, *smnu, *mni, *opttri, *hpane, *table, *label, *butt;
  GtkAccelGroup *accel_group=NULL;
  GSList *group=NULL, *group3=NULL;
  gdouble fll=0;

  gtk_init(&argc, &argv);
  window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Harmonic Spectrum Analyser");
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
  svg=gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE, NULL);
  gtk_widget_add_accelerator(svg, "activate", accel_group, GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(mnu), svg);
  gtk_widget_show(svg);
  mni=gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
  gtk_widget_show(mni);
  mni=gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accel_group);
  gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(gtk_main_quit), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
  gtk_widget_show(mni);
  mni=gtk_menu_item_new_with_mnemonic("_File");
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
  dlm=gtk_radio_menu_item_new_with_label(group, "Raw Delimited Data");
  group=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(dlm));
  gtk_menu_shell_append(GTK_MENU_SHELL(smnu), dlm);
  gtk_widget_show(dlm);
  mni=gtk_menu_item_new_with_label("Data Format:");
  gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
  gtk_widget_show(mni);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), smnu);
  tracmenu=gtk_menu_new();
  trac=gtk_menu_item_new_with_label("Trace:");
  gtk_menu_shell_append(GTK_MENU_SHELL(mnu), trac);
  gtk_widget_show(trac);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(trac), tracmenu);
  trans=gtk_check_menu_item_new_with_label("Transmission Measurement?");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(trans), TRUE);
  gtk_menu_shell_append(GTK_MENU_SHELL(mnu), trans);
  gtk_widget_show(trans);
  dBs=gtk_check_menu_item_new_with_label("Data in dBs?");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(dBs), TRUE);
  gtk_menu_shell_append(GTK_MENU_SHELL(mnu), dBs);
  gtk_widget_show(dBs);
  neg=gtk_check_menu_item_new_with_label("Negate?");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(neg), FALSE);
  gtk_menu_shell_append(GTK_MENU_SHELL(mnu), neg);
  gtk_widget_show(neg);
  mni=gtk_menu_item_new_with_mnemonic("_Properties");
  gtk_widget_show(mni);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), mnu);
  gtk_menu_shell_append(GTK_MENU_SHELL(mnb), mni);
  mnu=gtk_menu_new();
  smnu=gtk_menu_new();
  ncmp=gtk_radio_menu_item_new_with_label(group3, "None");
  group3=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(ncmp));
  gtk_menu_shell_append(GTK_MENU_SHELL(smnu), ncmp);
  gtk_widget_show(ncmp);
  lcmp=gtk_radio_menu_item_new_with_label(group3, "1st order\nspectral shadowing");
  group3=gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(lcmp));
  gtk_menu_shell_append(GTK_MENU_SHELL(smnu), lcmp);
  gtk_widget_show(lcmp);
  mni=gtk_menu_item_new_with_label("Nonlinear\nCompensation:");
  gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
  gtk_widget_show(mni);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), smnu);
  bat=gtk_check_menu_item_new_with_label("Batch Process Data?");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bat), FALSE);
  gtk_menu_shell_append(GTK_MENU_SHELL(mnu), bat);
  gtk_widget_show(bat);
  /*
  if (check if harmonicconf is installed)
    {
    smnu=gtk_menu_new();
    mni=gtk_menu_item_new_with_label("Create File");
    g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(btc), NULL); change to run executable
    gtk_menu_shell_append(GTK_MENU_SHELL(smnu), mni);
    gtk_widget_show(mni);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(bat), smnu);
    }
  */
  twopionx=gtk_check_menu_item_new_with_label("Invert domain?");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(twopionx), FALSE);
  gtk_menu_shell_append(GTK_MENU_SHELL(mnu), twopionx);
  gtk_widget_show(twopionx);
  opttri=gtk_check_menu_item_new_with_label("Optimise Triangle fit?");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(opttri), FALSE);
  gtk_menu_shell_append(GTK_MENU_SHELL(mnu), opttri);
  gtk_widget_show(opttri);
  mni=gtk_menu_item_new_with_mnemonic("_Advanced");
  gtk_widget_show(mni);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), mnu);
  gtk_menu_shell_append(GTK_MENU_SHELL(mnb), mni);
  mnu=gtk_menu_new();
  mni=gtk_menu_item_new_with_label("Instructions");
  gtk_widget_add_accelerator(mni, "activate", accel_group, GDK_F1, 0, GTK_ACCEL_VISIBLE);
  g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(help), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
  gtk_widget_show(mni);
  mni=gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);;
  g_signal_connect(G_OBJECT(mni), "activate", G_CALLBACK(about), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(mnu), mni);
  gtk_widget_show(mni);
  mni=gtk_menu_item_new_with_mnemonic("_Help");
  gtk_widget_show(mni);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(mni), mnu);
  gtk_menu_shell_append(GTK_MENU_SHELL(mnb), mni);
  gtk_menu_item_right_justify(GTK_MENU_ITEM(mni));
  hpane=gtk_hpaned_new();
  gtk_widget_show(hpane);
  gtk_container_add(GTK_CONTAINER(vbox), hpane);
  notebook=gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
  table=gtk_table_new(5, 3, FALSE);
  gtk_widget_show(table);
  label=gtk_label_new("Spectrum Start:");
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(label);
  adj=(GtkAdjustment *) gtk_adjustment_new(0, -32767, 32768, 1.0, 5.0, 0.0);
  bsr=gtk_spin_button_new(adj, 0.5, 3);
  g_signal_connect(G_OBJECT(bsr), "value-changed", G_CALLBACK(upbsra), NULL);
  gtk_table_attach(GTK_TABLE(table), bsr, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(bsr);
  label=gtk_label_new("Spectrum Stop:");
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(label);
  adj=(GtkAdjustment *) gtk_adjustment_new(0, -32767, 32768, 1.0, 5.0, 0.0);
  bsp=gtk_spin_button_new(adj, 0.5, 3);
  g_signal_connect(G_OBJECT(bsp), "value-changed", G_CALLBACK(upbspa), NULL);
  gtk_table_attach(GTK_TABLE(table), bsp, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(bsp);
  label=gtk_label_new("Offset:");
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(label);
  adj=(GtkAdjustment *) gtk_adjustment_new(0, -32767, 32768, 1.0, 5.0, 0.0);
  fst=gtk_spin_button_new(adj, 0.5, 2);
  gtk_table_attach(GTK_TABLE(table), fst, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(fst);
  label=gtk_label_new("j index:");
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(label);
  adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, MXDM, 1.0, 5.0, 0.0);
  jind=gtk_spin_button_new(adj, 0, 0);
  g_signal_connect(G_OBJECT(jind), "value-changed", G_CALLBACK(upj), NULL);
  gtk_table_attach(GTK_TABLE(table), jind, 2, 3, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(jind);
  label=gtk_label_new("Zero Padding 2^:");
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(label);
  adj=(GtkAdjustment *) gtk_adjustment_new(12, 0, 31, 1.0, 5.0, 0.0);
  zpd=gtk_spin_button_new(adj, 0, 0);
  gtk_table_attach(GTK_TABLE(table), zpd, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(zpd);
  butt=gtk_button_new_with_label("Reset\nArrays");
  g_signal_connect(G_OBJECT(butt), "clicked", G_CALLBACK(reset), NULL);
  gtk_table_attach(GTK_TABLE(table), butt, 2, 3, 2, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(butt);
  tr=gtk_button_new_with_label("Transform Spectrum");
  gtk_table_attach(GTK_TABLE(table), tr, 0, 3, 4, 5, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(tr);label = gtk_label_new("Spectrum");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), table, label);
  table=gtk_table_new(6, 3, FALSE);
  gtk_widget_show(table);
  label=gtk_label_new("Inverse Spectrum Start:");
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(label);
  adj=(GtkAdjustment *) gtk_adjustment_new(0, -32767, 32768, 1.0, 5.0, 0.0);
  isr=gtk_spin_button_new(adj, 0.5, 3);
  g_signal_connect(G_OBJECT(isr), "value-changed", G_CALLBACK(upisra), NULL);
  gtk_table_attach(GTK_TABLE(table), isr, 0, 1, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(isr);
  label=gtk_label_new("Inverse Spectrum Stop:");
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(label);
  adj=(GtkAdjustment *) gtk_adjustment_new(0, -32767, 32768, 1.0, 5.0, 0.0);
  isp=gtk_spin_button_new(adj, 0.5, 3);
  g_signal_connect(G_OBJECT(isp), "value-changed", G_CALLBACK(upispa), NULL);
  gtk_table_attach(GTK_TABLE(table), isp, 1, 2, 1, 2, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(isp);
  label=gtk_label_new("Triangle Centre:");
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(label);
  adj=(GtkAdjustment *) gtk_adjustment_new(0, -32767, 32768, 1.0, 5.0, 0.0);
  tc=gtk_spin_button_new(adj, 0.5, 3);
  g_signal_connect(G_OBJECT(tc), "value-changed", G_CALLBACK(uptca), NULL);
  gtk_table_attach(GTK_TABLE(table), tc, 0, 1, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(tc);
  label=gtk_label_new("Triangle Full Width:");
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(label);
  adj=(GtkAdjustment *) gtk_adjustment_new(2, 0, 65535, 1.0, 5.0, 0.0);
  tw=gtk_spin_button_new(adj, 0.5, 3);
  g_signal_connect(G_OBJECT(tw), "value-changed", G_CALLBACK(uptwa), NULL);
  gtk_table_attach(GTK_TABLE(table), tw, 1, 2, 3, 4, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(tw);
  label=gtk_label_new("DC Peak Width:");
  gtk_table_attach(GTK_TABLE(table), label, 1, 2, 4, 5, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(label);
  adj=(GtkAdjustment *) gtk_adjustment_new(2, 0, 65535, 1.0, 5.0, 0.0);
  zw=gtk_spin_button_new(adj, 0.5, 3);
  g_signal_connect(G_OBJECT(zw), "value-changed", G_CALLBACK(upzwa), NULL);
  gtk_table_attach(GTK_TABLE(table), zw, 1, 2, 5, 6, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(zw);
  label=gtk_label_new("j index:");
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(label);
  adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, 0, 1.0, 5.0, 0.0);
  jind2=gtk_spin_button_new(adj, 0, 0);
  g_signal_connect(G_OBJECT(jind2), "value-changed", G_CALLBACK(upj), NULL);
  gtk_table_attach(GTK_TABLE(table), jind2, 2, 3, 1, 2, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 2, 2);
  gtk_widget_show(jind2);
  label=gtk_label_new("k index:");
  gtk_table_attach(GTK_TABLE(table), label, 2, 3, 2, 3, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(label);
  adj=(GtkAdjustment *) gtk_adjustment_new(0, 0, 0, 1.0, 5.0, 0.0);
  kind=gtk_spin_button_new(adj, 0, 0);
  g_signal_connect(G_OBJECT(kind), "value-changed", G_CALLBACK(upk), NULL);
  gtk_table_attach(GTK_TABLE(table), kind, 2, 3, 3, 4, GTK_FILL|GTK_SHRINK, GTK_FILL|GTK_SHRINK, 2, 2);
  gtk_widget_show(kind);
  butt=gtk_button_new_with_label("Reset\nArrays");
  g_signal_connect(G_OBJECT(butt), "clicked", G_CALLBACK(reset2), NULL);
  gtk_table_attach(GTK_TABLE(table), butt, 2, 3, 4, 6, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(butt);
  pr=gtk_button_new_with_label("Process\nSpectrum");
  gtk_table_attach(GTK_TABLE(table), pr, 0, 1, 4, 6, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  gtk_widget_show(pr);
  label=gtk_label_new("Inverse Spectrum");
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
  label=gtk_label_new("Spectrum");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), table, label);
  table=gtk_table_new(1, 1, FALSE);
  gtk_widget_show(table);
  plot2=plot_linear_new();
  ((PLOT_LINEAR(plot2))->xlab)=g_strdup("Inverse Domain");
  g_signal_connect(plot2, "moved", G_CALLBACK(pltmv), NULL);
  gtk_widget_show(plot2);
  gtk_table_attach(GTK_TABLE(table), plot2, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  label=gtk_label_new("Inverse Spectrum");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), table, label);
  table=gtk_table_new(1, 1, FALSE);
  gtk_widget_show(table);
  plot3=plot_linear_new();
  g_signal_connect(plot3, "moved", G_CALLBACK(pltmv), NULL);
  gtk_widget_show(plot3);
  gtk_table_attach(GTK_TABLE(table), plot3, 0, 1, 0, 1, GTK_FILL|GTK_SHRINK|GTK_EXPAND, GTK_FILL|GTK_SHRINK|GTK_EXPAND, 2, 2);
  label=gtk_label_new("Analysis Results");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), table, label);
  gtk_widget_show(notebook2);
  gtk_paned_add2(GTK_PANED(hpane), notebook2);
  statusbar=gtk_statusbar_new();
  gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 2);
  gtk_widget_show(statusbar);
  bsra=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXD);
  g_array_append_val(bsra, fll);
  bspa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXD);
  g_array_append_val(bspa, fll);
  zwa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXD);
  isra=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXDS);
  ispa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXDS);
  tca=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXDS);
  twa=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), MXDS);
  x=g_array_new(FALSE, FALSE, sizeof(gdouble));
  yb=g_array_new(FALSE, FALSE, sizeof(gdouble));
  specs=g_array_new(FALSE, FALSE, sizeof(gdouble));
  stars=g_array_new(FALSE, FALSE, sizeof(gdouble));
  xsb=g_array_new(FALSE, FALSE, sizeof(gdouble));
  ysb=g_array_new(FALSE, FALSE, sizeof(gdouble));
  delx=g_array_new(FALSE, FALSE, sizeof(gdouble));
  gtk_widget_show(window);
  gtk_main();
  return 0;
  }
