#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <libnotify/notify.h>
#include "userdata.h"

#define NOTIFICATION_LITERAL_BODY_FORMAT      "<span>%s</span>"
#define NOTIFICATION_LITERAL_CATEGORY         "volume"
#define NOTIFICATION_LITERAL_HINT_SYNCHRONOUS "synchronous"
#define NOTIFICATION_LITERAL_HINT_VALUE       "value"

#define ICON_FILE_PREFIX        "audio-volume-"
#define ICON_FILE_SUFFIX        "-symbolic.svg"
#define ICON_FILE_MUTED         ICON_FILE_PREFIX "muted"         ICON_FILE_SUFFIX
#define ICON_FILE_LOW           ICON_FILE_PREFIX "low"           ICON_FILE_SUFFIX
#define ICON_FILE_MEDIUM        ICON_FILE_PREFIX "medium"        ICON_FILE_SUFFIX
#define ICON_FILE_HIGH          ICON_FILE_PREFIX "high"          ICON_FILE_SUFFIX
#define ICON_FILE_OVERAMPLIFIED ICON_FILE_PREFIX "overamplified" ICON_FILE_SUFFIX

extern const char* NOTIFICATION_BODY_FORMAT;
extern const int   NOTIFICATION_BODY_FORMAT_SIZE;

void display_volume_notification(userdata_t *userdata);

#endif