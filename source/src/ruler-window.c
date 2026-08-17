#include "ruler-window.h"
#include "x11-screen.h"
#include "options-window.h"
#include <math.h>
#include <string.h>

static void
apply_rgba_visual(GtkWidget *widget)
{
	GdkScreen *screen = gtk_widget_get_screen(widget);
	GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
	if (visual && gdk_screen_is_composited(screen))
		gtk_widget_set_visual(widget, visual);
	else
		g_warning("ruler-window: Compositor not active, transparency not available!");
}

static gchar *
format_measure(gdouble px, gfloat mul, gint dec, const gchar *sym)
{
	return g_strdup_printf("%.*f%s", dec, px * mul, sym);
}

/* ---- draw handle ------------------------------ */

static gboolean
on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	RulerWindow *rw = user_data;
	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);

	RulerConfig *cfg = rw->config;

/* ruler color */
	cairo_set_source_rgba(cr, cfg->back_color.red, cfg->back_color.green,
						cfg->back_color.blue, 1.0);
	cairo_paint(cr);

	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_set_line_width(cr, 1.0);

	gint w = alloc.width;
	gint h = alloc.height;
	gint area = cfg->area;

/* referent corner (origin point) */
	gdouble ox = (cfg->ref == 0 || cfg->ref == 1) ? 0 : w;
	gdouble oy = (cfg->ref == 0 || cfg->ref == 3) ? 0 : h;
	gdouble fx = w - ox;
	gdouble fy = h - oy;

	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	gdouble size = fmax(10.0, sqrt((gdouble)w * h) / 30.0);
	cairo_set_font_size(cr, size);

/* show division markers (opt8) */
	if (cfg->options & RULER_OPT_SHOW_GRADATION) {
		gdouble kx = (gdouble)w / cfg->divs;
		gdouble ky = (gdouble)h / cfg->divs;

/* origin indikator */
		{
			gdouble dx = (ox == 0) ? 1.0 : -1.0;
			gdouble dy = (oy == 0) ? 1.0 : -1.0;
			cairo_move_to(cr, ox, oy + dy * area * 2);
			cairo_line_to(cr, ox + dx * area * 2, oy);
			cairo_stroke(cr);
		}

		for (int i = 1; i < cfg->divs; i++) {
			if (kx > 5) {
				cairo_move_to(cr, i * kx, 0);
				cairo_line_to(cr, i * kx, area);
				cairo_move_to(cr, i * kx, h);
				cairo_line_to(cr, i * kx, h - area);
			}
			if (ky > 5) {
				cairo_move_to(cr, 0, i * ky);
				cairo_line_to(cr, area, i * ky);
				cairo_move_to(cr, w, i * ky);
				cairo_line_to(cr, w - area, i * ky);
			}
		}
		cairo_stroke(cr);

/* division markers */
		if (cfg->divm > 0) {
			for (int i = 1; i < cfg->divs; i++) {
				g_autofree gchar *label = g_strdup_printf("%d", i * cfg->divm);
				if (kx > 20) {
					cairo_move_to(cr, i * kx - size / 1.5, area + size);
					cairo_show_text(cr, label);
				}
				if (ky > 20) {
					cairo_move_to(cr, area + size / 8, i * ky + size / 2);
					cairo_show_text(cr, label);
				}
			}
		}
	}

/* mouse coordinates */
	gint mx = rw->mouse_x;
	gint my = rw->mouse_y;

	gboolean hair_x = (cfg->options & RULER_OPT_HAIRLINE_X) != 0;
	gboolean hair_y = (cfg->options & RULER_OPT_HAIRLINE_Y) != 0;

/* measure from origin to mouse */
	gdouble width_measure	= (ox == 0) ? mx : (w - mx);
	gdouble height_measure = (oy == 0) ? my : (h - my);

/* hairline crte (opt64/opt256) */
	if (hair_x) {
		cairo_move_to(cr, mx, oy);
		cairo_line_to(cr, mx, my);
	}
	if (hair_y) {
		cairo_move_to(cr, ox, my);
		cairo_line_to(cr, mx, my);
	}
	if (hair_x || hair_y)
		cairo_stroke(cr);

