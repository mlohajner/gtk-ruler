#pragma once

#include <gtk/gtk.h>

/*
 * X11 access trough Xlib for:
 *  - Global mouse/cursor position
 *  - Pixel color at position
 */

/* Get global mouse cursor coordinates TRUE if successfull */
gboolean x11_screen_get_pointer(gint *x, gint *y);

/* Read pixel color from coordinates root window. */
gboolean x11_screen_get_pixel_color(gint x, gint y, GdkRGBA *out_color);
