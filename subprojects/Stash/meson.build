project('Stash', 'cpp',
	version : '1.0.0',
	license : 'GPL3',
	default_options : ['cpp_std=c++11', 'warning_level=3', 'optimization=3'])

sources = files('src/Stash.cpp', 'src/Read.cpp', 'src/city.cc')

include_dirs = include_directories('include')

cpp = meson.get_compiler('cpp')
btllib = cpp.find_library('btllib')
openmp = dependency('openmp')
dependencies = [btllib, openmp]

install_dir = join_paths(meson.current_source_dir(), 'lib')

stash = static_library('stash',
	sources,
	include_directories : include_dirs,
	dependencies : dependencies,
	install : true,
	install_dir : install_dir)

headers = files('include/Stash.h', 'include/Read.h', 'include/city.h', 'include/config.h')
install_headers(headers, install_dir : join_paths(install_dir, 'include/stash'))

stash_dep = declare_dependency(include_directories : include_dirs, link_with : stash)
