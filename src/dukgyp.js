'use strict';

require('babel-polyfill');
const gyp = require('gyp.js');

try {
  const code = gyp.main(bindings.argv.slice(1));
  if (code !== null)
    bindings.exit(code);
} catch (e) {
  bindings.error(e.stack + '\n');
  bindings.exit(-1);
}
bindings.exit(0);
