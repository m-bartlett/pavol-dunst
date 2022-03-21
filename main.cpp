#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h> // EXIT_*
#include <stdio.h>  // printf
#include <string.h> // strcpy

#include "userdata.h"
#include "notification.h"
#include "pulseaudio.h"
#include "processmutex.h"
#include "Xresources.h"

#define DEFAULT_NOTIFICATION_TIMEOUT 1500
#define DEFAULT_ICON_SIZE 64

static int usage(char *argv[]) {
  fprintf(stderr, "%s [-h] [-m [on|off|toggle] [-v [+|-]number]\n", argv[0]);
  return pulseaudio_quit(EXIT_FAILURE);
}

static bool parse_volume_argument(char *optarg, userdata_t &userdata) {
  userdata.volume = (int) strtol(optarg, NULL, 10);
  if (userdata.volume == 0 && strcmp(optarg, "0") != 0) {
    // strtol() defaults to 0 on error. If string isn't literally '0', assume there's an error.
    return false;
  }
  if (optarg[0] == '-' || optarg[0] == '+') userdata.volume_delta = true;
  return true;
}


int main(int argc, char *argv[]) {
  userdata_t userdata = { .volume               = -1,
                          .new_volume           = -1,
                          .volume_delta         = false,
                          .mute                 = MUTE_UNKNOWN,
                          .notification_timeout = DEFAULT_NOTIFICATION_TIMEOUT,
                          .notification_body    = (char*)"",
                          .icon_primary_color   = NULL,
                          .icon_secondary_color = NULL,
                          .icon_size            = DEFAULT_ICON_SIZE };
  bool process_mutex = true;

  /*
    struct option = { name, has_argument, flag, return_val}
    has_argument:
      - no_argument (or 0) if no arg
      - required_argument (or 1) if the option requires an argument
      - optional_argument (or 2) if the option takes an optional argument.
    flag: specifies how results are returned for a long option.
      - NULL: getopt_long() returns val (e.g. set val to the equivalent short option character.)
      - else: getopt_long() returns 0, and flag points to a variable which is set to val if the
         option is found, but left unchanged if the option is not found.
    return_val: value to return, or to load into the variable pointed to by flag.
  */

  static const struct option long_options[] = {
    {"help",            no_argument,       /*flag=*/NULL, /*return_val=*/'h'},
    {"mute",            required_argument, /*flag=*/NULL, /*return_val=*/'m'},
    {"volume",          required_argument, /*flag=*/NULL, /*return_val=*/'v'},
    {"body",            required_argument, /*flag=*/NULL, /*return_val=*/'b'},
    {"timeout",         required_argument, /*flag=*/NULL, /*return_val=*/'t'},
    {"unlock",          required_argument, /*flag=*/NULL, /*return_val=*/'u'},
    {"primary-color",   required_argument, /*flag=*/NULL, /*return_val=*/'P'},
    {"secondary-color", required_argument, /*flag=*/NULL, /*return_val=*/'S'},
    {"icon-size",       required_argument, /*flag=*/NULL, /*return_val=*/'I'},
    {NULL,     0, NULL, 0}  // null entry indicates end of option array
  };

  int opt;
  while ((opt = getopt_long(argc, argv, "-huv:m:b:t:P:S:I:", long_options, NULL) ) != -1) {
    switch (opt) {

      case 'h'/*elp*/: return usage(argv);

      case 'm'/*ute*/:
        if      (strcmp("toggle", optarg) == 0) userdata.mute=MUTE_TOGGLE;
        else if (strcmp("-1",     optarg) == 0) userdata.mute=MUTE_TOGGLE;
        else if (strcmp("off",    optarg) == 0) userdata.mute=MUTE_OFF;
        else if (strcmp("0",      optarg) == 0) userdata.mute=MUTE_OFF;
        else if (strcmp("on",     optarg) == 0) userdata.mute=MUTE_ON;
        else if (strcmp("1",      optarg) == 0) userdata.mute=MUTE_ON;
        if (userdata.mute == MUTE_UNKNOWN) return usage(argv);
        break;

      case 't'/*imeout*/:
        userdata.notification_timeout = (int) strtol(optarg, NULL, 10);
        break;

      case 'b'/*ody*/:
        userdata.notification_body = optarg;
        break;

      case 'v'/*olume*/:
        if (!parse_volume_argument(optarg, userdata)) return usage(argv);
        break;

      case 'u'/*nlock*/:
        process_mutex = false;
        break;

      case 'P'/*rimary*/:
        userdata.icon_primary_color = optarg;
        break;

      case 'S'/*econdary*/:
        userdata.icon_secondary_color = optarg;
        break;

      case 'I'/*con sIze*/:
        userdata.icon_size = atof(optarg);
        if (userdata.icon_size <= 0) userdata.icon_size = DEFAULT_ICON_SIZE;
        break;

      default:
        // Positional argument, parse as volume modification
        if (!parse_volume_argument(optarg, userdata)) return usage(argv);
    }
  }

  for (char **positional = &argv[optind]; *positional; positional++) {
    // Positional arguments proceeding '--', also parse these as volume modifications
    if (!parse_volume_argument(*positional, userdata)) return usage(argv);
  }

  // Lock process mutex
  if (process_mutex) process_mutex_lock();

  read_from_Xresources(&userdata);


  mainloop = pa_mainloop_new();
  if (!mainloop) {
    fprintf(stderr, "Could not create PulseAudio main loop\n");
    process_mutex_unlock();
    return pulseaudio_quit(EXIT_FAILURE);
  }

  mainloop_api = pa_mainloop_get_api(mainloop);
  if (pa_signal_init(mainloop_api) != 0) {
    fprintf(stderr, "Could not initialize PulseAudio UNIX signal subsystem\n");
    process_mutex_unlock();
    return pulseaudio_quit(EXIT_FAILURE);
  }

  context = pa_context_new(mainloop_api, argv[0]);
  if (!context || pulseaudio_init_context(context, pa_retval) != 0) {
    fprintf(stderr, "Could not initialize PulseAudio context\n");
    process_mutex_unlock();
    return pulseaudio_quit(EXIT_FAILURE);
  }

  char *default_sink_name[256];
  wait_loop(pa_context_get_server_info(context, get_server_info_callback, &default_sink_name));
  wait_loop(pa_context_get_sink_info_by_name( context,
                                              (char *) default_sink_name,
                                              set_volume_callback,
                                              &userdata ) );
  display_volume_notification(&userdata);

  process_mutex_unlock();

  return pulseaudio_quit(pa_retval);
}