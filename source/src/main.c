#include <gtk/gtk.h>
#include "ruler-window.h"

#define RULER_VERSION "2.0"

static gboolean
on_handle_local_options(GApplication *application,
						GVariantDict *options, gpointer user_data)
{
	(void)application;
	(void)user_data;

	if (g_variant_dict_contains(options, "version")) {
		g_print("\\ GTK Ruler \\ Tool V%s\n", RULER_VERSION);
		g_print("                😃 by Mario Lohajner 2026\n");
		g_print("\n");
		return 0;
	}

	return -1;
}

static void
on_activate(GtkApplication *app, gpointer user_data)
{
	(void)user_data;

	RulerWindow *rw = ruler_window_new(app);

/*
 * RulerWindow structure attached to GtkWindow -
 * gtk_window_close() frees RulerWindow structure.
 */
	g_object_set_data_full(G_OBJECT(rw->window),
			"ruler-window", rw, (GDestroyNotify)ruler_window_free);

}

int
main(int argc, char **argv)
{
	GtkApplication *app =
		gtk_application_new("hr.manjo.ruler",
										G_APPLICATION_DEFAULT_FLAGS);

	static const GOptionEntry options[] = {
		{
			"version",
			'v',
			G_OPTION_FLAG_NONE,
			G_OPTION_ARG_NONE,
			NULL,
			"Show version information",
			NULL
		},
		{
			NULL
		}
	};

	g_application_add_main_option_entries(G_APPLICATION(app), options);

	g_signal_connect(app,"handle-local-options",
						G_CALLBACK(on_handle_local_options), NULL);

	g_signal_connect(app,"activate",
									G_CALLBACK(on_activate), NULL);

	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	return status;
}
