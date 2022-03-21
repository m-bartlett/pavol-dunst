#include <stdbool.h>
#include <libnotify/notify.h>
#include <librsvg/rsvg.h>
#include <cairo.h>
#include <gdk/gdk.h>


const char* icon_stylesheet_template =
  ".A{fill:%s;stroke:none}.a{stroke:%s;}.b{stroke:%s}.a,.b{stroke-width:.378;fill:none;}";

bool render_notification_icon(NotifyNotification* notification,
                              guint8*             icon_body,
                              gsize               icon_body_size,
                              double              icon_size,
                              char*               primary_color,
                              char*               secondary_color) {

  RsvgHandle *rsvg_handle = rsvg_handle_new_from_data(icon_body, icon_body_size, NULL);
  int stylesheet_size = snprintf(NULL, 0, icon_stylesheet_template,
                                 primary_color, primary_color, secondary_color);

  char stylesheet[stylesheet_size];
  sprintf(stylesheet, icon_stylesheet_template, primary_color, primary_color, secondary_color);

  bool stylesheet_status = rsvg_handle_set_stylesheet(rsvg_handle,
                                                      (const guint8*)stylesheet,
                                                      (gsize)stylesheet_size,
                                                      NULL);
  if (!stylesheet_status) return false;

  cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, icon_size, icon_size);

  cairo_t *cairo = cairo_create(surface);
  if (cairo_status(cairo) != CAIRO_STATUS_SUCCESS) return false;

  GError *error = NULL;
  const RsvgRectangle viewport = {.x=0.0, .y=0.0, .width=icon_size, .height=icon_size};
  bool render_success = rsvg_handle_render_document (rsvg_handle, cairo, &viewport, &error);
  if (!render_success) { g_printerr ("SVG render failed: %s", error->message); exit(1); }

  cairo_surface_flush(surface);

  // GdkPixbuf* pixbuf = rsvg_handle_get_pixbuf(rsvg_handle);   // <- this doesn't scale image
  GdkPixbuf* pixbuf = gdk_pixbuf_get_from_surface (surface, 0, 0, icon_size, icon_size);

  notify_notification_set_image_from_pixbuf(notification, pixbuf);

  cairo_destroy (cairo);
  cairo_surface_destroy (surface);
  g_object_unref(rsvg_handle);
  g_object_unref(pixbuf);
  return true;
}