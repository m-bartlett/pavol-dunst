#ifndef PULSEAUDIO_H
#define PULSEAUDIO_H

#include <pulse/pulseaudio.h>
#include <stdint.h>

#define PULSEAUDIO_OVERAMPLIFIED_MAX 150

const uint16_t PULSEAUDIO_OVERAMPLIFIED_RANGE = PULSEAUDIO_OVERAMPLIFIED_MAX - 100;

extern pa_mainloop *mainloop;
extern pa_mainloop_api *mainloop_api;
extern pa_context *context;
extern int pa_retval;

enum mute_t { MUTE_ON, MUTE_OFF, MUTE_TOGGLE, MUTE_UNKNOWN };

int pulseaudio_init_context(pa_context *context, int pa_retval);

int pulseaudio_quit(int new_pa_retval);

void wait_loop(pa_operation *op);

int constrain_volume(int volume);

int normalize(pa_volume_t volume);

pa_volume_t denormalize(int volume);

void set_volume_callback( pa_context *context,
                                 const pa_sink_info *sink_info,
                                 __attribute__((unused)) int eol,
                                 void *userdata );

void get_server_info_callback( __attribute__((unused)) pa_context *context,
                                      const pa_server_info *sink_info,
                                      void *userdata );

#endif