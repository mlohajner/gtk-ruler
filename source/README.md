# Ruler — Source

Native GTK3 + X11 on-screen ruler for Linux, written in C.

## History

Ruler is a Linux port of a much older tool: a small on-screen measuring utility originally written in **Visual Basic**, in **Visual Studio**, back in the early 2000s. That original was a simple Windows desktop app — sit on top of the screen, measure things, get out of the way.

This is a full rewrite, not a translation of the old VB code. The logic, event handling, and rendering were rebuilt from scratch in C on top of GTK3/X11, but the goal stayed the same: a small, single-purpose ruler that's always at hand.

## Stack

| | |
|---|---|
| Language | C |
| Toolkit | GTK3 |
| Display / screen access | X11 (Xlib) — overlay window, pixel color sampling, multi-monitor handling |
| UI design | [Cambalache](https://gitlab.gnome.org/jpu/cambalache) |
| Build system | Meson + Ninja |
| Packaging | RPM spec for Fedora (`packaging/fedora/`) |

## UI design with Cambalache

The options/config dialog isn't hand-written XML — it's built visually in **Cambalache**, GNOME's GTK UI designer, and exported to GtkBuilder `.ui`:

- `data/options-window.cmb` — the Cambalache project file, edit this in Cambalache
- `data/options-window.ui` — the generated GtkBuilder XML, loaded at runtime via `data/ruler.gresource.xml`

If you want to change the options window layout, open the `.cmb` file in Cambalache rather than editing the `.ui` by hand — regenerate the `.ui` from there so both stay in sync.

## Source layout

```
source/
├── data/
│   ├── options-window.cmb     # Cambalache project (edit this)
│   ├── options-window.ui      # generated GtkBuilder UI
│   └── ruler.gresource.xml    # GResource bundle definition
├── src/
│   ├── main.c                 # entry point
│   ├── ruler-window.c/.h      # the ruler overlay itself: drawing, drag/resize, keyboard input
│   ├── options-window.c/.h    # config dialog logic, backed by the Cambalache-built .ui
│   ├── x11-screen.c/.h        # X11/Xlib glue: screen geometry, multi-monitor, pixel color sampling
│   └── config.c/.h            # persisted settings (size, colors, toggles, units)
├── meson.build
└── LICENSE
```

## Build

```bash
meson setup build
ninja -C build
./build/ruler
```

Dependencies (Debian/Ubuntu): `libgtk-3-dev libx11-dev meson ninja-build`
Dependencies (Fedora): `sudo dnf builddep packaging/fedora/ruler.spec`

For end-user install instructions and an RPM build, see the [top-level README](../README.md).
