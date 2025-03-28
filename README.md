# rphii/c

Collection of rphii's C-libraries all in one place.

## Containing:

### Header-only:

- [`platform-detect.h`](src/platform-detect.h) Platform detect header
- [`attr.h`](src/attr.h) Attribute header
- [`colorprint.h`](src/colorprint.h) Colored formatting helper header
- [`err.h`](src/err.h) Error (try,etc) header
- [`lut.h`](src/lut.h) Generic lookup-table header
- [`vec.h`](src/vec.h) Generic vector header

### Source-files (only headers listed):

- [`str.h`](src/str.h) String functions
- [`utf8.h`](src/utf8.h) UTF8-helper functions
- [`file.h`](src/file.h) File functions

## Compilation, installation

This process is simplified with the help of the [meson build system](https://github.com/mesonbuild/meson):

    meson setup build
    meson compile -C build
    meson install -C build

