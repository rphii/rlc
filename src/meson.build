
sources = [
  'file.c',
  'str.c',
  'utf8.c',
  ]

headers = [
  'attr.h',
  'colorprint.h',
  'err.h',
  'file.h',
  'lut.h',
  'platform-detect.h',
  'str.h',
  'utf8.h',
  'vec.h',
  ]

install_headers(headers, subdir: 'rphii')
rphiic = library('rphiic',
  sources,
  link_args: '-lm',
  install: true,
  )

