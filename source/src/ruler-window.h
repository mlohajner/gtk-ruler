#pragma once

#include <gtk/gtk.h>
#include "config.h"

/* MainRulerWindow */

typedef struct {
	GtkWidget *window;
	GtkWidget *drawing_area;
	RulerConfig *config;

/* blobals */
	gint      drag_button;
	gint      drag_start_x, drag_start_y;
	gboolean  scaling_x, scaling_y;
	gboolean  panning_x, panning_y;

/* key states */
	gboolean  key_shift, key_ctrl;
	gboolean  key_left, key_right, key_up, key_down;
	gboolean  key_plus, key_minus, key_h, key_w;
	gboolean  color_pick_active;
	GdkRGBA   saved_color;
	gdouble   saved_opacity;

/* window-relative mouse position */
	gint      mouse_x, mouse_y;

/* 50ms interval timer */
	guint     tick_timer_id;

/* window drag detection */
	gboolean possible_window_drag;
	gint window_drag_start_x;
	gint window_drag_start_y;
	guint window_drag_button;
} RulerWindow;

RulerWindow *ruler_window_new(GtkApplication *app);
void ruler_window_free(RulerWindow *rw);

void ruler_window_redraw(RulerWindow *rw);
