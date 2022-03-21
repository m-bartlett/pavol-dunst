#ifndef SVG_H
#define SVG_H

bool render_notification_icon(NotifyNotification* notification,
                              guint8*             icon_body,
                              gsize               icon_body_size,
                              double              icon_size,
                              char*               primary_color,
                              char*               secondary_color);

#endif