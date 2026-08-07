#pragma once

#include <gtk/gtk.h>
#include <stdbool.h>

/*
 * RulerConfig persistent settings
 */

/* Bit-flag options */
typedef enum {
	RULER_OPT_OPEN_AT_CURSOR   = 1 << 0, /* open @ cursor */
	RULER_OPT_TEMP_COLOR       = 1 << 1, /* keep ruler color */
	RULER_OPT_SHOW_AREA        = 1 << 2, /* show area */
	RULER_OPT_SHOW_GRADATION   = 1 << 3, /* show divisions */
	RULER_OPT_SHOW_EDGE_DIMS   = 1 << 4, /* show main sizes */
	RULER_OPT_SHOW_CENTER      = 1 << 5, /* show size in center */
	RULER_OPT_HAIRLINE_X       = 1 << 6, /* show vertical hairline */
	RULER_OPT_SHOW_DIAGONAL    = 1 << 7, /* show dijagonal */
	RULER_OPT_HAIRLINE_Y       = 1 << 8, /* show horizontal hairline */
} RulerOption;


typedef struct {
/* --- persistent settings (in ~/.config/ruler/ruler.conf) --- */
	guint32   options;
	gint      area;
	gint      window_x;
	gint      window_y;
	gint      window_width;
	gint      window_height;
	gdouble   opacity;
	GdkRGBA   back_color;
	gfloat    mul;
	gchar    *sym;
	gint      dec;
	gint      divs;
	gint      divm;
	gint      ref;

	gdouble   aspect;
	gchar    *color_pick_str;
	gchar    *area_str;
} RulerConfig;

/* create default config */
RulerConfig *ruler_config_new_defaults(void);

void ruler_config_free(RulerConfig *cfg);

/* read ruler config */
void ruler_config_load(RulerConfig *cfg);

/* save ruler config */
void ruler_config_save(const RulerConfig *cfg);
