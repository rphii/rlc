sources = [
  'rlc/array.c',
  'rlc/color.c',
  'rlc/err.c',
  ]

headers = [
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

install_headers(headers, subdir: 'rlc')
install_headers('rlc.h')
librlc = library('rlc',
  sources,
  dependencies: [m_dep],
  install: true,
  )

pkg = import('pkgconfig')
pkg.generate(
  libraries: librlc,
  version: '9999',
  name: 'rlc',
  description: 'Essential meta-package for rl*',
  )


rlc_dep = declare_dependency(
  link_with: librlc,
  include_directories: '.',
  )


