project('zlib', 'c', version : '1.2.11', license : 'zlib',
  default_options: [
    'c_std=c99',
  ]
)

link_args = []

cc = meson.get_compiler('c')

if cc.get_id() == 'gcc' and host_machine.system() == 'linux'
  vflag = '-Wl,--version-script,@0@/zlib.map'.format(meson.current_source_dir())
  link_args += [vflag]
endif

conf = configuration_data()

with_gzfileop = false
if get_option('ZLIB_COMPAT').enabled()
    with_gzfileop = true
    # Don't know why this isn't sensed via zconf.h
    add_project_arguments('-DZLIB_COMPAT=1', language: 'c')
    SUFFIX = ''
else
    SUFFIX = '-ng'
endif
if (with_gzfileop)
    add_project_arguments('-DWITH_GZFILEOP=1', language: 'c')
    conf.set('WITH_GZFILEOP', 1)
endif

# Add and then expand #cmakedefine directives
generate_cmakein = find_program('meson/generate-cmakein.py')
foreach f: ['zconf'+SUFFIX+'.h']
  run_command(
    generate_cmakein,
    meson.current_source_dir() / f + '.in',
    meson.current_build_dir() / f + '.cmakein',
    check: true)
  configure_file(
    input: meson.current_build_dir() / f + '.cmakein',
    output: f,
    format: 'cmake',
    configuration: conf,
    install: true,
    install_dir: 'include',
  )
endforeach

pkg = import('pkgconfig')

src = [
  'adler32.c',
  'compress.c',
  'crc32.c',
  'deflate.c',
  'deflate_fast.c',
  'deflate_medium.c',
  'deflate_slow.c',
  'functable.c',
  'infback.c',
  'inffast.c',
  'inflate.c',
  'inftrees.c',
  'insert_string.c',
  'trees.c',
  'uncompr.c',
  'zutil.c',
]

gzsrc = [
  'gzclose.c',
  'gzlib.c',
  'gzread.c',
  'gzwrite.c',
]
if (with_gzfileop)
  src += gzsrc
endif

headers = ['zlib.h']

zlib = library('z', src,
  c_args : ['-DZLIB_DLL'],
  link_args : link_args,
  install : true,
  version : meson.project_version())

incdir = include_directories('.')

zlib_dep = declare_dependency(link_with : zlib,
  include_directories : incdir)

install_headers(headers)

pkg.generate(name: 'zlib',
             description: 'zlib compression library',
             version: meson.project_version(),
             libraries: zlib)

if get_option('ZLIB_ENABLE_TESTS').enabled()
  # Tests get built WITH_GZFILEOP come heck or high water
  if with_gzfileop
    gztsrc = []
  else
    gztsrc = gzsrc
  endif

  example = executable('example', 'test/example.c', gztsrc,
    c_args: '-DWITH_GZFILEOP=1',
    link_with: zlib)
  test('example', example)

  run_and_redirect = find_program('meson/run-and-redirect.py')
  minigzip = executable('minigzip', 'test/minigzip.c', gztsrc,
    c_args: '-DWITH_GZFILEOP=1',
    link_with: zlib)
  CVES = ['CVE-2002-0059', 'CVE-2004-0797', 'CVE-2005-1849', 'CVE-2005-2096']
  foreach CVE: CVES
    input = meson.current_source_dir() / 'test' / CVE / 'test.gz'
    test(CVE, run_and_redirect, args: [input, '/dev/null', minigzip, '-d'])
  endforeach
endif

summary({
  'ZLIB_COMPAT': get_option('ZLIB_COMPAT').enabled(),
  'ZLIB_ENABLE_TESTS': get_option('ZLIB_ENABLE_TESTS').enabled(),
})
