test_sources = files([
  'src/flags.test.cpp',
  'src/typetraits.test.cpp',
  'src/tuple.test.cpp',
  'src/main.cpp',
  'src/lambda.test.cpp',
])

test_exe = executable('tests',
  test_sources,
  include_directories : '../include',
  native : true,
  cpp_args : ['-std=gnu++2a', '-DCATCH_CONFIG_FAST_COMPILE'])

test('STL tests', test_exe, args : ['-s', '--use-colour', 'no'])
