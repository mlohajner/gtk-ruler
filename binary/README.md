# Binaries

This directory contains a prebuilt binary of Ruler (GTK3), compiled and ready to run.

No installation needed — just download `ruler` and run it directly, or copy it somewhere on your `$PATH` (e.g. `/usr/local/bin`).

```bash
chmod +x ruler
./ruler
```

This binary is provided for *Linux (x86-64) users* who prefer not to compile the source code manually.

Requires `GTK3` and `X11` runtime libraries to already be present on the system (these are pulled in by default on most Debian/Ubuntu and Fedora desktop installs).

They are intended for convenience and quick testing of the tool.

⚠️ Use at your own discretion and ensure you trust the provided builds before running them. If in doubt, build from `source/` instead — see the main [README](../README.md) for build instructions.
