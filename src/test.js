'use strict';

let tap = {
  total: 13,
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
    const out = `Expected "${a}" to be equal to "${b}"`;
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


assert.doesNotThrow = (fn, msg) => {
  const id = ++tap.current;
  try {
    fn();
  } catch (e) {
    bindings.log(`not ok ${id} ${msg}\n`);
    bindings.error(e.stack + '\n');
    tap.errors++;
    return;
  }

  bindings.log(`ok ${id} ${msg}\n`);
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

assert(fs.existsSync(fs.realpathSync(bindings.argv[1])),
       'fs.existsSync() must return true');
assert(!fs.existsSync('random-stuff-that-does-not-exist'),
       'fs.existsSync() must return false');

assert.doesNotThrow(() => {
  fs.mkdirpSync('/tmp/dukgyp-test/sub/b');
}, 'fs.mkdirpSync() should not throw');
assert.doesNotThrow(() => {
  fs.writeFileSync('/tmp/dukgyp-test/sub/b/file', 'hello');
}, 'fs.writeFileSync() should not throw');
assert.equal(fs.readFileSync('/tmp/dukgyp-test/sub/b/file').toString(), 'hello',
             'fs.readFileSync() should return data');

//
// TAP end
//
if (tap.current !== tap.total)
  throw new Error('TAP PROBLEM, update `tap.total` in test.js');

bindings.exit(tap.errors === 0 ? 0 : -1);
throw new Error('Unreachable');
