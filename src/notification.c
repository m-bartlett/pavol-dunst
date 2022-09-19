#include <stdbool.h>
#include "notification.h"
#include "pulseaudio.h"
#include "icons.h"
#include "svg.h"

const char NOTIFICATION_BODY_FORMAT[] = NOTIFICATION_LITERAL_BODY_FORMAT;
const int  NOTIFICATION_BODY_FORMAT_SIZE = strlen(NOTIFICATION_BODY_FORMAT) - 2;

const guint8* icon_bodies[] = { (guint8*) silent_svg_raw,
                                (guint8*)    low_svg_raw,
                                (guint8*) medium_svg_raw,
                                (guint8*)   high_svg_raw };

const gsize icon_body_sizes[] = { silent_svg_raw_size,
                                     low_svg_raw_size,
                                  medium_svg_raw_size,
                                    high_svg_raw_size };

const size_t icon_bodies_size = sizeof(icon_bodies) / sizeof(icon_bodies[0]);
const unsigned char divisor = 100 / icon_bodies_size;

void display_volume_notification(userdata_t *userdata) {
  int volume = userdata->new_volume;

  printf("%d\n", volume);
  fflush(stdout);

  char *summary        = (char*)NOTIFICATION_LITERAL_CATEGORY;
  guint8* icon_body    = (guint8*) high_svg_raw;
  gsize icon_body_size = (gsize)   high_svg_raw_size;

  if (userdata->mute == MUTE_ON) { icon_body      = (guint8*) muted_svg_raw;
                                   icon_body_size = (gsize)   muted_svg_raw_size; }
  else {
    unsigned char loudness = volume ? (volume-1)/divisor : 0;
    if (loudness < icon_bodies_size) { icon_body      = (guint8*) icon_bodies[loudness];
                                       icon_body_size = (gsize)   icon_body_sizes[loudness]; }

    else { icon_body      = (guint8*) overamplified_svg_raw;
           icon_body_size = (gsize)   overamplified_svg_raw_size;
           volume         = (volume % 100) * 100 / PULSEAUDIO_OVERAMPLIFIED_RANGE; }
  }

  #ifdef FORMAT_VOLUME_IN_NOTIFICATION_BODY

    /* Attempt to format the current volume percentage integer into the user's notification body */
    size_t formatted_body_size =strlen(userdata->notification_body) - 2/*len("%d")*/ + 3/*len("100")*/;
    char formatted_body[formatted_body_size];
    sprintf(formatted_body, userdata->notification_body, volume);

    size_t body_size = NOTIFICATION_BODY_FORMAT_SIZE + strlen(formatted_body) + 1;
    char body[body_size];
    sprintf(body, NOTIFICATION_BODY_FORMAT, formatted_body);

  #else

    size_t body_width = NOTIFICATION_BODY_FORMAT_SIZE + strlen(userdata->notification_body)+1;
    char body[body_width];
    sprintf(body, NOTIFICATION_BODY_FORMAT, userdata->notification_body, volume);

  #endif

  notify_init(summary);
  NotifyNotification *notification = notify_notification_new(summary, body, NULL);
  notify_notification_set_category(notification, NOTIFICATION_LITERAL_CATEGORY);
  notify_notification_set_timeout(notification,  userdata->notification_timeout);

  /* https://people.gnome.org/~desrt/glib-docs/glib-GVariant.html
        info on GVariants since notify_notification_set_hint_* methods are deprecated */

  /* Set notification synchronous (overwrite existing notification instead of making a fresh one) */
  notify_notification_set_hint(notification,
                               NOTIFICATION_LITERAL_HINT_STACKTAG,
                               g_variant_new_string(NOTIFICATION_LITERAL_CATEGORY));

  /* Add 'value' hint for dunst progress bar to show new volume */
  notify_notification_set_hint( notification,
                                NOTIFICATION_LITERAL_HINT_VALUE,
                                g_variant_new_int32(volume) );

  #ifdef ENABLE_TRANSIENT_HINT
  /* transient notifications will still timeout even if the user is considered idle */
  notify_notification_set_hint(notification, "transient", g_variant_new_boolean(true));
  #endif

  bool icon_success = render_notification_icon(notification,
                                               icon_body,
                                               icon_body_size,
                                               userdata->icon_size,
                                               userdata->icon_primary_color,
                                               userdata->icon_secondary_color);

  if (!icon_success) { fprintf(stderr, "Error rendering notification icon\n"); exit(EXIT_FAILURE); }

  notify_notification_show(notification, NULL);

  g_object_unref(notification);
  notify_uninit();
}
