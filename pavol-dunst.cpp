#include <getopt.h>
#include <libnotify/notify.h>
#include <locale.h>
#include <math.h>
#include <memory.h>
#include <pulse/pulseaudio.h>
#include <stdbool.h>
#include <stddef.h> // wchar_t
#include <stdio.h>
#include <string.h>
#include <uchar.h>
#include <unistd.h> // usleep
#include <wchar.h>
#include <stdint.h>

// #define VOLUME_MUTE_SYMBOL_LITERAL "ﱝ"
// // #define VOLUME_QUIET_SYMBOL_LITERAL "<span font_features=\"dlig=1, -kern, afrc on\">奄</span>"
// #define VOLUME_QUIET_SYMBOL_LITERAL ""
// #define VOLUME_NORMAL_SYMBOL_LITERAL ""
// #define VOLUME_LOUD_SYMBOL_LITERAL ""

#define VOLUME_MUTE_SYMBOL_LITERAL "🔇"
#define VOLUME_QUIET_SYMBOL_LITERAL "🔈"
#define VOLUME_NORMAL_SYMBOL_LITERAL "🔉"
#define VOLUME_LOUD_SYMBOL_LITERAL "🔊"

#define NOTIFICATION_TIMEOUT_MS 1000
#define NOTIFICATION_CATEGORY_LITERAL "volume"
#define SYNCHRONOUS_LITERAL "synchronous"
#define NOTIFICATION_HINT_VALUE_LITERAL "value"


static pa_mainloop *mainloop = NULL;
static pa_mainloop_api *mainloop_api = NULL;
static pa_context *context = NULL;
int retval = EXIT_SUCCESS;

typedef struct Command {
  char *format;
  bool is_delta_volume;
  bool is_mute_off;
  bool is_mute_on;
  bool is_mute_toggle;
  bool is_snoop;
  int volume;
} Command;


static void wait_loop(pa_operation *op) {
  while (pa_operation_get_state(op) == PA_OPERATION_RUNNING)
    if (pa_mainloop_iterate(mainloop, 1, &retval) < 0) break;
  pa_operation_unref(op);
}

static int constrain_volume(int volume) { if (volume < 0) return 0; return volume; }
static int normalize(pa_volume_t volume) { return (int) round(volume * 100.0 / PA_VOLUME_NORM); }
static pa_volume_t denormalize(int volume) { return (pa_volume_t) round(volume * PA_VOLUME_NORM / 100); }

static void
set_volume( pa_context *c,
            const pa_sink_info *i,
            __attribute__((unused)) int eol,
            void *userdata
          ) {
  if (i == NULL) return;

  Command *command = (Command *) userdata;
  if (command->is_mute_on)  pa_context_set_sink_mute_by_index(c, i->index, 1, NULL, NULL);
  if (command->is_mute_off) pa_context_set_sink_mute_by_index(c, i->index, 0, NULL, NULL);
  if (command->is_mute_toggle) pa_context_set_sink_mute_by_index(c, i->index, !i->mute, NULL, NULL);
  if (command->volume == -1 && !command->is_delta_volume) return;

  // Turn muting off on any volume change, unless muting was specifically turned on or toggled.
  // if (!command->is_mute_on && !command->is_mute_toggle)
    // pa_context_set_sink_mute_by_index(c, i->index, 0, NULL, NULL);

  pa_cvolume *cvolume = (pa_cvolume*)&i->volume;
  int new_volume = (
    command->is_delta_volume ?
    normalize(pa_cvolume_avg(cvolume)) + command->volume :
    command->volume
  );

  pa_cvolume *new_cvolume = (
    pa_cvolume_set(
      cvolume,
      i->volume.channels,
      denormalize(constrain_volume(new_volume))
    )
  );

  pa_context_set_sink_volume_by_index(c, i->index, new_cvolume, NULL, NULL);
}

static void get_server_info( __attribute__((unused)) pa_context *c,
                             const pa_server_info *i,
                             void *userdata                         ) {
  if (i == NULL) return;
  strncpy((char*)userdata, i->default_sink_name, 255);
}


static void print_volume( __attribute__((unused)) pa_context *c,
                          const pa_sink_info *i,
                          __attribute__((unused)) int eol,
                           void *userdata                        ) {
  if (i == NULL) return;

  Command *command = (Command *) userdata;
  char output[4] = "---";
  auto volume = normalize(pa_cvolume_avg(&(i->volume)));
  if (!i->mute) snprintf(output, 4, "%d", volume);
  printf(command->format, output);
  printf("\n");
  fflush(stdout);

  char *summary = (char*)NOTIFICATION_CATEGORY_LITERAL;
  char *body;

  if (i->mute) body = (char*) u8"" VOLUME_MUTE_SYMBOL_LITERAL;
  else {
    uint8_t loudness = volume/33;
    switch(loudness) {
      case 0: body = (char*) u8"" VOLUME_QUIET_SYMBOL_LITERAL; break;
      case 1: body = (char*) u8"" VOLUME_NORMAL_SYMBOL_LITERAL; break;
      case 2:
      default: body = (char*) u8"" VOLUME_LOUD_SYMBOL_LITERAL; break;
    }
  }

  notify_init(summary);
  NotifyNotification *notification = notify_notification_new(summary, body, NULL);
  notify_notification_set_category(notification, NOTIFICATION_CATEGORY_LITERAL);

  /*
    https://people.gnome.org/~desrt/glib-docs/glib-GVariant.html
    info on GVariants since notify_notification_set_hint_* methods are deprecated
  */

  // Set notification synchronous (overwrite existing notification instead of making a fresh one)
  notify_notification_set_hint(
    notification,
    SYNCHRONOUS_LITERAL,
    g_variant_new_string(NOTIFICATION_CATEGORY_LITERAL)
  );

  // Add 'value' hint for dunst progress bar to show
  notify_notification_set_hint(
    notification,
    NOTIFICATION_HINT_VALUE_LITERAL,
    g_variant_new_int32(volume)
  );

  notify_notification_set_timeout(notification, NOTIFICATION_TIMEOUT_MS);
  notify_notification_show(notification, NULL);
  g_object_unref(notification);
  notify_uninit();
}


