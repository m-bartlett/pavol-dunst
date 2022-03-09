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