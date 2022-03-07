// #include "Xresources.h"

// void read_xresource_value(char *key, char* retval) {

//   Display *display = XOpenDisplay(NULL);
//   if (!display) { fprintf(stderr, "Error opening display\n"); exit(EXIT_FAILURE); }

//   char *resource_manager;
//   char *type;
//   XrmDatabase db;
//   XrmValue ret;
//   XrmInitialize();
//   resource_manager = XResourceManagerString(display);
//   if (resource_manager == NULL) {
//     fprintf(stderr, "Error loading resource manager\n");
//     exit(EXIT_FAILURE);
//   }

//   db = XrmGetStringDatabase(resource_manager);

//   #define XRESOURCE_LOAD_STRING(NAME, DST) \
//     XrmGetResource(db, NAME, NAME, &type, &ret); \
//     if (ret.addr != NULL && !strncmp("String", type, 64)) DST = (char*)ret.addr;

//   XRESOURCE_LOAD_STRING(XRESOURCE_ICON_THEME_PATH, icon_theme_path);

//   printf("%s = %s\n", XRESOURCE_ICON_THEME_PATH, icon_theme_path);
// }