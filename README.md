# pavol-dunst
A keybindable application to modify volume levels on audio sinks within PulseAudio with dunst-centric libnotify integrations to display performant volume notifications in [dunst](https://github.com/dunst-project/dunst/). This was partially inspired by [pavol](https://github.com/dturing/pavol) and [pavolume](https://github.com/andornaut/pavolume), as well as my previous attempt of this application [pavolumenotify](https://github.com/m-bartlett/pavolumenotify).

<p align="center">
<img src="https://user-images.githubusercontent.com/85039141/159210149-61b51c1f-6674-4aaf-9589-3f7906c8c507.gif">
<br/><sub>on-screen keyboard input display software is <a href="https://github.com/critiqjo/key-mon">key-mon</a></sub>
</p>
<br/><br/><br/>
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
  <sub>These color schemes are achieved using this application's <a href="#xresource-support">Xresource support</a> and my project <a href="https://github.com/m-bartlett/wpal">wpal</a> which generates color palettes from the desktop wallpaper and exports the colors to Xresources.</sub>
</p>

## Install
Succinctly:
```sh
cd ./src/
make && sudo make install
````

`make` will execute a multi-threaded build by default; alternatively run `make pavol-dunst`.

If you do not have `sudo` privileges then modify the `PREFIX` variable, for example: `PREFIX=$HOME/.local make install`

## Usage

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

For extra usage details please see the
[Shared Memory Singleton-Process Mutex Lock](#shared-memory-singleton-process-mutex-lock),
[PulseAudio Support](#pulseaudio-support),
and
[Xresource Support](#xresource-support)
sections below.


## About

### Key Binding
This will obviously depend on your Linux distribution, desktop environment, and preferred means of creating global keyboard shortcuts.

For i3wm I use the following in my i3 configuration file:

```php
bindsym XF86AudioRaiseVolume exec --no-startup-id pavol-dunst -v +5
bindsym XF86AudioLowerVolume exec --no-startup-id pavol-dunst -v -5
bindsym XF86AudioMute        exec --no-startup-id pavol-dunst -m toggle
```

### Custom Icons
In theory this application supports adding custom icons, however this was not intended as a first-class feature. Consequently, there is no minification pipeline to reduce the embedded SVG string body sizes. This application renders icon colors dynamically by implementing CSS classes that get passed to the RSVG rendering backend. To reduce compiled binary size, these CSS classes are not named verbosely. One may find the raw CSS stylesheet string that gets passed to the icon rendering in [`svg.cpp`](src/svg.cpp), however a more semantically expressive version follows:
```css
* { --primary: #fff; --secondary: #888; } /* librsvg doesn't support var() in stylesheet rendering, this is just for explanation */
.A { fill: var(--primary); stroke:none }  /* class A is "fill this path with the primary color */
/* Currently no icons need filled with the secondary color, but I would use class B to represent that */
.a { stroke: var(--primary); }            /* class "a" is "stroke this path with the primary color" */
.b { stroke: var(--secondary); }          /* class "b" is "stroke this path with the secondary color" */
.a,.b {stroke-width:.378; fill:none}      /* these styles apply to any strokes (irrelvant to fill styles) */
```

As such your custom icons should implement classes `A`, `a`, and `b` to with the current code to render properly with the application's color arguments.

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
As one might expect, this application changes the volume of the current default audio sink in PulseAudio. Emphasis on "the current default audio sink". If you have multiple sinks registered this application may not modify the volume of the audio source you are expecting. This is currently a limitation and the user will need to modify the default sink using a different application (probably [`pavucontrol`](https://freedesktop.org/software/pulseaudio/pavucontrol/)).

### Xresource Support
This application supports reading the CSS colors for the SVG icon renderring from [Xresources](https://wiki.archlinux.org/title/X_resources). The user may also provide color changing arguments via the `--primary` and `--secondary` flags. The user can see which Xresource keys are most currently expected in [Xresources.h](src/Xresources.h). As of writing the application will query for the following keys:

- pavol-dunst.primaryColor: _valid CSS color_
- pavol-dunst.secondaryColor: _valid CSS color_

For example the user may test this feature with:
```sh
xrdb -merge <(echo -e "pavol-dunst.primaryColor: #f00 \n pavol-dunst.primaryColor: #0ff")
```


### Shared Memory Singleton-Process Mutex Lock
This application uses shared memory to set a process mutually-exclusive (mutex) lock. This is simply to prevent jumpy audio level fluctuations caused by several instances being spawned in short succession. This may be due to either to the oparting system scheduling the processes in an unexpected order or the audio sink having a large latency. For example, a user may easily reproduce this behavior by holding down a keyboard key that this process is bound to, while the audio sink is a bluetooth device&mdash;the processes execute and queue a volume change faster than the bluetooth sink can receive and acknowledge the volume change, leading to unpredicatble volume level "relapsing".

If the process unexpectedly exits due to an unforeseen error, this single-process lock memory might be locked and not unlocked due to the unexpected exit. If you see the error message `Process mutex locked, dying.` when no other iteration of this process is running, then use the `--unlock` a.k.a. `-u` flag to forcibly unlock the process mutex. This flag may also be provided to each invocation of this application to disable the single-process mutex lock feature entirely.

### Developer Notes

When scaling the rendered SVG to GdkPixBuf&mdash;especially in the context of creating an icon for libnotify&mdash;it seemed obvious to me that the appropriate function to reference the allocated pixbuf would be `rsvg_handle_get_pixbuf` using the RSVG handle containing the rendered graphic. However this function failed to produce a re-scaled image, i.e. the resulting icon was always whatever the intrinsic document scale was despite passing in differing viewbox values. As a workaround, I found that rendering to the cairo surface directly and then producing the pixbuf from the cairo surface was successful in producing a re-scaled image. In short, using the pixbuf from `gdk_pixbuf_get_from_surface` as the notification icon was sufficient to enable dynamic image scaling. This required linking the gdk main library.
