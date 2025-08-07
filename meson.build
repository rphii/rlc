project('rlc', 'c',
  meson_version: '>=1.1')

subdir('src')
subdir('bash')

pkg = import('pkgconfig')
pkg.generate(
  libraries: rlc,
  version: '9999',
  name: 'rlc',
  description: 'Essential meta-package for rl.',
  )

dep_rlc = declare_dependency(link_with: rlc)

