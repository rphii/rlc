
sources = [
  'arg.c',
  'array.c',
  'color.c',
  'file.c',
  'str.c',
  'utf8.c',
  'err.c',
  ]

headers = [
  'arg.h',
  'array.h',
  'attr.h',
  'color.h',
  'colorprint.h',
  'endian-detect.h',
  'err.h',
  'file.h',
  'lut.h',
  'platform-detect.h',
  'str.h',
  'utf8.h',
  'vec.h',
  ]

install_headers(headers, subdir: 'rl')
rlc = library('rlc',
  sources,
  link_args: '-lm',
  install: true,
  )

