# pavol-dunst
A keybindable application to modify volume levels on audio sinks within PulseAudio with dunst-centric libnotify integrations to display volume notifications in [dunst](https://github.com/dunst-project/dunst/) performantly. This was partially inspired by [pavol](https://github.com/dturing/pavol) and [pavolume](https://github.com/andornaut/pavolume), as well as my previous attempt of this application [pavolumenotify](https://github.com/m-bartlett/pavolumenotify).

<p align="center">
<img src="https://user-images.githubusercontent.com/85039141/159210149-61b51c1f-6674-4aaf-9589-3f7906c8c507.gif">
<br/><sub><i>on-screen keyboard input display software is <a href="https://github.com/critiqjo/key-mon">key-mon</a></i></sub>
</p>
<br/><br/>
<p align="center">
  <span>
    <img
      width="250px"
      alt="Xresource color example 1"
      title="Xresource color example 1"
      src="https://user-images.githubusercontent.com/85039141/159217593-700adb57-1bd9-4d52-8660-2362159d00dd.jpg"
    >
    <img
      width="250px"
      alt="Xresource color example 2"
      title="Xresource color example 2"
      src="https://user-images.githubusercontent.com/85039141/159217603-d813ae03-9390-48ea-bc97-3f9228d2730f.jpg"
    >
    <img
      width="250px"
      alt="Xresource color example 3"
      title="Xresource color example 3"
      src="https://user-images.githubusercontent.com/85039141/159217599-6f1f9cdd-235c-4f74-a626-f631800f4d3f.jpg"
    >
  </span>
  <span>
    <img
      width="250px"
      alt="Xresource color example 4"
      title="Xresource color example 4"
      src="https://user-images.githubusercontent.com/85039141/159217600-0f7ce327-d5d1-4727-a366-38b4ea239d20.jpg"
    >
    <img
      width="250px"
      alt="Xresource color example 5"
      title="Xresource color example 5"
      src="https://user-images.githubusercontent.com/85039141/159217598-f1abcdf2-1b9c-4684-95c9-3423adda6866.jpg"
    >
    <img
      width="250px"
      alt="Xresource color example 6"
      title="Xresource color example 6"
      src="https://user-images.githubusercontent.com/85039141/159217596-2e964021-0091-4114-a83c-32a36f686391.jpg"
    >
  </span>
  <span>
    <img
      width="250px"
      alt="Xresource color example 7"
      title="Xresource color example 7"
      src="https://user-images.githubusercontent.com/85039141/159217602-a18ba988-bb28-4f97-9173-3b883b9ac10a.jpg"
    >
    <img
      width="250px"
      alt="Xresource color example 8"
      title="Xresource color example 8"
      src="https://user-images.githubusercontent.com/85039141/159217597-245293b9-d4f5-4ecc-b88f-46d555c83951.jpg"
    >
    <img
      width="250px"
      alt="Xresource color example 9"
      title="Xresourcece color example 9"
      src="https://user-images.githubusercontent.com/85039141/159217604-722ccbf2-a4e9-4696-87fe-8516196592d8.jpg"
    >
  </span>
  <br/>
  <sub>These color schemes use this application's <a href="#xresource-support">Xresource support</a> and my project <a href="https://github.com/m-bartlett/wpal">wpal</a> which exports color palettes generated from the desktop wallpaper to Xresources.</sub>
</p>

<br/><br/>

---

### Table of Contents
* [Install](#install)
* [Usage](#usage)
  * [CLI Arguments](#cli-arguments)
  * [Dunstrc config](#dunstrc-config)
  * [Extra](#extra)
* [About](#about)
  * [Optional Features](#optional-features)
  * [Key Binding](#key-binding)
  * [Custom Icons](#custom-icons)
  * [PulseAudio Support](#pulseaudio-support)
  * [Xresource Support](#xresource-support)
  * [Shared Memory Singleton-Process Mutex Lock](#shared-memory-singleton-process-mutex-lock)
  * [Developer Notes](#developer-notes)

---

<br/>

## Install
Succinctly:
```sh
cd ./src/
make && sudo make install
````

`make` will execute a multi-threaded build by default; alternatively run `make pavol-dunst`.

If you do not have `sudo` privileges then modify the `PREFIX` variable, for example: `PREFIX=$HOME/.local make install`

## Usage

### CLI Arguments

Stray positional arguments without a preceding flag will be parsed as arguments to `--volume`, e.g.
```sh
pavol-dunst 5       # == pavol-dunst -v 5
pavol-dunst +5      # == pavol-dunst -v +5
pavol-dunst -- -5   # == pavol-dunst -v -5
  # '-5' without '--' would parse as a flag '-5' and throw an error, '--' indicates no more flag args
```


See `pavol-dunst --help` for documentation on the various command flags:
```
  [-h|--help] - print this usage information and exit
  
  [-m|--mute] [ [1|"on"] | [0|"off"] | [-1|"toggle"] ]
      Mute audio if arg is "1" or "on"
      Unmute audio if arg is "0" or "off"
      Toggle audio muted if arg is "-1" or "toggle"
      
  [-v|--volume] [+|-]VAL
      If arg starts with "+" increase by VAL -> +5 is current volume + 5
      If arg starts with "-" decrease by VAL -> -7 is current volume - 7
      Set absolute VAL if neither "+" or "-" are present -> 50 sets volume to 50
      
  [-t|--timeout] MILLISECONDS - end volume notification after MILLISECONDS milliseconds.
  
  [-b|--body] BODY - set volume notification body to whatever string is provided as BODY.
  
  [-u|--unlock]
      Forcibly unlock (or prevent the locking of) the shared-memory mutex lock that prevents
      concurrent instances of this process from running.
      
  [-P|--primary-color] CSS_COLOR - set volume notification icon primary color.
      If this arg is unset it will be read from the Xresources key pavol-dunst.primaryColor
      or a default value.
      
  [-S|--secondary-color] CSS_COLOR - set volume notification icon secondary color.
      If this arg is unset it will be read from the Xresources key pavol-dunst.secondaryColor
      or a default value.
      
  [-I|--icon-size] PIXELS - render volume notification icon size to be PIXELS pixels big.
```


### Dunstrc config

The user will need to have an appropriate configuration in their dunst config, most commonly `$HOME/.config/dunst/dunstrc`

The notification generated by this binary will have the category `volume` _(this can be modified in [`notification.h`](src/notification.h) by changing `NOTIFICATION_LITERAL_CATEGORY`)_.

As of writing, I am using `dunst` version 1.8.1 which supports `hide_text` to hide the notification body, and `icon_position = top` to display the icon top and centered within the notification window.

I use the following configuration section in my `dunstrc` to achieve the notification style featured in the screenshots:
```ini
[volume]
    category="volume"   # Match criteria
    format = %b
    icon_position = top
    hide_text = yes
    alignment = center
    fullscreen = show
    history_ignore = yes
    min_icon_size=64
```

### Extra
For extra usage details please see the
[Shared Memory Singleton-Process Mutex Lock](#shared-memory-singleton-process-mutex-lock),
[PulseAudio Support](#pulseaudio-support),
and
[Xresource Support](#xresource-support)
sections below.


## About

### Optional Features
There are certain features which can be enabled at compile-time. These are features that would would add overhead to the generation and display of the notification if they were specified with a boolean CLI flag, and are generally things that users would either he interested in having all the time or never. The feature inclusion is ultimately decided by the preprocessor, but I've added some logic in the `Makefile` to make it simpler for the user to specify the inclusion of these optional features.

From the table below, export the **Feature name variable** value as a non-empty value to `make`. Note that specifying a feature variable *after* building will not indicate to `make` that it needs to rebuild; one should use the `-B` flag to force `make` to rebuild.

For example, to enable the `FORMAT_VOLUME_IN_NOTIFICATION_BODY` feature, I would recommend executing:
`FORMAT_VOLUME_IN_NOTIFICATION_BODY=1 make -B`

<br/>

<table>
    <tr align="center">
        <th>Feature name variable</th>
        <th>Feature description</th>
        <th>Example</th>
    </tr>
    <tr>
        <td><code>ENABLE_TRANSIENT_HINT</code></td>
        <td>Transient notifications will still timeout even if the user is considered idle. The default <code>dunst</code> config disables idle timeout, so only enable this if you use this dunst feature and would like volume notifications to still disappear.</td>
        <td></td>
    </tr>
    <tr>
        <td><code>FORMAT_VOLUME_IN_NOTIFICATION_BODY</code></td>
        <td>

Support formatting the volume percentage integer into the notification body. This literally runs `sprintf` with the volume integer as an argument. For example with a body argument of `~%d~` and the current volume being 25, the resulting notification body would show `~25~`. <br/>**WARNING**: This feature has no sanitation of the user provided body value. If you provide any formatter besides `%d` expect to get a segmentation fault.

</td>
        <td>
          <table>
              <tr align="center">
                  <th>Disabled</th>
              </tr>
              <tr>
                  <td>
                      <img  alt="Body NOT formatted with volume"
                            title="Body NOT formatted with volume"
                            width="220"
                            src="https://user-images.githubusercontent.com/85039141/190920774-58d4c9db-417e-4fe5-88d2-be387d1b4247.png">
                  </td>
              </tr>
              <tr align="center">
                  <th>Enabled</th>
              </tr>
              <tr>
                  <td>
                      <img  alt="Body formatted with volume"
                            title="Body formatted with volume"
                            width="220"
                            src="https://user-images.githubusercontent.com/85039141/190920714-15924138-fe49-46b5-aede-16d71d968b69.png">
                  </td>
              </tr>
          </table>
        </td>
    </tr>
</table>


### Key Binding
This will obviously depend on your Linux distribution, desktop environment, and preferred means of creating global keyboard shortcuts.

For i3wm I use the following in my i3 configuration file:

```php
bindsym XF86AudioRaiseVolume exec --no-startup-id pavol-dunst -v +5
bindsym XF86AudioLowerVolume exec --no-startup-id pavol-dunst -v -5
bindsym XF86AudioMute        exec --no-startup-id pavol-dunst -m toggle
```

### Custom Icons
In theory this application supports adding custom icons, simply modify the .svg files in [`svg/`](src/svg/). However, this was not intended as a first-class feature so there is no minification pipeline to reduce the embedded SVG string body sizes (the user will need to manually minify their SVGs). This application renders icon colors dynamically by implementing CSS classes referenced in the CSS stylesheet that get passed to the RSVG rendering backend. To reduce compiled binary size, these CSS classes are currently named with just a single character. One may find the raw CSS stylesheet string that gets passed to the icon rendering in [`svg.cpp`](src/svg.cpp), however a more semantically expressive version follows:
```css
* { --primary: #fff; --secondary: #888; } /* librsvg doesn't support var() in stylesheet rendering, this is just for explanation */
.A { fill: var(--primary); stroke:none }  /* class A is "fill this path with the primary color */
/* Currently no icons need filled with the secondary color, but I would use class B to represent that */
.a { stroke: var(--primary); }            /* class "a" is "stroke this path with the primary color" */
.b { stroke: var(--secondary); }          /* class "b" is "stroke this path with the secondary color" */
.a,.b {stroke-width:.378; fill:none}      /* these styles apply to any strokes (irrelvant to fill styles) */
```

As such your custom icons should implement classes `A`, `a`, and `b` to work with the current code to render properly with the application's color arguments.

For reference, here was a previous possible icon set I designed myself, you may use these under the same [license](LICENSE) as the rest of this repository.

<p align="center">
<span>
  <img alt="muted" title="muted" src="https://user-images.githubusercontent.com/85039141/159212762-ecfc67f3-ca3f-4410-8c98-aafd541b6420.svg">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <img alt="low" title="low" src="https://user-images.githubusercontent.com/85039141/159212766-4e8384d8-c2d4-4f18-a056-20150888b404.svg">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <img alt="medium" title="medium" src="https://user-images.githubusercontent.com/85039141/159212765-ba488320-87b3-497c-b7c8-978f0abc3dca.svg">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <img alt="high" title="high" src="https://user-images.githubusercontent.com/85039141/159212768-4408899b-36c2-4529-b1e1-7f0575296bcf.svg">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <img alt="overamplified" title="overamplified" src="https://user-images.githubusercontent.com/85039141/159212764-7a57e53a-6ff7-4013-9053-208b1ceab7fa.svg">
</span>
</p>

### PulseAudio Support
As one might expect, this application changes the volume of the current default audio sink in PulseAudio. Emphasis on "the current default audio sink". If you have multiple sinks registered this application may not modify the volume of the sound output you are expecting. This is currently a limitation and the user will need to modify the default sink using a different application (probably [`pavucontrol`](https://freedesktop.org/software/pulseaudio/pavucontrol/)).

### Xresource Support
This application supports reading the CSS colors for the SVG icon renderring from [Xresources](https://wiki.archlinux.org/title/X_resources). The user may also provide color changing arguments via the `--primary` and `--secondary` flags. The user can see which Xresource keys are most currently expected in [Xresources.h](src/Xresources.h). As of writing the application will query for the following keys:

- pavol-dunst.primaryColor: _valid CSS color_
- pavol-dunst.secondaryColor: _valid CSS color_

For example the user may test this feature with:
```sh
xrdb -merge <(echo -e "pavol-dunst.primaryColor: #f00\npavol-dunst.secondaryColor: #0ff")
```


### Shared Memory Singleton-Process Mutex Lock
This application uses shared memory to set a process mutually-exclusive (mutex) lock. This is simply to prevent jumpy audio level fluctuations caused by several instances being spawned in short succession. This may be due to either to the oparting system scheduling the processes in an unexpected order or the audio sink having a large latency. For example, a user may easily reproduce this behavior by holding down a keyboard key that this process is bound to, while the audio sink is a bluetooth device&mdash;the processes execute and queue a volume change faster than the bluetooth sink can receive and acknowledge the volume change, leading to unpredicatble volume level "relapsing".

If the process unexpectedly exits due to an unforeseen error, this single-process lock memory might be locked and not unlocked due to the unexpected exit. If you see the error message `Process mutex locked, dying.` when no other iteration of this process is running, then use the `--unlock` a.k.a. `-u` flag to forcibly unlock the process mutex. This flag may also be provided to each invocation of this application to disable the single-process mutex lock feature entirely.

### Developer Notes

- [List of all hints supported by dunst](https://dunst-project.org/documentation/#NOTIFY-SEND)

- When scaling the rendered SVG to GdkPixBuf&mdash;especially in the context of creating an icon for libnotify&mdash;it seemed obvious to me that the appropriate function to reference the allocated pixbuf would be `rsvg_handle_get_pixbuf` using the RSVG handle containing the rendered graphic. However this function failed to produce a re-scaled image, i.e. the resulting icon was always whatever the intrinsic document scale was despite passing in differing viewbox values. As a workaround, I found that rendering to the cairo surface directly and then producing the pixbuf from the cairo surface was successful in producing a re-scaled image. In short, using the pixbuf from `gdk_pixbuf_get_from_surface` as the notification icon was sufficient to enable dynamic image scaling. This requires linking the gdk main library with `$ pkg-config --libs gdk-3.0`.

- dunst 1.9.0 seems to now cache the icon image for synchronous notifications. This results in the notification icon not changing to reflect the volume magnitude visually if the process is executed again while the previous notification is still displayed despite having the image explicitly set. Recent releases of `pavol-dunst` add a workaround preventing this caching by first displaying a transparent image with the same dimensions and then updating the notification with the real image data with afterward. Caching the blank image should be easy for dunst, but the post-display update allows us to circumvent the notification caching and display the respective symbolic icon showing proportional volume level if `pavol-dunst` is executed in rapid succession

- An alternative to `notify_notification_set_image_from_pixbuf` is using the `image-data` hint supported by `dunst`. This code was informed from the [test case for the `image-data` hint](https://github.com/dunst-project/dunst/blob/1280c9a9f20f46b24b08ebc99d29a788e5256a43/test/helpers.c#L7-L35):

  ```C
  GVariant *hint_data = g_variant_new_from_data(G_VARIANT_TYPE("ay"),
                                                gdk_pixbuf_read_pixels(pixbuf),
                                                gdk_pixbuf_get_byte_length(pixbuf),
                                                TRUE,
                                                (GDestroyNotify) g_object_unref,
                                                g_object_ref(pixbuf));
  GVariant *hint = g_variant_new(
                          "(iiibii@ay)",
                          gdk_pixbuf_get_width(pixbuf),
                          gdk_pixbuf_get_height(pixbuf),
                          gdk_pixbuf_get_rowstride(pixbuf),
                          gdk_pixbuf_get_has_alpha(pixbuf),
                          gdk_pixbuf_get_bits_per_sample(pixbuf),
                          gdk_pixbuf_get_n_channels(pixbuf),
                          hint_data);

  notify_notification_set_hint(notification, "image-data", hint);
  ```
