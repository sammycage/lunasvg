project('lunasvg', 'cpp',
    version: '3.3.0',
    license: 'MIT',
    meson_version: '>=0.59.0',
    default_options: ['cpp_std=c++17']
)

plutovg_dep = dependency('plutovg',
    required: true,
    version: '>=1.0.0',
    fallback: ['plutovg', 'plutovg_dep']
)

lunasvg_sources = [
    'source/lunasvg.cpp',
    'source/graphics.cpp',
    'source/svgelement.cpp',
    'source/svggeometryelement.cpp',
    'source/svgpaintelement.cpp',
    'source/svgparser.cpp',
    'source/svgproperty.cpp',
    'source/svglayoutstate.cpp',
    'source/svgrenderstate.cpp',
    'source/svgtextelement.cpp'
]

lunasvg_compile_args = []
if get_option('default_library') == 'static'
    lunasvg_compile_args += ['-DLUNASVG_BUILD_STATIC']
endif

lunasvg_lib = library('lunasvg', lunasvg_sources,
    include_directories: include_directories('include', 'source'),
    dependencies: plutovg_dep,
    version: meson.project_version(),
    cpp_args: ['-DLUNASVG_BUILD'] + lunasvg_compile_args,
    gnu_symbol_visibility: 'hidden',
    install: true
)

lunasvg_dep = declare_dependency(
    link_with: lunasvg_lib,
    include_directories: include_directories('include'),
    compile_args: lunasvg_compile_args
)

meson.override_dependency('lunasvg', lunasvg_dep)
install_headers('include/lunasvg.h', subdir: 'lunasvg')

if not get_option('examples').disabled()
    subdir('examples')
endif

pkgmod = import('pkgconfig')
pkgmod.generate(lunasvg_lib,
    name: 'LunaSVG',
    description: 'SVG rendering library',
    filebase: 'lunasvg',
    subdirs: 'lunasvg'
)
