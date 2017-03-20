'use strict';

bindings.env = new Proxy({}, {
  has: (target, key) => {
    return false;
  },
  get: (target, key) => {
    return bindings.getenv(key);
  },
  set: () => {
    throw new Error('env is read-only');
  },
  deleteProperty: () => {
    throw new Error('env is read-only');
  }
});
