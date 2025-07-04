use_relative_paths = True

vars = {
    'google_libwebsockets_git' : 'https://android.googlesource.com',
    'google_libwebsockets_revision' : 'ba7e6c39895f4a9468a93b26b5e3c730f1d43629',
}

deps = {
    'external/libwebsockets/v4.3': Var('google_libwebsockets_git') + '/platform/external/libwebsockets' + '@' + Var('google_libwebsockets_revision'),
}

hooks = [
  {
    'name': 'apply_libwebsockets_patch',
    'pattern': '.',
    'action': ['python3', '../../../scripts/apply_patch.py', 'include', 'libwebsockets.patch'],
    'cwd': 'external/libwebsockets/v4.3',
  },
]