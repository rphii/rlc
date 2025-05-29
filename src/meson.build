
sources = [
  'file.c',
  'str2.c',
  'utf8.c',
  'arg.c',
  'map.c',
  'array.c',
  'array.c',
  ]

headers = [
  'attr.h',
  'colorprint.h',
  'err.h',
  'file.h',
  'lut.h',
  'platform-detect.h',
  'str2.h',
  'utf8.h',
  'vec.h',
  'arg.h',
  'map.h',
  'array.h',
  ]

install_headers(headers, subdir: 'rphii')
rphiic = library('rphiic',
  sources,
  link_args: '-lm',
  install: true,
  )

