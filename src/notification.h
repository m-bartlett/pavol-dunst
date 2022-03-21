#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <libnotify/notify.h>
#include "userdata.h"

#define NOTIFICATION_LITERAL_BODY_FORMAT      "<span>%s</span>"
#define NOTIFICATION_LITERAL_CATEGORY         "volume"
#define NOTIFICATION_LITERAL_HINT_SYNCHRONOUS "synchronous"
#define NOTIFICATION_LITERAL_HINT_VALUE       "value"

extern const char NOTIFICATION_BODY_FORMAT[];
extern const int  NOTIFICATION_BODY_FORMAT_SIZE;

void display_volume_notification(userdata_t *userdata);

#endif
