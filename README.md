# pavol-dunst
A fork of [pavol](https://github.com/dturing/pavol) and [pavolume](https://github.com/andornaut/pavolume) with dunst-centric libnotify integrations to display performant volume notifications

![demo](https://user-images.githubusercontent.com/85039141/143789483-ebb2ee4f-c4d4-48c2-9476-1b6da7ea4bb7.gif)

<sub>on-screen keyboard input display software is [key-mon](https://github.com/critiqjo/key-mon)</sub>

### Developer Notes

When scaling the rendered SVG to GdkPixBuf&mdash;especially in the context of creating an icon for libnotify&mdash;it seemed obvious to me that the appropriate function to reference the allocated pixbuf would be `rsvg_handle_get_pixbuf` using the RSVG handle containing the rendered graphic. However this function failed to produce a re-scaled image, i.e. the resulting icon was always whatever the intrinsic document scale was despite passing in differing viewbox values. As a workaround, I found that rendering to the cairo surface directly and then producing the pixbuf from the cairo surface was successful in producing a re-scaled image. In short, using the pixbuf from `gdk_pixbuf_get_from_surface` as the notification icon was sufficient to enable dynamic image scaling. This required linking the gdk main library.