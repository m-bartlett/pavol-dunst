#include <string.h>
#include <math.h>
#include "pulseaudio.h"
#include "userdata.h"

pa_mainloop     *mainloop     = NULL;
pa_mainloop_api *mainloop_api = NULL;
pa_context      *context      = NULL;
int             pa_retval     = EXIT_SUCCESS;

const unsigned short PULSEAUDIO_OVERAMPLIFIED_RANGE = PULSEAUDIO_OVERAMPLIFIED_MAX - 100;


int pulseaudio_init_context(pa_context *context, int pa_retval) {
  pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL);
  pa_context_state_t state;
  while (state = pa_context_get_state(context), true) {
    if (state == PA_CONTEXT_READY)  return 0;
    if (state == PA_CONTEXT_FAILED) return 1;
    pa_mainloop_iterate(mainloop, 1, &pa_retval);
  }
  return 0;
}


int pulseaudio_quit(int new_pa_retval) {
  // Set pa_retval if not changed elsewhere (e.g. by pa_mainloop_iterate()).
  if (pa_retval == EXIT_SUCCESS) pa_retval = new_pa_retval;
  if (context) pa_context_unref(context);
  if (mainloop_api) mainloop_api->quit(mainloop_api, pa_retval);
  if (mainloop) {
    pa_signal_done();
    pa_mainloop_free(mainloop);
  }
  return pa_retval;
}


void pa_wait_loop(pa_operation *op) {
  while (pa_operation_get_state(op) == PA_OPERATION_RUNNING)
    if (pa_mainloop_iterate(mainloop, 1, &pa_retval) < 0) break;
  pa_operation_unref(op);
}


int constrain_volume(int volume) {
  if (volume < 0) return 0; return volume;
}


int normalize(pa_volume_t volume) {
  return (int) round(volume * 100.0 / PA_VOLUME_NORM);
}


pa_volume_t denormalize(int volume) {
  return (pa_volume_t) round(volume * PA_VOLUME_NORM / 100);
}


void callback__set_volume( pa_context *context,
                          const pa_sink_info *sink_info,
                          __attribute__((unused)) int eol,
                          void *pulseaudio_userdata ) {
  if (sink_info == NULL) return;

  userdata_t *userdata = (userdata_t *) pulseaudio_userdata;

  switch (userdata->mute) {
    case MUTE_ON:
      pa_context_set_sink_mute_by_index(context, sink_info->index, 1, NULL, NULL);
      break;
    case MUTE_OFF:
      pa_context_set_sink_mute_by_index(context, sink_info->index, 0, NULL, NULL);
      break;
    case MUTE_TOGGLE: {
      bool new_mute_state = !sink_info->mute;
      pa_context_set_sink_mute_by_index(context, sink_info->index, new_mute_state, NULL, NULL);
      userdata->mute = new_mute_state ? MUTE_ON : MUTE_OFF;
      break;
    }
    case MUTE_UNKNOWN:
      userdata->mute = sink_info->mute ? MUTE_ON : MUTE_OFF;
      break;
  }

  // /* Turn muting off on any volume change, unless muting was specifically turned on or toggled. */
  // if (!userdata->is_mute_on && !userdata->is_mute_toggle)
    // pa_context_set_sink_mute_by_index(context, sink_info->index, 0, NULL, NULL);

  pa_cvolume *cvolume = (pa_cvolume*)&sink_info->volume;
  int old_volume = normalize(pa_cvolume_avg(cvolume));

  /* Return current volume for display if no volume argument was provided
     Setting an absolute negative volume would be undefined behavior anyway. */
  if (userdata->volume < 0 && !userdata->volume_delta) {
    userdata->new_volume = old_volume;
    return;
  }

  int new_volume;
  if (userdata->volume_delta) new_volume = old_volume + userdata->volume;
  else                        new_volume = userdata->volume;

  if (new_volume > PULSEAUDIO_OVERAMPLIFIED_MAX) new_volume = PULSEAUDIO_OVERAMPLIFIED_MAX;
  else if (new_volume < 0)                       new_volume = 0;

  userdata->new_volume = new_volume;

  pa_cvolume *new_cvolume = pa_cvolume_set( cvolume,
                                            sink_info->volume.channels,
                                            denormalize(constrain_volume(new_volume)) );
  pa_context_set_sink_volume_by_index(context, sink_info->index, new_cvolume, NULL, NULL);
}

void callback__get_server_info( __attribute__((unused)) pa_context *context,
                               const pa_server_info *sink_info,
                               void *userdata ) {
  if (sink_info == NULL) return;
  strncpy((char*)userdata, sink_info->default_sink_name, 255);
}