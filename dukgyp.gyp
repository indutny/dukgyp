{
  "variables": {
    "dukgyp_platform%": "unknown",
  },
  "target_defaults": {
    "include_dirs": [
      ".",
    ],
    "conditions": [
      ["OS=='mac'", {
        "variables": {
          "dukgyp_platform": "darwin",
        },
      }],
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

    "defines": [
      "DUKGYP_PLATFORM=\"<(dukgyp_platform)\"",
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
  }, {
    "target_name": "copy_binary",
    "type":"none",
    "dependencies" : [ "dukgyp" ],
    "copies": [
      {
        "destination": "./bin/",
        "files": ["./out/Release/dukgyp"]
      },
    ],
  }],
}
