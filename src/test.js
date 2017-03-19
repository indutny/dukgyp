'use strict';

let tap = {
  total: 8,
  current: 0,
  errors: 0
};

const assert = (cond, msg) => {
  const id = ++tap.current;
  if (cond) {
    bindings.log(`ok ${id} ${msg}\n`);
  } else {
    bindings.log(`not ok ${id} ${msg}\n`);
    tap.errors++;
  }
};

assert.equal = (a, b, msg) => {
  const id = ++tap.current;
  if (a === b) {
    bindings.log(`ok ${id} ${msg}\n`);
  } else {
    bindings.log(`not ok ${id} ${msg}\n`);
    const out = msg || `Expected "${a}" to be equal to "${b}"`;
    bindings.error(out + '\n');
    tap.errors++;
  }
};

assert.throws = (fn, msg) => {
  const id = ++tap.current;
  try {
    fn();
  } catch (e) {
    bindings.log(`ok ${id} ${msg}\n`);
    return;
  }

  bindings.log(`not ok ${id} ${msg}\n`);
  tap.errors++;
};

//
// General bindings
//

bindings.log(`1..${tap.total}\n`);

assert.equal(typeof bindings.platform, 'string',
             'Platform must be present');
assert([
  'darwin', 'solaris', 'win32', 'linux', 'freebsd'
].indexOf(bindings.platform) !== -1, 'Platform must be known');
assert.equal(typeof bindings.cwd(), 'string', 'cwd() must return string');
assert.equal(bindings.argv[0], 'duktape', 'argv[0] must be `duktape`');
assert(/dukgyp-test/.test(bindings.argv[1]), 'argv[1] must be proper');

//
// FS
//
const fs = bindings.fs;

assert.equal(JSON.stringify(fs.readdirSync()), '[]',
             'fs.readdirSync() stub must return an array');
assert.throws(() => {
  fs.realpathSync('random-non-existent-stuff');
}, 'fs.realpathSync(random) must throw');
assert(/dukgyp-test/.test(fs.realpathSync(bindings.argv[1])),
       'fs.realpathSync() should return meaningful data');

//
// TAP end
//
if (tap.current !== tap.total)
  throw new Error('TAP PROBLEM, change `tap.total` in test.js');

bindings.exit(tap.errors === 0 ? 0 : -1);
throw new Error('Unreachable');
