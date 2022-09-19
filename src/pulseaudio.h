#ifndef PULSEAUDIO_H
#define PULSEAUDIO_H

#include <pulse/pulseaudio.h>

#define PULSEAUDIO_OVERAMPLIFIED_MAX 150

extern const unsigned short PULSEAUDIO_OVERAMPLIFIED_RANGE;

extern pa_mainloop *mainloop;
extern pa_mainloop_api *mainloop_api;
extern pa_context *context;
extern int pa_retval;

typedef enum mute_t { MUTE_ON, MUTE_OFF, MUTE_TOGGLE, MUTE_UNKNOWN } mute_t;

int pulseaudio_init_context(pa_context *context, int pa_retval);

int pulseaudio_quit(int new_pa_retval);

void pa_wait_loop(pa_operation *op);

int constrain_volume(int volume);

int normalize(pa_volume_t volume);

pa_volume_t denormalize(int volume);

void callback__set_volume( pa_context *context,
                          const pa_sink_info *sink_info,
                          __attribute__((unused)) int eol,
                          void *userdata );

void callback__get_server_info( __attribute__((unused)) pa_context *context,
                               const pa_server_info *sink_info,
                               void *userdata );

#endif