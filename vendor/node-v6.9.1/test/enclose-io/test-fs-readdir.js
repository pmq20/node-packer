'use strict';

const common = require('../common');
const assert = require('assert');
const fs = require('fs');
const path = require('path');

// Check the readdir Sync version
var x = fs.readdirSync(path.join(common.fixturesDir, 'b')).sort();
var y = fs.readdirSync('__enclose_io_memfs__/b').sort();
assert.deepStrictEqual(x, y);

// Check the readdir async version
fs.readdir('__enclose_io_memfs__/b', common.mustCall(function(err, f) {
  assert.ifError(err);
  assert.deepStrictEqual(x, f.sort());
}));
