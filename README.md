# pavol-dunst
A fork of [pavol](https://github.com/dturing/pavol) and [pavolume](https://github.com/andornaut/pavolume) with dunst-centric libnotify integrations to display performant volume notifications in [dunst](https://github.com/dunst-project/dunst/).

![demo](https://user-images.githubusercontent.com/85039141/143789483-ebb2ee4f-c4d4-48c2-9476-1b6da7ea4bb7.gif)

<sub>on-screen keyboard input display software is [key-mon](https://github.com/critiqjo/key-mon)</sub>

## About

### Custom Icons

### PulseAudio Support

### Xresource Support

### Shared Memory Single-Process Mutex Lock
This application uses shared memory to set a process mutually-exclusive (mutex) lock. This is simply to prevent jumpy audio level fluctuations caused by several instances being spawned in short succession. This may be due to either to the oparting system scheduling the processes in an unexpected order or the audio sink having a large latency. For example, a user may easily reproduce this behavior by holding down a keyboard key that this process is bound to, while the audio sink is a bluetooth device&mdash;the processes execute and queue a volume change faster than the bluetooth sink can receive and acknowledge the volume change, leading to unpredicatble volume level "relapsing".

If the process unexpectedly exits due to an unforeseen error, this single-process lock memory might be locked and not unlocked due to the unexpected exit. If you see the error message `Process mutex locked, dying.` when no other iteration of this process is running, then use the `--unlock` a.k.a. `-u` flag to forcibly unlock the process mutex. This flag may also be provided to each invocation of this application to disable the single-process mutex lock feature entirely.

### Developer Notes

When scaling the rendered SVG to GdkPixBuf&mdash;especially in the context of creating an icon for libnotify&mdash;it seemed obvious to me that the appropriate function to reference the allocated pixbuf would be `rsvg_handle_get_pixbuf` using the RSVG handle containing the rendered graphic. However this function failed to produce a re-scaled image, i.e. the resulting icon was always whatever the intrinsic document scale was despite passing in differing viewbox values. As a workaround, I found that rendering to the cairo surface directly and then producing the pixbuf from the cairo surface was successful in producing a re-scaled image. In short, using the pixbuf from `gdk_pixbuf_get_from_surface` as the notification icon was sufficient to enable dynamic image scaling. This required linking the gdk main library.