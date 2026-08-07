project('ruler', 'c', version : '0.1.0', default_options : ['warning_level=2'])

gtk_dep = dependency('gtk+-3.0')
x11_dep = dependency('x11')
m_dep = meson.get_compiler('c').find_library('m', required : false)

gnome = import('gnome')

resources = gnome.compile_resources(
  'ruler-resources',
  'data' / 'ruler.gresource.xml',
  source_dir : 'data',
  c_name : 'ruler'
)

sources = [
  'src/main.c',
  'src/config.c',
  'src/x11-screen.c',
  'src/ruler-window.c',
  'src/options-window.c',
  resources,
]

executable('ruler',
  sources,
  dependencies : [gtk_dep, x11_dep, m_dep],
  install : true
)
