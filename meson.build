project('Mart', 'c',
  version : '1.0',
  default_options : ['warning_level=3'])

dep = [
    dependency('SDL3'),
    dependency('SDL3_ttf'),
    dependency('SDL3_image'),
    dependency('libmpdclient'),
]
src = [
    'tomlc99/toml.c',
    'src/about_main.c',
    'src/album-art.c',
    'src/compat.c',
    'src/config.c',
    'src/gui_controlls.c',
    'src/imgui.c',
    'src/state.c',
    'src/main.c',
]

inc = [
    include_directories('tomlc99', 'embeds'),
]

executable('mart',
            src,
            dependencies : dep,
            include_directories : inc,
            install : true)
install_man('docs/mart.1', 'docs/mart.toml.5')