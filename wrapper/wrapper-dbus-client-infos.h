/* Generated by dbus-binding-tool; do not edit! */

#include <glib.h>
#include <dbus/dbus-glib.h>

G_BEGIN_DECLS

#ifndef _DBUS_GLIB_ASYNC_DATA_FREE
#define _DBUS_GLIB_ASYNC_DATA_FREE
static inline void
_dbus_glib_async_data_free (gpointer stuff)
{
	g_slice_free (DBusGAsyncData, stuff);
}
#endif

#ifndef DBUS_GLIB_CLIENT_WRAPPERS_org_xfce_Panel_Wrapper
#define DBUS_GLIB_CLIENT_WRAPPERS_org_xfce_Panel_Wrapper

static inline gboolean
wrapper_dbus_provider_signal (DBusGProxy *proxy, const guint IN_signal, GError **error)

{
  dbus_g_proxy_call_no_reply (proxy, "ProviderSignal", G_TYPE_UINT, IN_signal, G_TYPE_INVALID, G_TYPE_INVALID);
  return TRUE;
}

typedef void (*wrapper_dbus_provider_signal_reply) (DBusGProxy *proxy, GError *error, gpointer userdata);

static void
wrapper_dbus_provider_signal_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  dbus_g_proxy_end_call (proxy, call, &error, G_TYPE_INVALID);
  (*(wrapper_dbus_provider_signal_reply)data->cb) (proxy, error, data->userdata);
  return;
}

static inline DBusGProxyCall*
wrapper_dbus_provider_signal_async (DBusGProxy *proxy, const guint IN_signal, wrapper_dbus_provider_signal_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_slice_new (DBusGAsyncData);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "ProviderSignal", wrapper_dbus_provider_signal_async_callback, stuff, _dbus_glib_async_data_free, G_TYPE_UINT, IN_signal, G_TYPE_INVALID);
}
static inline gboolean
wrapper_dbus_remote_event_result (DBusGProxy *proxy, const guint IN_handle, const gboolean IN_result, GError **error)

{
  dbus_g_proxy_call_no_reply (proxy, "RemoteEventResult", G_TYPE_UINT, IN_handle, G_TYPE_BOOLEAN, IN_result, G_TYPE_INVALID, G_TYPE_INVALID);
  return TRUE;
}

typedef void (*wrapper_dbus_remote_event_result_reply) (DBusGProxy *proxy, GError *error, gpointer userdata);

static void
wrapper_dbus_remote_event_result_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  dbus_g_proxy_end_call (proxy, call, &error, G_TYPE_INVALID);
  (*(wrapper_dbus_remote_event_result_reply)data->cb) (proxy, error, data->userdata);
  return;
}

static inline DBusGProxyCall*
wrapper_dbus_remote_event_result_async (DBusGProxy *proxy, const guint IN_handle, const gboolean IN_result, wrapper_dbus_remote_event_result_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_slice_new (DBusGAsyncData);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "RemoteEventResult", wrapper_dbus_remote_event_result_async_callback, stuff, _dbus_glib_async_data_free, G_TYPE_UINT, IN_handle, G_TYPE_BOOLEAN, IN_result, G_TYPE_INVALID);
}
#endif /* defined DBUS_GLIB_CLIENT_WRAPPERS_org_xfce_Panel_Wrapper */

G_END_DECLS