/* area measurement (hairline) vs (total) */
	gdouble area_px = hair_x ? (width_measure * height_measure * cfg->mul * cfg->mul) : ((gdouble)w * h);

	gdouble width_val  = hair_x ? width_measure  : (gdouble)w;
	gdouble height_val = hair_y ? height_measure : (gdouble)h;

/* dijagonal measurement */
	gboolean show_diagonal_value = FALSE;
	gdouble dia_px = 0;
	if (hair_x && hair_y) {
		dia_px = sqrt(width_measure * width_measure + height_measure * height_measure);
		show_diagonal_value = TRUE;

		if ((cfg->options & RULER_OPT_SHOW_DIAGONAL) && show_diagonal_value) {
			g_autofree gchar *dia_txt = format_measure(dia_px, cfg->mul, cfg->dec, cfg->sym);

			cairo_move_to(cr, ox, oy);
			cairo_line_to(cr, mx, my);
			cairo_stroke(cr);

			gdouble dx = mx - ox;
			gdouble dy = my - oy;
			gdouble angle = atan2(dy, dx);
			gdouble tpos = 0.6;
/* Keep text upright */
			if (angle > G_PI / 2.0) {
				angle -= G_PI;
				tpos= 0.8;
			}
			if (angle < -G_PI / 2.0) {
				angle += G_PI;
				tpos= 0.8;
			}
			
			gdouble tx = ox + tpos * dx;
			gdouble ty = oy + tpos * dy - size / 2.0;
			
			cairo_save(cr);
			cairo_translate(cr, tx, ty);
			cairo_rotate(cr, angle);
			cairo_move_to(cr, 0, 0);
			cairo_show_text(cr, dia_txt);
			cairo_restore(cr);
		}
	} else {
		dia_px = sqrt((gdouble)w * w + (gdouble)h * h);
		show_diagonal_value = TRUE;

		if ((cfg->options & RULER_OPT_SHOW_DIAGONAL) && show_diagonal_value) {
			g_autofree gchar *dia_txt = format_measure(dia_px, cfg->mul, cfg->dec, cfg->sym);

			gdouble lx0 = ox + 0.75 * (fx - ox);
			gdouble ly0 = oy + 0.75 * (fy - oy);
			cairo_move_to(cr, lx0, ly0);
			cairo_line_to(cr, fx, fy);
			cairo_stroke(cr);

			gdouble tx = ox + 0.8 * (fx - ox);
			gdouble ty = oy + 0.8 * (fy - oy) - size;
			gdouble angle_sign = ((fx - ox) * (fy - oy) >= 0) ? 1.0 : -1.0;
			gdouble angle = angle_sign * atan2((gdouble)h, (gdouble)w);

			cairo_save(cr);
			cairo_translate(cr, tx, ty);
			cairo_rotate(cr, angle);
			cairo_move_to(cr, 0, 0);
			cairo_show_text(cr, dia_txt);
			cairo_restore(cr);
		}
	}

/* dimensions in the center (opt32) */
	if (cfg->options & RULER_OPT_SHOW_CENTER) {
		g_autofree gchar *wtxt = format_measure(width_val, cfg->mul, cfg->dec, cfg->sym);
		g_autofree gchar *htxt = format_measure(height_val, cfg->mul, cfg->dec, cfg->sym);
		g_autofree gchar *text = g_strdup_printf("width: %s\nheight: %s", wtxt, htxt);

		if (cfg->options & RULER_OPT_SHOW_AREA) {
			const gchar *prefix = "";
			gint decimals = cfg->dec;
			
			if (area_px >= 1000000.0) {
				prefix = "M";
				decimals = 3;
				area_px /= 1000000.0;
			} else if (area_px >= 1000.0) {
				prefix = "k";
				decimals = 2;
				area_px /= 1000.0;
			}
			g_autofree gchar *area_str = g_strdup_printf("\narea: %.*f%s%s",
								decimals, area_px, prefix, cfg->sym);
			gchar *combined = g_strconcat(text, area_str, NULL);
			g_free(text);
			text = combined;
		}

		if (cfg->color_pick_str && *cfg->color_pick_str) {
			gchar *combined = g_strconcat(text, "\n", cfg->color_pick_str, NULL);
			g_free(text);
			text = combined;
		}

		gchar **lines = g_strsplit(text, "\n", -1);
		gdouble ty = h / 2.0 - 20;
		for (gchar **l = lines; *l; l++) {
			cairo_move_to(cr, w / 2.0 - 40, ty);
			cairo_show_text(cr, *l);
			ty += size + 4;
		}
		g_strfreev(lines);
	}

