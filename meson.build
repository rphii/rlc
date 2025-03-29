project('rphiic', 'c')
subdir('src')

pkg = import('pkgconfig')
pkg.generate(
  libraries: rphiic,
  subdirs: headers,
  version: '9999',
  name: 'rphiic',
  description: 'Collection of rphii\'s C-libraries all in one place.',
  )

