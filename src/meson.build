
sources = [
  'array.c',
  'color.c',
  'err.c',
  ]

headers = [
  'array.h',
  'attr.h',
  'color.h',
  'colorprint.h',
  'endian-detect.h',
  'err.h',
  'lut.h',
  'platform-detect.h',
  'vec.h',
  ]

install_headers(headers, subdir: 'rl')
librlc = library('rlc',
  sources,
  link_args: '-lm',
  install: true,
  )

rlc_dep = declare_dependency(
  link_with: librlc,
  include_directories: include_directories('src'),
  )

