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

#define ICON_PATH_VOLUME_MUTED         "/usr/share/icons/Papirus-Dark/symbolic/status/audio-volume-muted-symbolic.svg"
#define ICON_PATH_VOLUME_LOW           "/usr/share/icons/Papirus-Dark/symbolic/status/audio-volume-low-symbolic.svg"
#define ICON_PATH_VOLUME_MEDIUM        "/usr/share/icons/Papirus-Dark/symbolic/status/audio-volume-medium-symbolic.svg"
#define ICON_PATH_VOLUME_HIGH          "/usr/share/icons/Papirus-Dark/symbolic/status/audio-volume-high-symbolic.svg"
#define ICON_PATH_VOLUME_OVERAMPLIFIED "/usr/share/icons/Papirus-Dark/symbolic/status/audio-volume-overamplified-symbolic.svg"

#define NOTIFICATION_BODY_WIDTH_MIN  2  // Width of notification body whitespace for non-overamplified volumes 0-100
#define NOTIFICATION_BODY_WIDTH_MAX  10 // Increase width on overamplification
#define PULSEAUDIO_OVERAMPLIFIED_MAX 150
#define PULSEAUDIO_OVERAMPLIFIED_RANGE (PULSEAUDIO_OVERAMPLIFIED_MAX - 100)
#define NOTIFICATION_TIMEOUT_MS 1000

#define NOTIFICATION_BODY_WIDTH_DELTA (NOTIFICATION_BODY_WIDTH_MAX - NOTIFICATION_BODY_WIDTH_MIN)
#define PULSEAUDIO_OVERAMPLIFIED_MAX_OFFSET (PULSEAUDIO_OVERAMPLIFIED_MAX - 100)
#define NOTIFICATION_CATEGORY_LITERAL    "volume"
#define NOTIFICATION_BODY_FORMAT_LITERAL "<span>%s</span>"
#define SYNCHRONOUS_LITERAL              "synchronous"
#define NOTIFICATION_HINT_VALUE_LITERAL  "value"


const char *NOTIFICATION_BODY_FORMAT = NOTIFICATION_BODY_FORMAT_LITERAL;
size_t NOTIFICATION_BODY_FORMAT_LENGTH = strlen(NOTIFICATION_BODY_FORMAT);


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
  char *icon_muted;
  char *icon_low;
  char *icon_medium;
  char *icon_high;
  char *icon_overamplified;
} Command;


static void wait_loop(pa_operation *op) {
  while (pa_operation_get_state(op) == PA_OPERATION_RUNNING)
    if (pa_mainloop_iterate(mainloop, 1, &retval) < 0) break;
  pa_operation_unref(op);
}

static int constrain_volume(int volume) { if (volume < 0) return 0; return volume; }
static int normalize(pa_volume_t volume) { return (int) round(volume * 100.0 / PA_VOLUME_NORM); }
static pa_volume_t denormalize(int volume) {
  return (pa_volume_t) round(volume * PA_VOLUME_NORM / 100);
}


