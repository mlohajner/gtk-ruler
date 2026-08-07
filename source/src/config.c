#include "config.h"
#include <glib/gstdio.h>

#define CONFIG_GROUP "ruler"

static gchar *
config_file_path(void)
{
	return g_build_filename(g_get_user_config_dir(), "ruler", "ruler.conf", NULL);
}

RulerConfig *
ruler_config_new_defaults(void)
{
	RulerConfig *cfg = g_new0(RulerConfig, 1);

/* Default config values */
	cfg->options = 255;
	cfg->area = 10;
	cfg->window_width = 290;
	cfg->window_height = 266;
	cfg->window_x = 100;
	cfg->window_y = 100;
	cfg->opacity = 0.5;
	gdk_rgba_parse(&cfg->back_color, "#FF8000");
	cfg->mul = 1.0f;
	cfg->sym = g_strdup("px");
	cfg->dec = 0;
	cfg->divs = 4;
	cfg->divm = 25;
	cfg->ref = 0;

	cfg->aspect = (gdouble)cfg->window_width / (gdouble)cfg->window_height;
	cfg->color_pick_str = g_strdup("");
	cfg->area_str = g_strdup("");

	return cfg;
}

void
ruler_config_free(RulerConfig *cfg)
{
	if (!cfg)
		return;
	g_free(cfg->sym);
	g_free(cfg->color_pick_str);
	g_free(cfg->area_str);
	g_free(cfg);
}

void
ruler_config_load(RulerConfig *cfg)
{
	g_autofree gchar *path = config_file_path();
	g_autoptr(GKeyFile) kf = g_key_file_new();
	g_autoptr(GError) err = NULL;

	if (!g_key_file_load_from_file(kf, path, G_KEY_FILE_NONE, &err)) {
/* ruler.conf not found */
		return;
	}

	cfg->options = (guint32)g_key_file_get_integer(kf, CONFIG_GROUP, "options", NULL);
	cfg->area = g_key_file_get_integer(kf, CONFIG_GROUP, "area", NULL);
	cfg->window_x = g_key_file_get_integer(kf, CONFIG_GROUP, "x", NULL);
	cfg->window_y = g_key_file_get_integer(kf, CONFIG_GROUP, "y", NULL);
	cfg->window_width = g_key_file_get_integer(kf, CONFIG_GROUP, "width", NULL);
	cfg->window_height = g_key_file_get_integer(kf, CONFIG_GROUP, "height", NULL);
	cfg->opacity = g_key_file_get_double(kf, CONFIG_GROUP, "opacity", NULL);

	g_autofree gchar *color_str = g_key_file_get_string(kf, CONFIG_GROUP, "backcolor", NULL);
	if (color_str)
		gdk_rgba_parse(&cfg->back_color, color_str);

	cfg->mul = (gfloat)g_key_file_get_double(kf, CONFIG_GROUP, "mul", NULL);

	g_free(cfg->sym);
	cfg->sym = g_key_file_get_string(kf, CONFIG_GROUP, "sym", NULL);
	if (!cfg->sym)
		cfg->sym = g_strdup("px");

	cfg->dec = g_key_file_get_integer(kf, CONFIG_GROUP, "dec", NULL);
	cfg->divs = g_key_file_get_integer(kf, CONFIG_GROUP, "divs", NULL);
	cfg->divm = g_key_file_get_integer(kf, CONFIG_GROUP, "divm", NULL);
	cfg->ref = g_key_file_get_integer(kf, CONFIG_GROUP, "ref", NULL);

	if (cfg->window_height != 0)
		cfg->aspect = (gdouble)cfg->window_width / (gdouble)cfg->window_height;
}

void
ruler_config_save(const RulerConfig *cfg)
{
	g_autofree gchar *path = config_file_path();
	g_autofree gchar *dir = g_path_get_dirname(path);
	g_mkdir_with_parents(dir, 0755);

	g_autoptr(GKeyFile) kf = g_key_file_new();

	g_key_file_set_integer(kf, CONFIG_GROUP, "options", (gint)cfg->options);
	g_key_file_set_integer(kf, CONFIG_GROUP, "area", cfg->area);
	g_key_file_set_integer(kf, CONFIG_GROUP, "x", cfg->window_x);
	g_key_file_set_integer(kf, CONFIG_GROUP, "y", cfg->window_y);
	g_key_file_set_integer(kf, CONFIG_GROUP, "width", cfg->window_width);
	g_key_file_set_integer(kf, CONFIG_GROUP, "height", cfg->window_height);
	g_key_file_set_double(kf, CONFIG_GROUP, "opacity", cfg->opacity);

	gchar *color_str = gdk_rgba_to_string(&cfg->back_color);
	g_key_file_set_string(kf, CONFIG_GROUP, "backcolor", color_str);
	g_free(color_str);

	g_key_file_set_double(kf, CONFIG_GROUP, "mul", cfg->mul);
	g_key_file_set_string(kf, CONFIG_GROUP, "sym", cfg->sym ? cfg->sym : "px");
	g_key_file_set_integer(kf, CONFIG_GROUP, "dec", cfg->dec);
	g_key_file_set_integer(kf, CONFIG_GROUP, "divs", cfg->divs);
	g_key_file_set_integer(kf, CONFIG_GROUP, "divm", cfg->divm);
	g_key_file_set_integer(kf, CONFIG_GROUP, "ref", cfg->ref);

	g_autoptr(GError) err = NULL;
	if (!g_key_file_save_to_file(kf, path, &err))
		g_warning("Unable to save config: %s", err->message);
}
