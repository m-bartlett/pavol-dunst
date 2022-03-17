#ifndef USERDATA_H
#define USERDATA_H

#include "pulseaudio.h"

typedef struct userdata_t {
  int volume;
  int new_volume;
  bool volume_delta;
  mute_t mute;
  int notification_timeout;
  char *notification_body;
  char *icon_primary_color;
  char *icon_secondary_color;
} userdata_t;

#endif