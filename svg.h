#ifndef SVG_H
#define SVG_H

bool populate_notification_icon(NotifyNotification *notification,
                                guint8* icon,
                                gsize   icon_size,
                                char*   primary_color,
                                char*   secondary_color);

#endif