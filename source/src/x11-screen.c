#include "x11-screen.h"
#include <X11/Xlib.h>
#include <gdk/gdkx.h>

static Display *
get_x11_display(void)
{
	GdkDisplay *gdk_display = gdk_display_get_default();

	if (!GDK_IS_X11_DISPLAY(gdk_display)) {
		g_warning("x11-screen: display is not X11/XWayland, Eyedropper not available!");
		return NULL;
	}

	return gdk_x11_display_get_xdisplay(gdk_display);
}

gboolean
x11_screen_get_pointer(gint *x, gint *y)
{
	Display *dpy = get_x11_display();
	if (!dpy)
		return FALSE;

	Window root = DefaultRootWindow(dpy);
	Window root_ret, child_ret;
	int root_x, root_y, win_x, win_y;
	unsigned int mask;

	if (!XQueryPointer(dpy, root, &root_ret, &child_ret,
							&root_x, &root_y, &win_x, &win_y, &mask))
		return FALSE;

	*x = root_x;
	*y = root_y;
	return TRUE;
}

gboolean
x11_screen_get_pixel_color(gint x, gint y, GdkRGBA *out_color)
{
	Display *dpy = get_x11_display();
	if (!dpy)
		return FALSE;

	Window root = DefaultRootWindow(dpy);

/* 1x1 XGetImage on root window */
	XImage *img = XGetImage(dpy, root, x, y, 1, 1, AllPlanes, ZPixmap);
	if (!img)
		return FALSE;

	unsigned long pixel = XGetPixel(img, 0, 0);

	/* Moder 24/32-bit TrueColor forma assumed */
	out_color->red	 = ((pixel & 0xFF0000) >> 16) / 255.0;
	out_color->green = ((pixel & 0x00FF00) >> 8)	/ 255.0;
	out_color->blue	= ((pixel & 0x0000FF))			 / 255.0;
	out_color->alpha = 1.0;

	XDestroyImage(img);
	return TRUE;
}
