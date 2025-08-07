project('rlc', 'c',
  meson_version: '>=1.1')

subdir('src')
subdir('bash')

pkg = import('pkgconfig')
pkg.generate(
  libraries: librlc,
  version: '9999',
  name: 'rlc',
  description: 'Essential meta-package for rl*',
  )

