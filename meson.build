project('StashDemo', 'cpp',
	version : '1.0.0',
	license : 'GPL3',
	default_options : ['cpp_std=c++11', 'warning_level=3', 'werror=true', 'optimization=3'])

stash_proj = subproject('Stash')

executable('Stash_Demo',
	'demo.cpp',
	dependencies : stash_proj.get_variable('stash_dep'),
	install : true)