/* outer width and height (opt16) */
	if (cfg->options & RULER_OPT_SHOW_EDGE_DIMS) {
		g_autofree gchar *wtxt = format_measure(w, cfg->mul, cfg->dec, cfg->sym);
		g_autofree gchar *htxt = format_measure(h, cfg->mul, cfg->dec, cfg->sym);

		cairo_text_extents_t ext;
		cairo_text_extents(cr, wtxt, &ext);
		cairo_move_to(cr, w / 2.0 - ext.width / 2.0, h - area * 1.5);
		cairo_show_text(cr, wtxt);

		cairo_save(cr);
		cairo_text_extents(cr, htxt, &ext);
		cairo_translate(cr, w - area * 1.5, h / 2.0 + ext.width / 2.0);
		cairo_rotate(cr, -G_PI / 2.0);
		cairo_move_to(cr, 0, 0);
		cairo_show_text(cr, htxt);
		cairo_restore(cr);
	}

	return FALSE;
}

void
ruler_window_redraw(RulerWindow *rw)
{
	gtk_widget_queue_draw(rw->drawing_area);
}

/* ---- Mouse buttons handling ------- */

static gboolean
on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	RulerWindow *rw = user_data;

	GtkAllocation alloc;
	gtk_widget_get_allocation(widget, &alloc);

	if(event->button == GDK_BUTTON_PRIMARY) {
		if (event->type == GDK_2BUTTON_PRESS) {
			GdkDisplay *display = gtk_widget_get_display(rw->window);
			GdkWindow *gwin = gtk_widget_get_window(rw->window);

			GdkMonitor *monitor = NULL;

			if (gwin)
				monitor = gdk_display_get_monitor_at_window(display, gwin);

			if (monitor) {
				GdkRectangle geom;
				gdk_monitor_get_workarea(monitor, &geom);
				gtk_window_move(GTK_WINDOW(rw->window), geom.x, geom.y);

/* Avoid exact monitor height -some X11 WMs treat it as maximized. */
				gtk_window_resize(GTK_WINDOW(rw->window),
								  geom.width,
								  geom.height - 1);
			}
			return TRUE;
		}
		else {
			if (!(rw->scaling_x || rw->scaling_y ||
				  rw->panning_x || rw->panning_y)) {

				rw->possible_window_drag = TRUE;
				rw->window_drag_start_x = event->x_root;
				rw->window_drag_start_y = event->y_root;
				rw->window_drag_button = event->button;

				return TRUE;
			}
		}
	}

	if (rw->drag_button == 0) {
		rw->drag_start_x = (gint)event->x_root;
		rw->drag_start_y = (gint)event->y_root;
		rw->drag_button = event->button;
		rw->scaling_x = event->x > (alloc.width - rw->config->area);
		rw->scaling_y = event->y > (alloc.height - rw->config->area);
		rw->panning_x = event->x < rw->config->area;
		rw->panning_y = event->y < rw->config->area;
	}

/* secondary click display options */
	if (event->button == GDK_BUTTON_SECONDARY)
		options_window_show(rw);

	return TRUE;
}

static gboolean
on_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	RulerWindow *rw = user_data;
	(void)widget; (void)event;
	rw->drag_button = 0;
	rw->possible_window_drag = FALSE;
	rw->window_drag_button = 0;
	return TRUE;
}

