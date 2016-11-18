/* eslint-disable strict */
const common = require('../common');
var assert = require('assert');
var fs = require('fs');

fs.stat('/__enclose_io_memfs__/b', common.mustCall(function(err, stats) {
  assert.ifError(err);
  assert.ok(stats.mtime instanceof Date);
  assert.strictEqual(this, global);
}));

fs.stat('/__enclose_io_memfs__/b', common.mustCall(function(err, stats) {
  assert.ok(stats.hasOwnProperty('blksize'));
  assert.ok(stats.hasOwnProperty('blocks'));
}));

fs.lstat('/__enclose_io_memfs__/b', common.mustCall(function(err, stats) {
  assert.ifError(err);
  assert.ok(stats.mtime instanceof Date);
  assert.strictEqual(this, global);
}));

fs.stat('/__enclose_io_memfs__/b/c.js', common.mustCall(function(err, s) {
  assert.ifError(err);
  assert.equal(false, s.isDirectory());
  assert.equal(true, s.isFile());
  assert.equal(false, s.isSocket());
  assert.equal(false, s.isBlockDevice());
  assert.equal(false, s.isCharacterDevice());
  assert.equal(false, s.isFIFO());
  assert.equal(false, s.isSymbolicLink());
  assert.ok(s.mtime instanceof Date);
}));
