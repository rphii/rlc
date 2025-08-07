
sources = [
  'rlc/array.c',
  'rlc/color.c',
  'rlc/err.c',
  ]

headers = [
  'rlc.h',
  'rlc/array.h',
  'rlc/attr.h',
  'rlc/color.h',
  'rlc/colorprint.h',
  'rlc/endian-detect.h',
  'rlc/err.h',
  'rlc/lut.h',
  'rlc/platform-detect.h',
  'rlc/vec.h',
  ]

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required: false)

install_headers(headers)
librlc = library('rlc',
  sources,
  link_args: [m_dep],
  install: true,
  )

rlc_dep = declare_dependency(
  link_with: librlc,
  include_directories: '.',
  )

