
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
rlc = library('rlc',
  sources,
  link_args: '-lm',
  install: true,
  )

