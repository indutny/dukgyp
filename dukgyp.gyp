{
  "variables": {
    "dukgyp_platform%": "<(OS)",
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
      ["OS=='solaris'", {
        "variables": {
          "dukgyp_platform": "sunos",
        },
      }],
    ],
    "defines": [
      "DUKGYP_PLATFORM=\"<(dukgyp_platform)\"",
    ],
    "include_dirs": [
      "deps/duktape",
      "<(SHARED_INTERMEDIATE_DIR)",
    ],
    "sources": [
      "deps/duktape/duktape.c",
      "src/dukgyp.c",
    ],
    "libraries": [
      "-lm",
    ],
  },
  "targets": [{
    "target_name": "dukgyp",
    "type": "executable",
    "dependencies": [
      "dukgyp_js",
    ],
  }, {
    "target_name": "dukgyp-test",
    "type": "executable",
    "dependencies": [
      "dukgyp_js",
    ],
    "defines": [
      "DUKGYP_TEST",
    ],
  }, {
    "target_name": "copy_binary",
    "type": "none",
    "dependencies" : [ "dukgyp" ],
    "copies": [
      {
        "destination": "./bin/",
        "files": ["./out/Release/dukgyp"]
      },
    ],
  }, {
    "target_name": "dukgyp_js",
    "type": "none",
    "dependencies": [
      "dukgyp_webpack",
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
    }, {
      'action_name': 'source2blob',
      'inputs': [
        'lib/test.js',
      ],
      'outputs': [
        '<(SHARED_INTERMEDIATE_DIR)/src/dukgyp-test-js.h'
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
    "target_name": "dukgyp_webpack",
    "type": "none",

    "actions": [{
      'action_name': 'webpack',
      'inputs': [
        'src/dukgyp.js',
        'src/test.js',
      ],
      'outputs': [
        'lib/dukgyp.js',
        'lib/test.js',
      ],
      'action': [
        "./node_modules/.bin/webpack",
      ],
    }],
  }],
}
