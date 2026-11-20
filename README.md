# nwm, A Naive Window Manager

nwm is a very basic tiling X window manager.

## Status
This is still under construction!

## Installation and Running

You would need to have GCC, make, Xlib, and an X server installed.
Run `make DEBUG=0` for a release version. The default is a debug build.

To run, you can do `startx /path/to/nwm -- :0` from a console tty.
Another way is to build a `.xinitrc` file like this:

```
exec /path/to/nwm
```
and just run `startx` from a console tty.

# Contribution
Despite this being a learning project, I am happy to accept pull requests.
