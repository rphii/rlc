project('rphiic', 'c',
  meson_version: '>=1.1')

subdir('src')
subdir('bash')

pkg = import('pkgconfig')
pkg.generate(
  libraries: rphiic,
  version: '9999',
  name: 'rphiic',
  description: 'Collection of rphii\'s C-libraries all in one place.',
  )

