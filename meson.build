project('leakcheck', 'c', default_options: ['c_std=gnu99'])

cc = meson.get_compiler('c')
dl_dep = cc.find_library('dl', required: true)

leakcheck_static = static_library('leakcheck_static',
    ['src/leakcheck/leakcheck.c'],
    dependencies: [dl_dep],
    include_directories: ['src'],
    install: false)

leakcheck_static_dep = declare_dependency(
    link_with: [leakcheck_static],
    dependencies: [dl_dep],
    include_directories: ['src'])

shared_library('leakcheck',
    ['src/api.c'],
    dependencies: [leakcheck_static_dep],
    link_args:  '-Wl,--version-script,@0@/leakcheck.map'.format(meson.current_source_dir()),
    link_depends: 'leakcheck.map',
    install: true)

install_data('leakcheck', install_dir: 'bin', install_mode: 'rwxr-xr-x')

executable('test_no_alloc', ['test/test_no_alloc.c'])
executable('test_malloc', ['test/test_malloc.c'])
executable('test_calloc', ['test/test_calloc.c'])
executable('test_realloc', ['test/test_realloc.c'])
executable('test_invalid_free', ['test/test_invalid_free.c'])
executable('test_missing_free', ['test/test_missing_free.c'])