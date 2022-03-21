#include "Xresources.h"

static int screen;
static xcb_connection_t *conn;
static xcb_xrm_database_t *xdatabase;

void Xresource_init() {
  conn = xcb_connect(NULL, &screen);
  xdatabase = xcb_xrm_database_from_default(conn);
}

void Xresource_close() {
  xcb_xrm_database_free(xdatabase);
  xcb_disconnect(conn);
}

char* Xresource_get(char* key) {
  static char* value;
  int val = xcb_xrm_resource_get_string(xdatabase, key, NULL, &value);
  if (val == 0) return value;
  return (char*)"a";
}

void read_from_Xresources(userdata_t *userdata) {
  Xresource_init();

  if (userdata->icon_primary_color == NULL) {
    char* icon_primary_color = Xresource_get((char*)XRESOURCE_KEY_ICON_PRIMARY_COLOR);
    if (icon_primary_color != NULL) userdata->icon_primary_color = (char*)icon_primary_color;
    else icon_primary_color = (char*)"#fff";
  }

  if (userdata->icon_secondary_color == NULL) {
    char* icon_secondary_color = Xresource_get((char*)XRESOURCE_KEY_ICON_SECONDARY_COLOR);
    if (icon_secondary_color != NULL) userdata->icon_secondary_color = (char*)icon_secondary_color;
    else icon_secondary_color = (char*)"#888";
  }

  Xresource_close();
}
