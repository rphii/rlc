# rlc

Essential meta-package for rl\*.

## Containing:

### Header-only:

- [`attr.h`](src/rlc/attr.h) Attribute header
- [`colorprint.h`](src/rlc/colorprint.h) Colored formatting helper header
- [`endian-detect.h`](src/rlc/endian-detect.h) Colored formatting helper header
- [`lut.h`](src/rlc/lut.h) Generic lookup-table header
- [`platform-detect.h`](src/rlc/platform-detect.h) Platform detect header
- [`vec.h`](src/rlc/vec.h) Generic vector header (compile time)

### Source-files (only headers listed):

- [`array.h`](src/rlc/array.h) Generic array header (faster to use than vec)
- [`color.h`](src/rlc/color.h) Basic operations in color space
- [`err.h`](src/rlc/err.h) Error (try,etc) header
- [`str.h`](src/rlc/str.h) String functions
- [`utf8.h`](src/rlc/utf8.h) UTF8-helper functions

## Compilation, installation

This process is simplified with the help of the [meson build system](https://github.com/mesonbuild/meson):

    meson setup build
    meson install -C build

