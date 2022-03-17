#include <libnotify/notify.h>
#include <librsvg/rsvg.h>

const char* icon_stylesheet_template =
  ".A{fill:%s;stroke:none}.a{stroke:%s;}.b{stroke:%s}.a,.b{stroke-width:.377953;fill:none;}";

bool populate_notification_icon(NotifyNotification *notification,
                                guint8* icon,
                                gsize   icon_size,
                                char*   primary_color,
                                char*   secondary_color) {

  RsvgHandle *rsvg_handle = rsvg_handle_new_from_data(icon, icon_size, NULL);
  int stylesheet_size = snprintf(NULL, 0, icon_stylesheet_template,
                                 primary_color,
                                 primary_color,
                                 secondary_color);

  char stylesheet[stylesheet_size];
  sprintf(stylesheet,
          icon_stylesheet_template,
          primary_color,
          primary_color,
          secondary_color);

  bool stylesheet_status = rsvg_handle_set_stylesheet(rsvg_handle,
                                                      (const guint8*)stylesheet,
                                                      (gsize)stylesheet_size,
                                                      NULL);
  if (!stylesheet_status) return false;

  GdkPixbuf* pixbuf = rsvg_handle_get_pixbuf(rsvg_handle);
  g_object_unref(rsvg_handle);

  notify_notification_set_image_from_pixbuf(notification, pixbuf);
  g_object_unref(pixbuf);
  return true;
}