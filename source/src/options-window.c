#include "options-window.h"
#include <stdlib.h>

/*
 * Options window, Widgets from
 * data/options-window.ui (GtkBuilder XML, Cambalache format)
 */

typedef struct {
	RulerWindow *rw;
	GtkBuilder  *builder;
} OptionsCtx;

static GtkWidget *W(GtkBuilder *b, const char *id) {
	return GTK_WIDGET(gtk_builder_get_object(b, id));
}

static GtkWidget *options_dialog = NULL;

static void
populate_from_config(OptionsCtx *ctx)
{
	GtkBuilder *b = ctx->builder;
	RulerConfig *cfg = ctx->rw->config;

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(W(b, "check_opt1")), cfg->options & RULER_OPT_OPEN_AT_CURSOR);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(W(b, "check_opt2")),   cfg->options & RULER_OPT_TEMP_COLOR);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(W(b, "check_opt4")),   cfg->options & RULER_OPT_SHOW_AREA);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(W(b, "check_opt8")),   cfg->options & RULER_OPT_SHOW_GRADATION);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(W(b, "check_opt16")),  cfg->options & RULER_OPT_SHOW_EDGE_DIMS);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(W(b, "check_opt32")),  cfg->options & RULER_OPT_SHOW_CENTER);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(W(b, "check_opt64")),  cfg->options & RULER_OPT_HAIRLINE_X);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(W(b, "check_opt128")), cfg->options & RULER_OPT_SHOW_DIAGONAL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(W(b, "check_opt256")), cfg->options & RULER_OPT_HAIRLINE_Y);

	gint ww, wh;
	gtk_window_get_size(GTK_WINDOW(ctx->rw->window), &ww, &wh);

	g_autofree gchar *w_str = g_strdup_printf("%d", ww);
	g_autofree gchar *h_str = g_strdup_printf("%d", wh);
	g_autofree gchar *area_str = g_strdup_printf("%d", cfg->area);
	g_autofree gchar *mul_str = g_strdup_printf("%g", cfg->mul);
	g_autofree gchar *dec_str = g_strdup_printf("%d", cfg->dec);
	g_autofree gchar *divs_str = g_strdup_printf("%d", cfg->divs);
	g_autofree gchar *divm_str = g_strdup_printf("%d", cfg->divm);

	gtk_entry_set_text(GTK_ENTRY(W(b, "entry_width")), w_str);
	gtk_entry_set_text(GTK_ENTRY(W(b, "entry_height")), h_str);
	gtk_entry_set_text(GTK_ENTRY(W(b, "entry_area")), area_str);
	gtk_entry_set_text(GTK_ENTRY(W(b, "entry_mul")), mul_str);
	gtk_entry_set_text(GTK_ENTRY(W(b, "entry_sym")), cfg->sym ? cfg->sym : "");
	gtk_entry_set_text(GTK_ENTRY(W(b, "entry_dec")), dec_str);
	gtk_entry_set_text(GTK_ENTRY(W(b, "entry_divs")), divs_str);
	gtk_entry_set_text(GTK_ENTRY(W(b, "entry_divm")), divm_str);

	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(W(b, "button_color")), &cfg->back_color);
}

static guint32
read_checkbox_options(GtkBuilder *b)
{
	guint32 opts = 0;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(W(b, "check_opt1"))))   opts |= RULER_OPT_OPEN_AT_CURSOR;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(W(b, "check_opt2"))))   opts |= RULER_OPT_TEMP_COLOR;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(W(b, "check_opt4"))))   opts |= RULER_OPT_SHOW_AREA;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(W(b, "check_opt8"))))   opts |= RULER_OPT_SHOW_GRADATION;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(W(b, "check_opt16"))))  opts |= RULER_OPT_SHOW_EDGE_DIMS;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(W(b, "check_opt32"))))  opts |= RULER_OPT_SHOW_CENTER;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(W(b, "check_opt64"))))  opts |= RULER_OPT_HAIRLINE_X;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(W(b, "check_opt128")))) opts |= RULER_OPT_SHOW_DIAGONAL;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(W(b, "check_opt256")))) opts |= RULER_OPT_HAIRLINE_Y;
  return opts;
}

static void
write_back_to_config(OptionsCtx *ctx)
{
	GtkBuilder *b = ctx->builder;
	RulerConfig *cfg = ctx->rw->config;

	cfg->options = read_checkbox_options(b);

	gint new_w = atoi(gtk_entry_get_text(GTK_ENTRY(W(b, "entry_width"))));
	gint new_h = atoi(gtk_entry_get_text(GTK_ENTRY(W(b, "entry_height"))));
	if (new_w > 10 && new_h > 10) {
		gtk_window_resize(GTK_WINDOW(ctx->rw->window), new_w, new_h);
		cfg->aspect = (gdouble)new_w / new_h;
	}

	cfg->area = atoi(gtk_entry_get_text(GTK_ENTRY(W(b, "entry_area"))));
	cfg->mul = (gfloat)atof(gtk_entry_get_text(GTK_ENTRY(W(b, "entry_mul"))));

	g_free(cfg->sym);
	cfg->sym = g_strdup(gtk_entry_get_text(GTK_ENTRY(W(b, "entry_sym"))));

	cfg->dec = atoi(gtk_entry_get_text(GTK_ENTRY(W(b, "entry_dec"))));
	cfg->divs = atoi(gtk_entry_get_text(GTK_ENTRY(W(b, "entry_divs"))));
	cfg->divm = atoi(gtk_entry_get_text(GTK_ENTRY(W(b, "entry_divm"))));

	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(W(b, "button_color")), &cfg->back_color);

	ruler_config_save(cfg);
	ruler_window_redraw(ctx->rw);
}

static void
on_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	OptionsCtx *ctx = user_data;

	if (response_id == GTK_RESPONSE_OK) {
		write_back_to_config(ctx);
		gtk_widget_destroy(GTK_WIDGET(dialog));
	} else if (response_id == 0) {
		gtk_widget_destroy(GTK_WIDGET(dialog));
		gtk_window_close(GTK_WINDOW(ctx->rw->window));
	} else {
		gtk_widget_destroy(GTK_WIDGET(dialog));
	}

	g_object_unref(ctx->builder);
	g_free(ctx);
}

static void
on_options_destroy(GtkWidget *widget, gpointer data)
{
	(void)widget;
	(void)data;
	options_dialog = NULL;
}

void
options_window_show(RulerWindow *rw)
{
	if (options_dialog) {
		gtk_window_present(GTK_WINDOW(options_dialog));
		return;
	}

	GtkBuilder *builder = gtk_builder_new_from_resource("/hr/manjo/ruler/options-window.ui");
	GtkWidget *dialog = W(builder, "options_dialog");
	options_dialog = dialog;
	
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(rw->window));

	OptionsCtx *ctx = g_new0(OptionsCtx, 1);
	ctx->rw = rw;
	ctx->builder = builder;

	populate_from_config(ctx);

	g_signal_connect(dialog, "response",
						G_CALLBACK(on_response), ctx);
	g_signal_connect(dialog, "destroy",
						G_CALLBACK(on_options_destroy), NULL);
	gtk_widget_show_all(dialog);
}