static gboolean
on_motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
	RulerWindow *rw = user_data;

	if (rw->possible_window_drag &&
		rw->window_drag_button == GDK_BUTTON_PRIMARY) {

		gint dx = event->x_root - rw->window_drag_start_x;
		gint dy = event->y_root - rw->window_drag_start_y;

		const gint threshold = 4;

		if ((dx * dx + dy * dy) >= threshold * threshold) {

			rw->possible_window_drag = FALSE;

			GdkWindow *gwin = gtk_widget_get_window(rw->window);

			if (gwin) {
				gdk_window_begin_move_drag(
					gwin,
					rw->window_drag_button,
					event->x_root,
					event->y_root,
					GDK_CURRENT_TIME);
			}

			return TRUE;
		}
	}

	GtkWindow *win = GTK_WINDOW(rw->window);
	gint wx, wy, ww, wh;
	gtk_window_get_position(win, &wx, &wy);
	gtk_window_get_size(win, &ww, &wh);

	if (rw->drag_button == GDK_BUTTON_PRIMARY) {
		gint dx = (gint)event->x_root - rw->drag_start_x;
		gint dy = (gint)event->y_root - rw->drag_start_y;

		if (rw->scaling_x || rw->scaling_y || rw->panning_x || rw->panning_y) {
			gint new_x = wx;
			gint new_y = wy;
			gint new_w = ww;
			gint new_h = wh;

			/* horizontal edge */
			if (rw->scaling_x) {
				new_w += dx;
			}
			else if (rw->panning_x) {
				new_x += dx;
				new_w -= dx;
			}

			/* vertical edge */
			if (rw->scaling_y) {
				new_h += dy;
			}
			else if (rw->panning_y) {
				new_y += dy;
				new_h -= dy;
			}

			new_w = MAX(20, new_w);
			new_h = MAX(20, new_h);

			if (new_x != wx || new_y != wy)
				gtk_window_move(win, new_x, new_y);

			if (new_w != ww || new_h != wh)
				gtk_window_resize(win, new_w, new_h);

			rw->config->aspect = (gdouble)new_w / new_h;
		}
		rw->drag_start_x = (gint)event->x_root;
		rw->drag_start_y = (gint)event->y_root;
	} else {
		GtkAllocation alloc;
		gtk_widget_get_allocation(widget, &alloc);
		rw->scaling_x = event->x > (alloc.width - rw->config->area);
		rw->scaling_y = event->y > (alloc.height - rw->config->area);
		rw->panning_x = event->x < rw->config->area;
		rw->panning_y = event->y < rw->config->area;

		GdkCursorType cursor_type = GDK_LEFT_PTR;
		if (rw->panning_x && rw->panning_y)
			cursor_type = GDK_TOP_LEFT_CORNER;
		else if (rw->scaling_x && rw->panning_y)
			cursor_type = GDK_TOP_RIGHT_CORNER;
		else if (rw->panning_x && rw->scaling_y)
			cursor_type = GDK_BOTTOM_LEFT_CORNER;
		else if (rw->scaling_x && rw->scaling_y)
			cursor_type = GDK_BOTTOM_RIGHT_CORNER;
		else if ((rw->scaling_x || rw->panning_x) &&
				 !(rw->scaling_y || rw->panning_y))
			cursor_type = GDK_SB_H_DOUBLE_ARROW;
		else if ((rw->scaling_y || rw->panning_y) &&
				 !(rw->scaling_x || rw->panning_x))
			cursor_type = GDK_SB_V_DOUBLE_ARROW;

		GdkWindow *gdk_win = gtk_widget_get_window(widget);
		if (gdk_win) {
			GdkCursor *cursor = gdk_cursor_new_for_display(gdk_display_get_default(), cursor_type);
			gdk_window_set_cursor(gdk_win, cursor);
			g_object_unref(cursor);
		}
	}

/* update and clamp mouse position */
	{
		GtkAllocation alloc;
		gtk_widget_get_allocation(widget, &alloc);
		rw->mouse_x = CLAMP((gint)event->x, 0, alloc.width - 1);
		rw->mouse_y = CLAMP((gint)event->y, 0, alloc.height - 1);
	}

	ruler_window_redraw(rw);
	return TRUE;
}