static int
init_context(pa_context *c, int retval) {
  pa_context_connect(c, NULL, PA_CONTEXT_NOFLAGS, NULL);
  pa_context_state_t state;
  while (state = pa_context_get_state(c), true) {
    if (state == PA_CONTEXT_READY)  return 0;
    if (state == PA_CONTEXT_FAILED) return 1;
    pa_mainloop_iterate(mainloop, 1, &retval);
  }
  return 0;
}


static int
quit(int new_retval) {
  // Only set `retval` if it hasn't been changed elsewhere (such as by PulseAudio in `pa_mainloop_iterate()`).
  if (retval == EXIT_SUCCESS) retval = new_retval;
  if (context) pa_context_unref(context);
  if (mainloop_api) mainloop_api->quit(mainloop_api, retval);
  if (mainloop) {
    pa_signal_done();
    pa_mainloop_free(mainloop);
  }
  return retval;
}


static int usage() {
  fprintf(stderr, "pavolumenotify [-h] [-m [on|off|toggle] [-v [+|-]number]\n");
  return quit(EXIT_FAILURE);
}


int main(int argc, char *argv[]) {

  // setlocale(LC_CTYPE, "");
  // setlocale(LC_ALL, "en_US.UTF-8");
  setlocale(LC_ALL, "en_US.utf8");

  Command command = {
    .format = (char*)"%s",
    .is_delta_volume = false,
    .is_mute_off = false,
    .is_mute_on = false,
    .is_mute_toggle = false,
    .is_snoop = false,
    .volume = -1,
  };


  /*
    struct option = { name, has_argument, flag, return_val}
    - has_argument:
      - no_argument (or 0) if no arg
      - required_argument (or 1) if the option requires an argument
      - optional_argument (or 2) if the option takes an optional argument.
    - flag: specifies how results are returned for a long option.
      - NULL: getopt_long() returns val (e.g. set val to the equivalent short option character.)
      - else: getopt_long() returns 0, and flag points to a variable which is set to val if the option is found, but left unchanged if the option is not found.
    - return_val: value to return, or to load into the variable pointed to by flag.
  */

  static const struct option long_options[] = {
    {"help",   0, NULL, 'h'},
    {"volume", 1, NULL, 'v'},
    {"mute",   1, NULL, 'm'},
    {NULL,     0, NULL, 0}  // Must have a null entry to terminate the array
  };

  int opt;
  while ((opt = getopt_long(argc, argv, "-hm:v:", long_options, NULL) ) != -1) {
    switch (opt) {
      case 'h': // help
        return usage();
      case 'm': // mute
        command.is_mute_off = strcmp("off", optarg) == 0;
        command.is_mute_on = strcmp("on", optarg) == 0;
        command.is_mute_toggle = strcmp("toggle", optarg) == 0;
        if (!(command.is_mute_off || command.is_mute_on || command.is_mute_toggle)) {
          return usage();
        }
        break;
      case 'v': //volume
        command.volume = (int) strtol(optarg, NULL, 10);
        if (command.volume == 0 && '0' != optarg[0]) {
          // If `strtol` converted the `optarg` to 0, but the argument didn't begin with a '0'
          // then it must not have been numeric.
          return usage();
        }
        if (optarg[0] == '-' || optarg[0] == '+') command.is_delta_volume = true;
        break;
      default:
        return usage();
    }
  }

  mainloop = pa_mainloop_new();
  if (!mainloop) {
    fprintf(stderr, "Could not create PulseAudio main loop\n");
    return quit(EXIT_FAILURE);
  }

  mainloop_api = pa_mainloop_get_api(mainloop);
  if (pa_signal_init(mainloop_api) != 0) {
    fprintf(stderr, "Could not initialize PulseAudio UNIX signal subsystem\n");
    return quit(EXIT_FAILURE);
  }

  context = pa_context_new(mainloop_api, argv[0]);
  if (!context || init_context(context, retval) != 0) {
    fprintf(stderr, "Could not initialize PulseAudio context\n");
    return quit(EXIT_FAILURE);
  }

  char *default_sink_name[256];
  wait_loop(pa_context_get_server_info(context, get_server_info, &default_sink_name));
  wait_loop(pa_context_get_sink_info_by_name(context, (char *) default_sink_name, set_volume, &command));
  wait_loop(pa_context_get_sink_info_by_name(context, (char *) default_sink_name, print_volume, &command));

  return quit(retval);
}