static void set_volume( pa_context *c,
                        const pa_sink_info *i,
                        __attribute__((unused)) int eol,
                        void *userdata ) {
  if (i == NULL) return;

  Command *command = (Command *) userdata;
  if (command->is_mute_on)     pa_context_set_sink_mute_by_index(c, i->index, 1, NULL, NULL);
  if (command->is_mute_off)    pa_context_set_sink_mute_by_index(c, i->index, 0, NULL, NULL);
  if (command->is_mute_toggle) pa_context_set_sink_mute_by_index(c, i->index, !i->mute, NULL, NULL);
  if (command->volume == -1 && !command->is_delta_volume) return;

  // Turn muting off on any volume change, unless muting was specifically turned on or toggled.
  // if (!command->is_mute_on && !command->is_mute_toggle)
    // pa_context_set_sink_mute_by_index(c, i->index, 0, NULL, NULL);

  pa_cvolume *cvolume = (pa_cvolume*)&i->volume;
  int old_volume = normalize(pa_cvolume_avg(cvolume));
  int new_volume = command->is_delta_volume ? old_volume + command->volume : command->volume;

  if (new_volume > PULSEAUDIO_OVERAMPLIFIED_MAX) new_volume = PULSEAUDIO_OVERAMPLIFIED_MAX;

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


static void display_volume_notification( __attribute__((unused)) pa_context *c,
                                         const pa_sink_info *i,
                                         __attribute__((unused)) int eol,
                                         void *userdata ) {
  if (i == NULL) return;

  Command *command = (Command *) userdata;
  char output[4] = "---";
  auto volume = normalize(pa_cvolume_avg(&(i->volume)));
  if (!i->mute) snprintf(output, 4, "%d", volume);
  printf(command->format, output);
  printf("\n");
  fflush(stdout);

  uint8_t body_width = NOTIFICATION_BODY_WIDTH_MIN;

  if (volume > 100) {
    body_width += ( (volume - 100) * NOTIFICATION_BODY_WIDTH_DELTA ) / PULSEAUDIO_OVERAMPLIFIED_MAX_OFFSET;
  }

  char body_space[body_width+1];
  memset(body_space, ' ', body_width);
  body_space[body_width] = '\0';
  body_width += NOTIFICATION_BODY_FORMAT_LENGTH;
  char body[body_width+1];
  sprintf(body, NOTIFICATION_BODY_FORMAT, body_space);
  body[body_width]='\0';

  char *summary = (char*)NOTIFICATION_CATEGORY_LITERAL;
  char *icon;

  if (i->mute) icon = (char*)command->icon_muted;
  else if (volume > 100) {
    icon = command->icon_overamplified;
    volume = (volume % 100) * 100 / PULSEAUDIO_OVERAMPLIFIED_RANGE;
  }
  else {
    uint8_t loudness = volume/33;
    switch(loudness) {
      case 0:  icon = (char*)command->icon_low;    break;
      case 1:  icon = (char*)command->icon_medium; break;
      case 2:
      default: icon = (char*)command->icon_high;   break;
    }
  }

  notify_init(summary);
  NotifyNotification *notification = notify_notification_new(summary, body, icon);
  notify_notification_set_category(notification, NOTIFICATION_CATEGORY_LITERAL);
  notify_notification_set_timeout(notification, NOTIFICATION_TIMEOUT_MS);

  /*
    https://people.gnome.org/~desrt/glib-docs/glib-GVariant.html
    info on GVariants since notify_notification_set_hint_* methods are deprecated
  */

  // Set notification synchronous (overwrite existing notification instead of making a fresh one)
  notify_notification_set_hint( notification,
                                SYNCHRONOUS_LITERAL,
                                g_variant_new_string(NOTIFICATION_CATEGORY_LITERAL) );

  // Add 'value' hint for dunst progress bar to show new volume
  notify_notification_set_hint( notification,
                                NOTIFICATION_HINT_VALUE_LITERAL,
                                g_variant_new_int32(volume) );
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


static int usage(char *argv[]) {
  fprintf(stderr, "%s [-h] [-m [on|off|toggle] [-v [+|-]number]\n", argv[0]);
  return quit(EXIT_FAILURE);
}

static bool parse_volume_argument(char *optarg, struct Command &command) {
  command.volume = (int) strtol(optarg, NULL, 10);
  if (command.volume == 0 && '0' != optarg[0]) {
    // `strtol` defaults to 0 if it errors. If the string isn't literally '0', assume there's an error.
    return FALSE;
  }
  if (optarg[0] == '-' || optarg[0] == '+') command.is_delta_volume = true;
  return TRUE;
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
    .icon_muted         = (char*)ICON_PATH_VOLUME_MUTED,
    .icon_low           = (char*)ICON_PATH_VOLUME_LOW,
    .icon_medium        = (char*)ICON_PATH_VOLUME_MEDIUM,
    .icon_high          = (char*)ICON_PATH_VOLUME_HIGH,
    .icon_overamplified = (char*)ICON_PATH_VOLUME_OVERAMPLIFIED,
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
    {"help",               /*has_arg=*/no_argument,       /*flag=*/NULL, /*return_val=*/'h'},
    {"mute",               /*has_arg=*/required_argument, /*flag=*/NULL, /*return_val=*/'m'},
    {"volume",             /*has_arg=*/required_argument, /*flag=*/NULL, /*return_val=*/'v'},
    {"icon-mute",          /*has_arg=*/required_argument, /*flag=*/NULL, /*return_val=*/'X'},
    {"icon-low",           /*has_arg=*/required_argument, /*flag=*/NULL, /*return_val=*/'L'},
    {"icon-medium",        /*has_arg=*/required_argument, /*flag=*/NULL, /*return_val=*/'M'},
    {"icon-high",          /*has_arg=*/required_argument, /*flag=*/NULL, /*return_val=*/'H'},
    {"icon-overamplified", /*has_arg=*/required_argument, /*flag=*/NULL, /*return_val=*/'O'},
    {NULL,     0, NULL, 0}  // Must have a null entry to terminate the array
  };

  int opt;
  while ((opt = getopt_long(argc, argv, "-hv:m:X:L:M:H:O:", long_options, NULL) ) != -1) {
    switch (opt) {

      case 'h'/*elp*/: return usage(argv);

      case 'm'/*ute*/:
        command.is_mute_off = strcmp("off", optarg) == 0;
        command.is_mute_on = strcmp("on", optarg) == 0;
        command.is_mute_toggle = strcmp("toggle", optarg) == 0;
        if (!(command.is_mute_off || command.is_mute_on || command.is_mute_toggle)) return usage(argv);
        break;

      case 'v'/*olume*/:
        if (!parse_volume_argument(optarg, command)) return usage(argv);
        break;

      case 'X': command.icon_muted         = optarg; break;
      case 'L': command.icon_low           = optarg; break;
      case 'M': command.icon_medium        = optarg; break;
      case 'H': command.icon_high          = optarg; break;
      case 'O': command.icon_overamplified = optarg; break;

      default:
        // Positional argument, parse as volume modification
        if (!parse_volume_argument(optarg, command)) return usage(argv);
    }
  }

  for (char **positional = &argv[optind]; *positional; positional++) {
    // Positional arguments proceeding '--', also parse these as volume modifications
    if (!parse_volume_argument(*positional, command)) return usage(argv);
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
  int new_volume;
  wait_loop(pa_context_get_server_info(context, get_server_info, &default_sink_name));
  wait_loop(pa_context_get_sink_info_by_name( context,
                                              (char *) default_sink_name,
                                              set_volume,
                                              &command ) );
  wait_loop(pa_context_get_sink_info_by_name( context,
                                              (char *) default_sink_name,
                                              display_volume_notification,
                                              &command ) );
  return quit(retval);
}