static gboolean
on_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
	RulerWindow *rw = user_data;
	(void)widget;
	gint dd = rw->key_ctrl ? 5 : 1;
	if (event->direction == GDK_SCROLL_SMOOTH) {
		if (event->delta_y > 0)
			dd = -dd;
	} else {
		if (event->direction == GDK_SCROLL_DOWN)
			dd = -dd;
	}

	gint ww, wh;
	gtk_window_get_size(GTK_WINDOW(rw->window), &ww, &wh);

	if (rw->key_shift) {
		gdouble op = gtk_widget_get_opacity(rw->window);
		gtk_widget_set_opacity(rw->window, CLAMP(op + dd / 100.0, 0.05, 1.0));
	} else if (rw->key_w) {
		gtk_window_resize(GTK_WINDOW(rw->window), MAX(20, ww + dd), wh);
		rw->config->aspect = (gdouble)ww / wh;
	} else if (rw->key_h) {
		gtk_window_resize(GTK_WINDOW(rw->window), ww, MAX(20, wh + dd));
		rw->config->aspect = (gdouble)ww / wh;
	} else {
		gint new_h = MAX(20, wh + dd);
		gtk_window_resize(GTK_WINDOW(rw->window), (gint)(new_h * rw->config->aspect), new_h);
	}
	return TRUE;
}

/* ---- Keyboard handle ----------------------------- */

static gboolean
on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	RulerWindow *rw = user_data;
	(void)widget;

	switch (event->keyval) {
		case GDK_KEY_Shift_L: case GDK_KEY_Shift_R: rw->key_shift = TRUE; break;
		case GDK_KEY_Control_L: case GDK_KEY_Control_R: rw->key_ctrl = TRUE; break;
		case GDK_KEY_Left: rw->key_left = TRUE; break;
		case GDK_KEY_Right: rw->key_right = TRUE; break;
		case GDK_KEY_Up: rw->key_up = TRUE; break;
		case GDK_KEY_Down: rw->key_down = TRUE; break;
		case GDK_KEY_KP_Add: case GDK_KEY_plus: rw->key_plus = TRUE; break;
		case GDK_KEY_KP_Subtract: case GDK_KEY_minus: rw->key_minus = TRUE; break;
		case GDK_KEY_h: case GDK_KEY_H: rw->key_h = TRUE; break;
		case GDK_KEY_w: case GDK_KEY_W: rw->key_w = TRUE; break;
		case GDK_KEY_c: case GDK_KEY_C:
			if (!rw->color_pick_active) {
				rw->saved_color = rw->config->back_color;
				rw->saved_opacity = gtk_widget_get_opacity(rw->window);
				gtk_widget_set_opacity(rw->window, 1.0);
			}
			rw->color_pick_active = TRUE;
			break;
		default: break;
	}
	return TRUE;
}

