#include <stdint.h>
#include "notification.h"
#include "pulseaudio.h"
#include <gio/gio.h>
#include "icons/high.h"

const char* NOTIFICATION_BODY_FORMAT = NOTIFICATION_LITERAL_BODY_FORMAT;
const int   NOTIFICATION_BODY_FORMAT_SIZE = strlen(NOTIFICATION_BODY_FORMAT) - 2;

void display_volume_notification(userdata_t *userdata) {
  int volume = userdata->new_volume;

  printf("%d\n", volume);
  fflush(stdout);

  char *summary = (char*)NOTIFICATION_LITERAL_CATEGORY;
  char *icon;

  if (userdata->mute == MUTE_ON) icon = (char*)userdata->icon_muted;
  else {
    uint8_t loudness = volume/33;
    switch(loudness) {
      case 0:  icon = (char*)userdata->icon_low;           break;
      case 1:  icon = (char*)userdata->icon_medium;        break;
      case 2:  icon = (char*)userdata->icon_high;          break;
      default:
        icon = (char*)userdata->icon_overamplified;
        volume = (volume % 100) * 100 / PULSEAUDIO_OVERAMPLIFIED_RANGE;
        break;
    }
  }


  size_t body_width = NOTIFICATION_BODY_FORMAT_SIZE + strlen(userdata->notification_body)+1;
  char body[body_width];
  sprintf(body, NOTIFICATION_BODY_FORMAT, userdata->notification_body);

  // gdk_pixbuf_new_from_inline ( -1, high, false, NULL);
  GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data ( (const guchar*) ICON_HIGH_PIXEL_DATA,
                                                 GDK_COLORSPACE_RGB,
                                                 /*has_alpha=*/true,
                                                 /*bits_per_sample=*/8,
                                                 ICON_HIGH_WIDTH,
                                                 ICON_HIGH_HEIGHT,
                                                 ICON_HIGH_ROWSTRIDE,
                                                 NULL,
                                                 NULL);




  notify_init(summary);
  // NotifyNotification *notification = notify_notification_new(summary, body, icon);
  NotifyNotification *notification = notify_notification_new(summary, body, NULL);
  notify_notification_set_category(notification, NOTIFICATION_LITERAL_CATEGORY);
  notify_notification_set_timeout(notification,  userdata->notification_timeout);
  notify_notification_set_image_from_pixbuf(notification, pixbuf);

  /* https://people.gnome.org/~desrt/glib-docs/glib-GVariant.html
     info on GVariants since notify_notification_set_hint_* methods are deprecated */

  // Set notification synchronous (overwrite existing notification instead of making a fresh one)
  notify_notification_set_hint( notification,
                                NOTIFICATION_LITERAL_HINT_SYNCHRONOUS,
                                g_variant_new_string(NOTIFICATION_LITERAL_CATEGORY) );

  // Add 'value' hint for dunst progress bar to show new volume
  notify_notification_set_hint( notification,
                                NOTIFICATION_LITERAL_HINT_VALUE,
                                g_variant_new_int32(volume) );

  notify_notification_show(notification, NULL);

  g_object_unref(notification);
  notify_uninit();
}
