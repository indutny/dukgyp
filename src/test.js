'use strict';

const assert = (cond, msg) => {
  if (!cond)
    throw new Error(msg || 'Assertion failed');
};

assert.equal = (a, b, msg) => {
  if (a !== b)
    throw new Error(msg || `Expected "${a}" to be equal to "${b}"`);
};

//
// General bindings
//

bindings.log('test start\n');
bindings.error('with stderr output too\n');

assert.equal(typeof bindings.platform, 'string');
assert([
  'darwin', 'solaris', 'win32', 'linux', 'freebsd'
].indexOf(bindings.platform) !== -1, 'Platform must be known');
assert.equal(typeof bindings.cwd(), 'string');

//
// FS
//
const fs = bindings.fs;

// Just a stub, but we still need it to output an array
assert.equal(JSON.stringify(fs.readdirSync()), '[]');

bindings.log('ok\n');
bindings.exit(0);
throw new Error('Unreachable');
