#ifndef XRESOURCES_H
#define XRESOURCES_H

#include <xcb/xcb.h>
#include <xcb/xcb_xrm.h>

void  Xresource_init();
void  Xresource_close();
char* Xresource_get(char* key);


#define XRESOURCE_KEY_ICON_PRIMARY_COLOR   "pavol-dunst.primaryColor"
#define XRESOURCE_KEY_ICON_SECONDARY_COLOR "pavol-dunst.secondaryColor"

#endif