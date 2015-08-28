/*
 * Copyright (C) 2011 Nick Schermer <nick@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundatoin; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundatoin, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gtk/gtk.h>
#include <xfconf/xfconf.h>
#include <migrate/migrate-config.h>



static guint
migrate_config_strchr_count (const gchar *haystack,
                             const gchar  needle)
{
  const gchar *p;
  guint        count;

  if (G_UNLIKELY (haystack != NULL))
    {
      for (p = haystack, count = 0; *p != '\0'; ++p)
        if (*p == needle)
          count++;
    }

  return count;
}



static void
xubuntu_migrate_tasklist (GHashTable    *plugins,
                          XfconfChannel *channel)
{
  const GValue *value;
  GPtrArray    *array;
  guint         j;
  gint          plugin_id, tsk_index, sep_index;

  /* First, check that plugin-2 and plugin-3 still match the default xubuntu
   * settings */
  value = g_hash_table_lookup (plugins, "/plugins/plugin-2");
  if (!G_VALUE_HOLDS_STRING (value)
      || g_strcmp0 (g_value_get_string (value), "tasklist") != 0)
    return;
  value = g_hash_table_lookup (plugins, "/plugins/plugin-3");
  if (!G_VALUE_HOLDS_STRING (value)
      || g_strcmp0 (g_value_get_string (value), "separator") != 0)
    return;

  /* Then, check that plugin-2 (tasklist) and plugin-3 (separator) are on the
   * first same panel as expected */
  array = xfconf_channel_get_arrayv (XFCONF_CHANNEL (channel), "/panels/panel-0/plugin-ids");
  if (array == NULL)
    return;

  tsk_index = sep_index = 0;
  for (j = 0; j < array->len; j++)
    {
      /* get the plugin id */
      value = g_ptr_array_index (array, j);
      if (value == NULL)
        continue;

      plugin_id = g_value_get_int (value);
      if (plugin_id == 2)
        tsk_index = j;
      else if (plugin_id == 3)
        sep_index = j;
    }
  xfconf_array_free (array);

  if (tsk_index == 0
      || sep_index == 0
      || sep_index != tsk_index + 1)
    return;

  /* Apparently this is the default xubuntu layout, so expand the
   * separator to workaround non-expanding tasklist in 4.10 */
  xfconf_channel_set_bool (XFCONF_CHANNEL (channel), "/plugins/plugin-3/expand", TRUE);
}



static void
migrate_config_session_menu (gpointer key,
                             gpointer value,
                             gpointer channel)
{
  const GValue *gvalue = value;
  const gchar  *prop = key;

  /* skip non root plugin properties */
  if (!G_VALUE_HOLDS_STRING (gvalue)
      || migrate_config_strchr_count (prop, G_DIR_SEPARATOR) != 2
      || g_strcmp0 (g_value_get_string (gvalue), "xfsm-logout-plugin") != 0)
    return;

  /* this plugin never had any properties and matches the default
   * settings of the new actions plugin */
  xfconf_channel_set_string (XFCONF_CHANNEL (channel), prop, "actions");
}



static const gchar *
migrate_config_action_48_convert (gint action)
{
  switch (action)
    {
    case 1: /* ACTION_LOG_OUT_DIALOG */
      return "+logout-dialog";

    case 2: /* ACTION_LOG_OUT */
      return "+logout";

    case 3: /* ACTION_LOCK_SCREEN */
      return "+lock-screen";

    case 4: /* ACTION_SHUT_DOWN */
      return "+shutdown";

    case 5: /* ACTION_RESTART */
      return "+restart";

    case 6: /* ACTION_SUSPEND */
      return "+suspend";

    case 7: /* ACTION_HIBERNATE */
      return "+hibernate";

    default: /* ACTION_DISABLED */
      return "-switch-user"; /* something else */
    }
}



static void
migrate_config_action_48 (gpointer key,
                          gpointer value,
                          gpointer channel)
{
  const GValue *gvalue = value;
  const gchar  *prop = key;
  gchar         str[64];
  gint          first_action_int;
  gint          second_action_int;
  const gchar  *first_action;
  const gchar  *second_action;

  /* skip non root plugin properties */
  if (!G_VALUE_HOLDS_STRING (gvalue)
      || migrate_config_strchr_count (prop, G_DIR_SEPARATOR) != 2
      || g_strcmp0 (g_value_get_string (gvalue), "actions") != 0)
    return;

  /* this is a bug that affects pre users: don't try to migrate
   * when the appearance property is already set */
  g_snprintf (str, sizeof (str), "%s/appearance", prop);
  if (xfconf_channel_has_property (channel, str))
    return;

  /* set appearance to button mode */
  xfconf_channel_set_uint (channel, str, 0);

  /* read and remove the old properties */
  g_snprintf (str, sizeof (str), "%s/first-action", prop);
  first_action_int = xfconf_channel_get_uint (channel, str, 0) + 1;
  xfconf_channel_reset_property (channel, str, FALSE);

  g_snprintf (str, sizeof (str), "%s/second-action", prop);
  second_action_int = xfconf_channel_get_uint (channel, str, 0);
  xfconf_channel_reset_property (channel, str, FALSE);

  /* corrections for new plugin */
  if (first_action_int == 0)
    first_action_int = 1;
  if (first_action_int == second_action_int)
    second_action_int = 0;

  /* set orientation */
  g_snprintf (str, sizeof (str), "%s/invert-orientation", prop);
  xfconf_channel_set_bool (channel, str, second_action_int > 0);

  /* convert the old value to new ones */
  first_action = migrate_config_action_48_convert (first_action_int);
  second_action = migrate_config_action_48_convert (second_action_int);

  /* set the visible properties */
  g_snprintf (str, sizeof (str), "%s/items", prop);
  xfconf_channel_set_array (channel, str,
                            G_TYPE_STRING, first_action,
                            G_TYPE_STRING, second_action,
                            G_TYPE_INVALID);
}



gboolean
migrate_config (XfconfChannel  *channel,
                gint            configver,
                GError        **error)
{
  GHashTable *plugins;
  guint       n, n_panels;
  gchar       buf[50];
  gboolean    horizontal;

  plugins = xfconf_channel_get_properties (channel, "/plugins");

  /* migrate plugins to the new actions plugin */
  if (configver < 1)
    {
      /* migrate xfsm-logout-plugin */
      g_hash_table_foreach (plugins, migrate_config_session_menu, channel);

      /* migrate old action plugins */
      g_hash_table_foreach (plugins, migrate_config_action_48, channel);

      /* hack for Xubuntu: expand the separator next to the tasklist */
      xubuntu_migrate_tasklist (plugins, channel);
    }

  /* migrate horizontal to mode property */
  if (configver < 2)
    {
      n_panels = xfconf_channel_get_uint (channel, "/panels", 0);
      for (n = 0; n < n_panels; n++)
        {
          /* read and remove old property */
          g_snprintf (buf, sizeof (buf), "/panels/panel-%u/horizontal", n);
          horizontal = xfconf_channel_get_bool (channel, buf, TRUE);
          xfconf_channel_reset_property (channel, buf, FALSE);

          /* set new mode */
          g_snprintf (buf, sizeof (buf), "/panels/panel-%u/mode", n);
          xfconf_channel_set_uint (channel, buf, horizontal ? 0 : 1);
        }
    }

  g_hash_table_destroy (plugins);

  return TRUE;
}
