/*
 * Copyright (C) 2009-2010 Nick Schermer <nick@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gtk/gtk.h>
#include <xfconf/xfconf.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/xfce-panel-macros.h>

#include <migrate/migrate-46.h>
#include <migrate/migrate-config.h>
#include <migrate/migrate-default.h>



#define DEFAULT_CONFIG_FILENAME "xfce4" G_DIR_SEPARATOR_S "panel" G_DIR_SEPARATOR_S "default.xml"
#define DEFAULT_CONFIG_PATH     XDGCONFIGDIR G_DIR_SEPARATOR_S DEFAULT_CONFIG_FILENAME


gint
main (gint argc, gchar **argv)
{
  GError        *error = NULL;
  GtkWidget     *dialog;
  GtkWidget     *button;
  gint           result;
  gint           retval = EXIT_SUCCESS;
  gint           default_response = GTK_RESPONSE_CANCEL;
  XfconfChannel *channel;
  gint           configver;
  gchar         *filename_46;
  gchar         *filename_default;
  gboolean       migrate_vendor_default;

  /* set translation domain */
  xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

#ifndef NDEBUG
  /* terminate the program on warnings and critical messages */
  g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING);
#endif

  gtk_init (&argc, &argv);

  if (!xfconf_init (&error))
    {
      g_critical ("Failed to initialize Xfconf: %s", error->message);
      g_error_free (error);
      return EXIT_FAILURE;
    }

  channel = xfconf_channel_get (XFCE_PANEL_CHANNEL_NAME);
  if (!xfconf_channel_has_property (channel, "/panels"))
    {
      /* lookup the old 4.6 config file */
      filename_46 = xfce_resource_lookup (XFCE_RESOURCE_CONFIG, XFCE_46_CONFIG);

      /* lookup the default configuration */
      xfce_resource_push_path (XFCE_RESOURCE_CONFIG, XDGCONFIGDIR);
      filename_default = xfce_resource_lookup (XFCE_RESOURCE_CONFIG, DEFAULT_CONFIG_FILENAME);
      xfce_resource_pop_path (XFCE_RESOURCE_CONFIG);

      if (filename_46 == NULL && filename_default == NULL)
        {
          g_warning ("No default or old configuration found");
          return EXIT_FAILURE;
        }

      /* if the default configuration does not match with the file found
       * by the resource lookup, migrate it without asking */
      migrate_vendor_default = (g_strcmp0 (DEFAULT_CONFIG_PATH, filename_default) != 0);

      /* check if we auto-migrate the default configuration */
      if (g_getenv ("XFCE_PANEL_MIGRATE_DEFAULT") != NULL
          || migrate_vendor_default)
        {
          if (filename_46 != NULL)
            g_message ("Tried to auto-migrate, but old configuration found");
          else if (filename_default == NULL)
            g_message ("Tried to auto-migrate, but no default configuration found");
          else
            goto migrate_default;
        }

      /* create question dialog */
      dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
                                       _("Welcome to the first start of the panel"));
      gtk_window_set_title (GTK_WINDOW (dialog), _("Panel"));
      gtk_window_set_icon_name (GTK_WINDOW (dialog), GTK_STOCK_PREFERENCES);
      gtk_window_stick (GTK_WINDOW (dialog));
      gtk_window_set_keep_above (GTK_WINDOW (dialog), TRUE);

      if (filename_46 != NULL)
        {
          gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s\n%s",
              _("Because the panel moved to a new system for storing the "
                "settings, it has to load a fresh initial configuration."),
              _("Choose below which setup you want for the first startup."));

          button = gtk_dialog_add_button (GTK_DIALOG (dialog), _("Migrate old config"), GTK_RESPONSE_OK);
          gtk_widget_set_tooltip_text (button, _("Migrate the old 4.6 configuration to Xfconf"));
          default_response = GTK_RESPONSE_OK;
        }
      else
        {
          gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
              _("Choose below which setup you want for the first startup."));
        }

      if (filename_default != NULL)
        {
          button = gtk_dialog_add_button (GTK_DIALOG (dialog), _("Use default config"), GTK_RESPONSE_YES);
          gtk_widget_set_tooltip_text (button, _("Load the default configuration"));

          if (default_response == GTK_RESPONSE_CANCEL)
            default_response = GTK_RESPONSE_YES;
        }

      button = gtk_dialog_add_button (GTK_DIALOG (dialog), _("One empty panel"), GTK_RESPONSE_CANCEL);
      gtk_widget_set_tooltip_text (button, _("Start with one empty panel"));

      gtk_dialog_set_default_response (GTK_DIALOG (dialog), default_response);
      result = gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);

      if (result == GTK_RESPONSE_OK && filename_46 != NULL)
        {
          /* restore 4.6 config */
          if (!migrate_46 (filename_46, channel, &error))
            {
              xfce_dialog_show_error (NULL, error, _("Failed to migrate the old panel configuration"));
              g_error_free (error);
              retval = EXIT_FAILURE;
            }
        }
      else if (result == GTK_RESPONSE_YES && filename_default != NULL)
        {
          migrate_default:

          /* apply default config */
          if (!migrate_default (filename_default, &error))
            {
              xfce_dialog_show_error (NULL, error, _("Failed to load the default configuration"));
              g_error_free (error);
              retval = EXIT_FAILURE;
            }
        }

      g_free (filename_46);
      g_free (filename_default);
    }

  configver = xfconf_channel_get_int (channel, "/configver", -1);
  if (configver < XFCE4_PANEL_CONFIG_VERSION)
    {
      g_message (_("Panel config needs migration..."));

      if (!migrate_config (channel, configver, &error))
        {
          xfce_dialog_show_error (NULL, error, _("Failed to migrate the existing configuration"));
          g_error_free (error);
          retval = EXIT_FAILURE;
        }
      else
        {
          g_message (_("Panel configuration has been updated."));
        }

      /* migration complete, set new version */
      xfconf_channel_set_int (channel, "/configver", XFCE4_PANEL_CONFIG_VERSION);
    }

  xfconf_shutdown ();

  return retval;
}