static gboolean
on_key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	RulerWindow *rw = user_data;
	(void)widget;

	switch (event->keyval) {
		case GDK_KEY_Shift_L: case GDK_KEY_Shift_R: rw->key_shift = FALSE; break;
		case GDK_KEY_Control_L: case GDK_KEY_Control_R: rw->key_ctrl = FALSE; break;
		case GDK_KEY_Left: rw->key_left = FALSE; break;
		case GDK_KEY_Right: rw->key_right = FALSE; break;
		case GDK_KEY_Up: rw->key_up = FALSE; break;
		case GDK_KEY_Down: rw->key_down = FALSE; break;
		case GDK_KEY_KP_Add: case GDK_KEY_plus: rw->key_plus = FALSE; break;
		case GDK_KEY_KP_Subtract: case GDK_KEY_minus: rw->key_minus = FALSE; break;
		case GDK_KEY_h: case GDK_KEY_H: rw->key_h = FALSE; break;
		case GDK_KEY_w: case GDK_KEY_W: rw->key_w = FALSE; break;
		case GDK_KEY_Return: case GDK_KEY_KP_Enter: {
/* copy-to-clipboard (CSS format, respects hairline vs global size) */
			gint ww, wh;
			gtk_window_get_size(GTK_WINDOW(rw->window), &ww, &wh);

			RulerConfig *cfg = rw->config;

/* isti izracun ishodista (ox/oy) kao u on_draw */
			gdouble ox = (cfg->ref == 0 || cfg->ref == 1) ? 0 : ww;
			gdouble oy = (cfg->ref == 0 || cfg->ref == 3) ? 0 : wh;

			gdouble width_measure  = (ox == 0) ? rw->mouse_x : (ww - rw->mouse_x);
			gdouble height_measure = (oy == 0) ? rw->mouse_y : (wh - rw->mouse_y);

			gboolean hair_x = (cfg->options & RULER_OPT_HAIRLINE_X) != 0;
			gboolean hair_y = (cfg->options & RULER_OPT_HAIRLINE_Y) != 0;

/* hairline ukljucen -> mjera do misa, inace -> globalna velicina */
			gdouble width_val  = hair_x ? width_measure  : (gdouble)ww;
			gdouble height_val = hair_y ? height_measure : (gdouble)wh;

			g_autofree gchar *wtxt = format_measure(width_val, cfg->mul, cfg->dec, cfg->sym);
			g_autofree gchar *htxt = format_measure(height_val, cfg->mul, cfg->dec, cfg->sym);

/* stvarno ocitana boja (color_pick_str), ne pozadina prozora */
			const gchar *color_hex = "#000000";
			if (cfg->color_pick_str) {
				const gchar *hash = strchr(cfg->color_pick_str, '#');
				if (hash)
					color_hex = hash;
			}

			g_autofree gchar *clip_text = g_strdup_printf(
				"width: %s;\nheight: %s;\ncolor: %s;",
				wtxt, htxt, color_hex);

			GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
			gtk_clipboard_set_text(clipboard, clip_text, -1);
			break;
		}
		case GDK_KEY_c: case GDK_KEY_C:
			if (rw->color_pick_active && (rw->config->options & RULER_OPT_TEMP_COLOR)) {
				rw->config->back_color = rw->saved_color;
				gtk_widget_set_opacity(rw->window, rw->saved_opacity);
			}
			rw->color_pick_active = FALSE;
			break;
		case GDK_KEY_r: case GDK_KEY_R: {
			gint ww, wh;
			gtk_window_get_size(GTK_WINDOW(rw->window), &ww, &wh);
			gtk_window_resize(GTK_WINDOW(rw->window), wh, ww);
			rw->config->aspect = (gdouble)wh / ww;
			break;
		}
		case GDK_KEY_o: case GDK_KEY_O:
			rw->config->ref = (rw->config->ref + 1) % 4;
			ruler_window_redraw(rw);
			break;
		case GDK_KEY_Escape:
			gtk_window_close(GTK_WINDOW(rw->window));
			break;
		default: break;
	}
	return TRUE;
}

/* ---- periodic tick ---------------------- */

static gboolean
on_tick(gpointer user_data)
{
	RulerWindow *rw = user_data;
	gint dd = rw->key_ctrl ? 5 : 1;
	gint wx, wy, ww, wh;
	gtk_window_get_position(GTK_WINDOW(rw->window), &wx, &wy);
	gtk_window_get_size(GTK_WINDOW(rw->window), &ww, &wh);
	gboolean moved = FALSE;

	if (rw->key_left)	{ if (rw->key_shift) { ww -= dd; moved = TRUE; } else { wx -= dd; moved = TRUE; } }
	if (rw->key_right) { if (rw->key_shift) { ww += dd; moved = TRUE; } else { wx += dd; moved = TRUE; } }
	if (rw->key_up)		{ if (rw->key_shift) { wh -= dd; moved = TRUE; } else { wy -= dd; moved = TRUE; } }
	if (rw->key_down)	{ if (rw->key_shift) { wh += dd; moved = TRUE; } else { wy += dd; moved = TRUE; } }
	if (rw->key_plus)	{ wh += dd; ww = (gint)(wh * rw->config->aspect); moved = TRUE; }
	if (rw->key_minus) { wh -= dd; ww = (gint)(wh * rw->config->aspect); moved = TRUE; }

	if (moved) {
		gtk_window_move(GTK_WINDOW(rw->window), wx, wy);
		gtk_window_resize(GTK_WINDOW(rw->window), MAX(20, ww), MAX(20, wh));
	}

	if (rw->color_pick_active) {
		gint px, py;
		if (x11_screen_get_pointer(&px, &py)) {
			GdkRGBA color;
			if (x11_screen_get_pixel_color(px, py, &color)) {
				rw->config->back_color = color;
				g_free(rw->config->color_pick_str);
				rw->config->color_pick_str = g_strdup_printf(
					"color: #%02X%02X%02X",
					(int)(color.red * 255), (int)(color.green * 255), (int)(color.blue * 255));
			}
		}
		ruler_window_redraw(rw);
	}

	return G_SOURCE_CONTINUE;
}

