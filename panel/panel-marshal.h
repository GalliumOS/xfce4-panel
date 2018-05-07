
#ifndef ___panel_marshal_MARSHAL_H__
#define ___panel_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:UINT,BOOLEAN (panel-marshal.list:1) */
G_GNUC_INTERNAL void _panel_marshal_VOID__UINT_BOOLEAN (GClosure     *closure,
                                                        GValue       *return_value,
                                                        guint         n_param_values,
                                                        const GValue *param_values,
                                                        gpointer      invocation_hint,
                                                        gpointer      marshal_data);

/* VOID:STRING,BOXED,UINT (panel-marshal.list:2) */
G_GNUC_INTERNAL void _panel_marshal_VOID__STRING_BOXED_UINT (GClosure     *closure,
                                                             GValue       *return_value,
                                                             guint         n_param_values,
                                                             const GValue *param_values,
                                                             gpointer      invocation_hint,
                                                             gpointer      marshal_data);

G_END_DECLS

#endif /* ___panel_marshal_MARSHAL_H__ */

