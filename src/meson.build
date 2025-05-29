
sources = [
  'arg.c',
  'array.c',
  'file.c',
  'map.c',
  'str2.c',
  'utf8.c',
  ]

headers = [
  'arg.h',
  'array.h',
  'attr.h',
  'color.h',
  'colorprint.h',
  'err.h',
  'file.h',
  'lut.h',
  'map.h',
  'platform-detect.h',
  'str2.h',
  'utf8.h',
  'vec.h',
  ]

install_headers(headers, subdir: 'rphii')
rphiic = library('rphiic',
  sources,
  link_args: '-lm',
  install: true,
  )

