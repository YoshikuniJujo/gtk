/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */

/* GTK+: gtkfilechooserbutton.c
 * 
 * Copyright (c) 2004 James M. Cape <jcape@ignore-your.tv>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <string.h>

#include <gtk/gtk.h>

static const char *backend = "gtk+";
static gboolean rtl = FALSE;
static GOptionEntry entries[] = {
  { "backend", 'b', 0, G_OPTION_ARG_STRING, &backend, "The filesystem backend to use.", "gtk+" },
  { "right-to-left", 'r', 0, G_OPTION_ARG_NONE, &rtl, "Force right-to-left layout.", NULL },
  { NULL }
};

static char *gtk_src_dir = NULL;

static void
print_selected_path_clicked_cb (GtkWidget *button,
				gpointer   user_data)
{
  GFile *folder, *filename;
  char *folder_uri, *filename_uri;

  folder = gtk_file_chooser_get_current_folder (user_data);
  filename = gtk_file_chooser_get_file (user_data);

  folder_uri = g_file_get_uri (folder);
  filename_uri = g_file_get_uri (filename);
  g_message ("Currently Selected:\n\tFolder: `%s'\n\tFilename: `%s'\nDone.\n",
	     folder_uri, filename_uri);
  g_free (folder_uri);
  g_free (filename_uri);

  g_object_unref (folder);
  g_object_unref (filename);
}

static void
add_pwds_parent_as_shortcut_clicked_cb (GtkWidget *button,
					gpointer   user_data)
{
  GFile *path = g_file_new_for_path (gtk_src_dir);
  GError *err = NULL;

  if (!gtk_file_chooser_add_shortcut_folder (user_data, path, &err))
    {
      g_message ("Couldn't add `%s' as shortcut folder: %s", gtk_src_dir,
		 err->message);
      g_error_free (err);
    }
  else
    {
      g_message ("Added `%s' as shortcut folder.", gtk_src_dir);
    }

  g_object_unref (path);
}

static void
del_pwds_parent_as_shortcut_clicked_cb (GtkWidget *button,
					gpointer   user_data)
{
  GFile *path = g_file_new_for_path (gtk_src_dir);
  GError *err = NULL;

  if (!gtk_file_chooser_remove_shortcut_folder (user_data, path, &err))
    {
      g_message ("Couldn't remove `%s' as shortcut folder: %s", gtk_src_dir,
		 err->message);
      g_error_free (err);
    }
  else
    {
      g_message ("Removed `%s' as shortcut folder.", gtk_src_dir);
    }

  g_object_unref (path);
}

static void
tests_button_clicked_cb (GtkButton *real_button,
			 gpointer   user_data)
{
  GtkWidget *tests;

  tests = g_object_get_data (user_data, "tests-dialog");

  if (tests == NULL)
    {
      GtkWidget *box, *button;

      tests = gtk_window_new ();
      gtk_window_set_hide_on_close (GTK_WINDOW (tests), TRUE);
      gtk_window_set_title (GTK_WINDOW (tests),
			    "Tests - TestFileChooserButton");
      gtk_window_set_transient_for (GTK_WINDOW (tests),
				    GTK_WINDOW (gtk_widget_get_root (user_data)));

      box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
      gtk_box_append (GTK_BOX (tests), box);

      button = gtk_button_new_with_label ("Print Selected Path");
      g_signal_connect (button, "clicked",
			G_CALLBACK (print_selected_path_clicked_cb), user_data);
      gtk_box_append (GTK_BOX (box), button);

      button = gtk_button_new_with_label ("Add $PWD's Parent as Shortcut");
      g_signal_connect (button, "clicked",
			G_CALLBACK (add_pwds_parent_as_shortcut_clicked_cb), user_data);
      gtk_box_append (GTK_BOX (box), button);

      button = gtk_button_new_with_label ("Remove $PWD's Parent as Shortcut");
      g_signal_connect (button, "clicked",
			G_CALLBACK (del_pwds_parent_as_shortcut_clicked_cb), user_data);
      gtk_box_append (GTK_BOX (box), button);

      g_object_set_data (user_data, "tests-dialog", tests);
    }

  gtk_window_present (GTK_WINDOW (tests));
}

static void
chooser_selection_changed_cb (GtkFileChooser *chooser,
			      gpointer        user_data)
{
  GFile *filename;
  char *uri;

  filename = gtk_file_chooser_get_file (chooser);

  uri = g_file_get_uri (filename);
  g_message ("%s::selection-changed\n\tSelection:`%s'\nDone.\n",
	     G_OBJECT_TYPE_NAME (chooser), uri);
  g_free (uri);

  g_object_unref (filename);
}

static void
add_new_filechooser_button (const char           *mnemonic,
                            const char           *chooser_title,
                            GtkFileChooserAction  action,
                            GtkWidget            *group_box,
                            GtkSizeGroup         *label_group)
{
  GtkWidget *hbox, *label, *chooser, *button;
  GFile *path;

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_box_append (GTK_BOX (group_box), hbox);

  label = gtk_label_new_with_mnemonic (mnemonic);
  gtk_size_group_add_widget (GTK_SIZE_GROUP (label_group), label);
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_box_append (GTK_BOX (hbox), label);

  chooser = gtk_file_chooser_button_new (g_strconcat(chooser_title,
                                                     " - testfilechooserbutton", NULL),
                                         action);
  gtk_widget_set_hexpand (chooser, TRUE);

  path = g_file_new_for_path (gtk_src_dir);
  gtk_file_chooser_add_shortcut_folder (GTK_FILE_CHOOSER (chooser), path, NULL);
  gtk_file_chooser_remove_shortcut_folder (GTK_FILE_CHOOSER (chooser), path, NULL);
  g_object_unref (path);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label), chooser);
  g_signal_connect (chooser, "selection-changed", G_CALLBACK (chooser_selection_changed_cb), NULL);
  gtk_box_append (GTK_BOX (hbox), chooser);

  button = gtk_button_new_with_label ("Tests");
  g_signal_connect (button, "clicked", G_CALLBACK (tests_button_clicked_cb), chooser);
  gtk_box_append (GTK_BOX (hbox), button);
}

static void
quit_cb (GtkWidget *widget,
         gpointer   data)
{
  gboolean *done = data;

  *done = TRUE;

  g_main_context_wakeup (NULL);
}

int
main (int   argc,
      char *argv[])
{
  GtkWidget *win, *vbox, *frame, *group_box;
  GtkSizeGroup *label_group;
  GOptionContext *context;
  char *cwd;
  gboolean done = FALSE;

  context = g_option_context_new ("- test GtkFileChooserButton widget");
  g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
  g_option_context_parse (context, &argc, &argv, NULL);
  g_option_context_free (context);

  gtk_init ();

  /* to test rtl layout, use "--right-to-left" */
  if (rtl)
    gtk_widget_set_default_direction (GTK_TEXT_DIR_RTL);

  cwd = g_get_current_dir();
  gtk_src_dir = g_path_get_dirname (cwd);
  g_free (cwd);

  win = gtk_dialog_new_with_buttons ("TestFileChooserButton", NULL, 0,
				     "_Quit", GTK_RESPONSE_CLOSE, NULL);
  g_signal_connect (win, "response", G_CALLBACK (quit_cb), &done);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
  gtk_widget_set_margin_start (vbox, 6);
  gtk_widget_set_margin_end (vbox, 6);
  gtk_widget_set_margin_top (vbox, 6);
  gtk_widget_set_margin_bottom (vbox, 6);
  gtk_box_append (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (win))), vbox);

  frame = gtk_frame_new ("<b>GtkFileChooserButton</b>");
  gtk_label_set_use_markup (GTK_LABEL (gtk_frame_get_label_widget (GTK_FRAME (frame))), TRUE);
  gtk_box_append (GTK_BOX (vbox), frame);

  gtk_widget_set_halign (frame, GTK_ALIGN_FILL);
  gtk_widget_set_valign (frame, GTK_ALIGN_FILL);
  g_object_set (frame, "margin-top", 6, "margin-start", 12, NULL);

  label_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  group_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_frame_set_child (GTK_FRAME (frame), group_box);

  /* OPEN */
  add_new_filechooser_button ("_Open:", "Select A File",
                              GTK_FILE_CHOOSER_ACTION_OPEN,
                              group_box, label_group);

  /* SELECT_FOLDER */
  add_new_filechooser_button ("Select _Folder:", "Select A Folder",
                              GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                              group_box, label_group);

  g_object_unref (label_group);

  gtk_widget_show (win);
  gtk_window_present (GTK_WINDOW (win));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