/* ---- window lifecycle --------------------------------------------- */

static gboolean
on_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	RulerWindow *rw = user_data;
	(void)widget; (void)event;

	gtk_window_get_position(GTK_WINDOW(rw->window), &rw->config->window_x, &rw->config->window_y);
	gtk_window_get_size(GTK_WINDOW(rw->window), &rw->config->window_width, &rw->config->window_height);
	rw->config->opacity = gtk_widget_get_opacity(rw->window);
	ruler_config_save(rw->config);

	return FALSE;
}

RulerWindow *
ruler_window_new(GtkApplication *app)
{
	RulerWindow *rw = g_new0(RulerWindow, 1);
	rw->config = ruler_config_new_defaults();
	ruler_config_load(rw->config);

	rw->window = gtk_application_window_new(app);
	gtk_window_set_decorated(GTK_WINDOW(rw->window), FALSE);
	gtk_window_set_keep_above(GTK_WINDOW(rw->window), TRUE);
	gtk_widget_set_app_paintable(rw->window, TRUE);

	gint pos_x = rw->config->window_x;
	gint pos_y = rw->config->window_y;

	if (rw->config->options & RULER_OPT_OPEN_AT_CURSOR) {
		gint px, py;
		if (x11_screen_get_pointer(&px, &py)) {
			pos_x = px - rw->config->window_width / 2;
			pos_y = py - rw->config->window_height / 2;
		}
	}

	gtk_window_move(GTK_WINDOW(rw->window), pos_x, pos_y);
	gtk_window_resize(GTK_WINDOW(rw->window), rw->config->window_width, rw->config->window_height);
	gtk_widget_set_opacity(rw->window, rw->config->opacity);

	apply_rgba_visual(rw->window);

	rw->drawing_area = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(rw->window), rw->drawing_area);

	gtk_widget_add_events(rw->window,
		GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
		GDK_SCROLL_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

	g_signal_connect(rw->drawing_area, "draw", G_CALLBACK(on_draw), rw);
	g_signal_connect(rw->window, "button-press-event", G_CALLBACK(on_button_press), rw);
	g_signal_connect(rw->window, "button-release-event", G_CALLBACK(on_button_release), rw);
	g_signal_connect(rw->window, "motion-notify-event", G_CALLBACK(on_motion_notify), rw);
	g_signal_connect(rw->window, "scroll-event", G_CALLBACK(on_scroll), rw);
	g_signal_connect(rw->window, "key-press-event", G_CALLBACK(on_key_press), rw);
	g_signal_connect(rw->window, "key-release-event", G_CALLBACK(on_key_release), rw);
	g_signal_connect(rw->window, "delete-event", G_CALLBACK(on_delete_event), rw);

	rw->tick_timer_id = g_timeout_add(50, on_tick, rw);

	gtk_widget_show_all(rw->window);
	return rw;
}

void
ruler_window_free(RulerWindow *rw)
{
	if (!rw)
		return;
	if (rw->tick_timer_id)
		g_source_remove(rw->tick_timer_id);
	ruler_config_free(rw->config);
	g_free(rw);
}
