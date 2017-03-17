{
  "target_defaults": {
    "include_dirs": [
      ".",
    ],
  },
  "targets": [{
    "target_name": "dukgyp",
    "type": "executable",

    "include_dirs": [
      "deps/duktape",
      "<(SHARED_INTERMEDIATE_DIR)",
    ],

    "sources": [
      "deps/duktape/duktape.c",
      "src/dukgyp.c",
    ],

    "actions": [{
      'action_name': 'source2blob',
      'inputs': [
        'lib/dukgyp.js',
      ],
      'outputs': [
        '<(SHARED_INTERMEDIATE_DIR)/src/dukgyp-js.h'
      ],
      'action': [
        'node',
        'tools/source2blob.js',
        'dukgyp-js',
        '<@(_inputs)',
        '<@(_outputs)',
      ],
    }],
  }],
}
