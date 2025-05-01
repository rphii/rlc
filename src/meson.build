
sources = [
  'arg.c',
  'file.c',
  'str.c',
  'tui.c',
  'utf8.c',
  'wstr.c',
  ]

headers = [
  'arg.h',
  'attr.h',
  'colorprint.h',
  'err.h',
  'file.h',
  'lut.h',
  'platform-detect.h',
  'str.h',
  'tui.h',
  'utf8.h',
  'vec.h',
  'wstr.h',
  ]

install_headers(headers, subdir: 'rphii')
rphiic = library('rphiic',
  sources,
  link_args: '-lm',
  install: true,
  )

