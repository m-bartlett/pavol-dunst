#include "notification.h"
#include "pulseaudio.h"
#include "icons.h"
#include "svg.h"

const char* NOTIFICATION_BODY_FORMAT = NOTIFICATION_LITERAL_BODY_FORMAT;
const int   NOTIFICATION_BODY_FORMAT_SIZE = strlen(NOTIFICATION_BODY_FORMAT) - 2;

const guint8* icons[] = { (guint8*) silent_svg_raw,
                          (guint8*)    low_svg_raw,
                          (guint8*) medium_svg_raw,
                          (guint8*)   high_svg_raw };

const gsize icon_sizes[] = { (gsize) silent_svg_raw_size,
                             (gsize)    low_svg_raw_size,
                             (gsize) medium_svg_raw_size,
                             (gsize)   high_svg_raw_size };

const size_t icons_size = sizeof(icons) / sizeof(icons[0]);
const unsigned char divisor = 100 / icons_size;

void display_volume_notification(userdata_t *userdata) {
  int volume = userdata->new_volume;

  printf("%d\n", volume);
  fflush(stdout);

  char *summary = (char*)NOTIFICATION_LITERAL_CATEGORY;
  guint8* icon    = (guint8*) high_svg_raw;
  gsize icon_size = (gsize)   high_svg_raw_size;

  if (userdata->mute == MUTE_ON) {
    icon      = (guint8*) muted_svg_raw;
    icon_size = (gsize)   muted_svg_raw_size;
  }
  else {
    unsigned char loudness = volume ? (volume-1)/divisor : 0;
    if (loudness < icons_size) {
      icon      = (guint8*) icons[loudness];
      icon_size = (gsize)   icon_sizes[loudness];
    }
    else {
      icon      = (guint8*) overamplified_svg_raw;
      icon_size = (gsize)   overamplified_svg_raw_size;
      volume = (volume % 100) * 100 / PULSEAUDIO_OVERAMPLIFIED_RANGE;
    }
  }

  size_t body_width = NOTIFICATION_BODY_FORMAT_SIZE + strlen(userdata->notification_body)+1;
  char body[body_width];
  sprintf(body, NOTIFICATION_BODY_FORMAT, userdata->notification_body);

  notify_init(summary);
  NotifyNotification *notification = notify_notification_new(summary, body, NULL);
  notify_notification_set_category(notification, NOTIFICATION_LITERAL_CATEGORY);
  notify_notification_set_timeout(notification,  userdata->notification_timeout);

  bool icon_success = populate_notification_icon(notification, icon, icon_size,
                                                 userdata->icon_primary_color,
                                                 userdata->icon_secondary_color);
  if (!icon_success) {
    fprintf(stderr, "Error rendering notification icon\n" );
    exit(EXIT_FAILURE);
  }

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
