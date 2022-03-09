#ifndef XRESOURCES_H
#define XRESOURCES_H

#include <xcb/xcb.h>
#include <xcb/xcb_xrm.h>

void  Xresource_init();
void  Xresource_close();
char* Xresource_get(char* key);


#define XRESOURCE_KEY_ICON_PATH          "pavol-dunst.iconPath"
#define XRESOURCE_KEY_ICON_SIZE          "pavol-dunst.iconSize"
#define XRESOURCE_KEY_ICON_MUTED         "pavol-dunst.iconMuted"
#define XRESOURCE_KEY_ICON_LOW           "pavol-dunst.iconLow"
#define XRESOURCE_KEY_ICON_MEDIUM        "pavol-dunst.iconMedium"
#define XRESOURCE_KEY_ICON_HIGH          "pavol-dunst.iconHigh"
#define XRESOURCE_KEY_ICON_OVERAMPLIFIED "pavol-dunst.iconOveramplified"


#endif