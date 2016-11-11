'use strict';
const common = require('../common');
const assert = require('assert');
const path = require('path');
const fs = require('fs');
const fn = '__enclose_io_memfs__/x.txt';

fs.readFile(fn, function(err, data) {
  assert.ok(data);
});

fs.readFile(fn, 'utf8', function(err, data) {
  assert.strictEqual('xyz\n', data);
});

assert.ok(fs.readFileSync(fn));
assert.strictEqual('xyz\n', fs.readFileSync(fn, 'utf8'));
