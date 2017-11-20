// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

'use strict';

const util = require('util');
const { deprecate, convertToValidSignal } = require('internal/util');
const { isUint8Array } = require('internal/util/types');
const { createPromise,
        promiseResolve, promiseReject } = process.binding('util');
const debug = util.debuglog('child_process');

const uv = process.binding('uv');
const spawn_sync = process.binding('spawn_sync');
const { Buffer } = require('buffer');
const { Pipe } = process.binding('pipe_wrap');
const child_process = require('internal/child_process');
const {
  _validateStdio,
  setupChannel,
  ChildProcess
} = child_process;

const errnoException = util._errnoException;
exports.ChildProcess = ChildProcess;

function stdioStringToArray(option) {
  switch (option) {
    case 'ignore':
    case 'pipe':
    case 'inherit':
      return [option, option, option, 'ipc'];
    default:
      throw new TypeError('Incorrect value of stdio option: ' + option);
  }
}

exports.fork = function(modulePath /*, args, options*/) {

  // Get options and args arguments.
  var execArgv;
  var options = {};
  var args = [];
  var pos = 1;
  if (pos < arguments.length && Array.isArray(arguments[pos])) {
    args = arguments[pos++];
  }

  if (pos < arguments.length && arguments[pos] != null) {
    if (typeof arguments[pos] !== 'object') {
      throw new TypeError('Incorrect value of args option');
    }

    options = util._extend({}, arguments[pos++]);
  }

  // Prepare arguments for fork:
  execArgv = options.execArgv || process.execArgv;

  if (execArgv === process.execArgv && process._eval != null) {
    const index = execArgv.lastIndexOf(process._eval);
    if (index > 0) {
      // Remove the -e switch to avoid fork bombing ourselves.
      execArgv = execArgv.slice();
      execArgv.splice(index - 1, 2);
    }
  }

  args = execArgv.concat([modulePath], args);

  if (typeof options.stdio === 'string') {
    options.stdio = stdioStringToArray(options.stdio);
  } else if (!Array.isArray(options.stdio)) {
    // Use a separate fd=3 for the IPC channel. Inherit stdin, stdout,
    // and stderr from the parent if silent isn't set.
    options.stdio = options.silent ? stdioStringToArray('pipe') :
      stdioStringToArray('inherit');
  } else if (options.stdio.indexOf('ipc') === -1) {
    throw new TypeError('Forked processes must have an IPC channel');
  }

  options.execPath = options.execPath || process.execPath;
  options.shell = false;

  return spawn(options.execPath, args, options);
};


exports._forkChild = function(fd) {
  // set process.send()
  var p = new Pipe(true);
  p.open(fd);
  p.unref();
  const control = setupChannel(process, p);
  process.on('newListener', function onNewListener(name) {
    if (name === 'message' || name === 'disconnect') control.ref();
  });
  process.on('removeListener', function onRemoveListener(name) {
    if (name === 'message' || name === 'disconnect') control.unref();
  });
};


function normalizeExecArgs(command, options, callback) {
  if (typeof options === 'function') {
    callback = options;
    options = undefined;
  }

  // Make a shallow copy so we don't clobber the user's options object.
  options = Object.assign({}, options);
  options.shell = typeof options.shell === 'string' ? options.shell : true;

  return {
    file: command,
    options: options,
    callback: callback
  };
}


exports.exec = function(command /*, options, callback*/) {
  var opts = normalizeExecArgs.apply(null, arguments);
  return exports.execFile(opts.file,
                          opts.options,
                          opts.callback);
};

const customPromiseExecFunction = (orig) => {
  return (...args) => {
    const promise = createPromise();

    orig(...args, (err, stdout, stderr) => {
      if (err !== null) {
        err.stdout = stdout;
        err.stderr = stderr;
        promiseReject(promise, err);
      } else {
        promiseResolve(promise, { stdout, stderr });
      }
    });
    return promise;
  };
};

Object.defineProperty(exports.exec, util.promisify.custom, {
  enumerable: false,
  value: customPromiseExecFunction(exports.exec)
});

exports.execFile = function(file /*, args, options, callback*/) {
  var args = [];
  var callback;
  var options = {
    encoding: 'utf8',
    timeout: 0,
    maxBuffer: 200 * 1024,
    killSignal: 'SIGTERM',
    cwd: null,
    env: null,
    shell: false
  };

  // Parse the optional positional parameters.
  var pos = 1;
  if (pos < arguments.length && Array.isArray(arguments[pos])) {
    args = arguments[pos++];
  } else if (pos < arguments.length && arguments[pos] == null) {
    pos++;
  }

  if (pos < arguments.length && typeof arguments[pos] === 'object') {
    util._extend(options, arguments[pos++]);
  } else if (pos < arguments.length && arguments[pos] == null) {
    pos++;
  }

  if (pos < arguments.length && typeof arguments[pos] === 'function') {
    callback = arguments[pos++];
  }

  if (!callback && pos < arguments.length && arguments[pos] != null) {
    throw new TypeError('Incorrect value of args option');
  }

  // Validate the timeout, if present.
  validateTimeout(options.timeout);

  // Validate maxBuffer, if present.
  validateMaxBuffer(options.maxBuffer);

  options.killSignal = sanitizeKillSignal(options.killSignal);

  var child = spawn(file, args, {
    cwd: options.cwd,
    env: options.env,
    gid: options.gid,
    uid: options.uid,
    shell: options.shell,
    windowsHide: !!options.windowsHide,
    windowsVerbatimArguments: !!options.windowsVerbatimArguments
  });

  var encoding;
  var _stdout;
  var _stderr;
  if (options.encoding !== 'buffer' && Buffer.isEncoding(options.encoding)) {
    encoding = options.encoding;
    _stdout = '';
    _stderr = '';
  } else {
    _stdout = [];
    _stderr = [];
    encoding = null;
  }
  var stdoutLen = 0;
  var stderrLen = 0;
  var killed = false;
  var exited = false;
  var timeoutId;

  var ex = null;

  var cmd = file;

  function exithandler(code, signal) {
    if (exited) return;
    exited = true;

    if (timeoutId) {
      clearTimeout(timeoutId);
      timeoutId = null;
    }

    if (!callback) return;

    // merge chunks
    var stdout;
    var stderr;
    if (encoding) {
      stdout = _stdout;
      stderr = _stderr;
    } else {
      stdout = Buffer.concat(_stdout);
      stderr = Buffer.concat(_stderr);
    }

    if (!ex && code === 0 && signal === null) {
      callback(null, stdout, stderr);
      return;
    }

    if (args.length !== 0)
      cmd += ' ' + args.join(' ');

    if (!ex) {
      ex = new Error('Command failed: ' + cmd + '\n' + stderr);
      ex.killed = child.killed || killed;
      ex.code = code < 0 ? uv.errname(code) : code;
      ex.signal = signal;
    }

    ex.cmd = cmd;
    callback(ex, stdout, stderr);
  }

  function errorhandler(e) {
    ex = e;

    if (child.stdout)
      child.stdout.destroy();

    if (child.stderr)
      child.stderr.destroy();

    exithandler();
  }

  function kill() {
    if (child.stdout)
      child.stdout.destroy();

    if (child.stderr)
      child.stderr.destroy();

    killed = true;
    try {
      child.kill(options.killSignal);
    } catch (e) {
      ex = e;
      exithandler();
    }
  }

  if (options.timeout > 0) {
    timeoutId = setTimeout(function delayedKill() {
      kill();
      timeoutId = null;
    }, options.timeout);
  }

  if (child.stdout) {
    if (encoding)
      child.stdout.setEncoding(encoding);

    child.stdout.on('data', function onChildStdout(chunk) {
      stdoutLen += encoding ? Buffer.byteLength(chunk, encoding) : chunk.length;

      if (stdoutLen > options.maxBuffer) {
        ex = new Error('stdout maxBuffer exceeded');
        kill();
      } else if (encoding) {
        _stdout += chunk;
      } else {
        _stdout.push(chunk);
      }
    });
  }

  if (child.stderr) {
    if (encoding)
      child.stderr.setEncoding(encoding);

    child.stderr.on('data', function onChildStderr(chunk) {
      stderrLen += encoding ? Buffer.byteLength(chunk, encoding) : chunk.length;

      if (stderrLen > options.maxBuffer) {
        ex = new Error('stderr maxBuffer exceeded');
        kill();
      } else if (encoding) {
        _stderr += chunk;
      } else {
        _stderr.push(chunk);
      }
    });
  }

  child.addListener('close', exithandler);
  child.addListener('error', errorhandler);

  return child;
};

Object.defineProperty(exports.execFile, util.promisify.custom, {
  enumerable: false,
  value: customPromiseExecFunction(exports.execFile)
});

const _deprecatedCustomFds = deprecate(
  function deprecateCustomFds(options) {
    options.stdio = options.customFds.map(function mapCustomFds(fd) {
      return fd === -1 ? 'pipe' : fd;
    });
  }, 'child_process: options.customFds option is deprecated. ' +
     'Use options.stdio instead.', 'DEP0006');

function _convertCustomFds(options) {
  if (options.customFds && !options.stdio) {
    _deprecatedCustomFds(options);
  }
}

function __enclose_io_memfs__node_shebang(file) {
  const fs = require('fs');
  const fd = fs.openSync(file, 'r');
  if (fd < 0) {
    return false;
  }
  var buffer = new Buffer(2);
  var bytesRead = fs.readSync(fd, buffer, 0, 2, 0);
  if (2 != bytesRead) {
    fs.closeSync(fd);
    return false;
  }
  if ('#'.charCodeAt(0) === buffer[0] && '!'.charCodeAt(0) === buffer[1]) {
    var line = '';
    var index = 0;
    do {
      var bytesRead = fs.readSync(fd, buffer, 0, 1, index);
      if (1 != bytesRead) {
        fs.closeSync(fd);
        return false;
      }
      ++index;
      line += String.fromCharCode(buffer[0]);
    } while ('\n' !== line[line.length - 1]);
    var result = line.match(new RegExp("#!/usr/bin/env node(\\n|\\b.*\\n)"));
    if (null !== result) {
      fs.closeSync(fd);
      return result[1];
    }
  }
  fs.closeSync(fd);
  return false;
}

function normalizeSpawnArguments(file, args, options) {
  // --------- [Enclose.IO Hack start] ---------
  debug('normalizeSpawnArguments started with', file, args, options);
  // --------- [Enclose.IO Hack end] ---------

  if (typeof file !== 'string' || file.length === 0)
    throw new TypeError('"file" argument must be a non-empty string');

  if (Array.isArray(args)) {
    args = args.slice(0);
  } else if (args !== undefined &&
             (args === null || typeof args !== 'object')) {
    throw new TypeError('Incorrect value of args option');
  } else {
    options = args;
    args = [];
  }

  if (options === undefined)
    options = {};
  else if (options === null || typeof options !== 'object')
    throw new TypeError('"options" argument must be an object');

  // Validate the cwd, if present.
  if (options.cwd != null &&
      typeof options.cwd !== 'string') {
    throw new TypeError('"cwd" must be a string');
  }

  // Validate detached, if present.
  if (options.detached != null &&
      typeof options.detached !== 'boolean') {
    throw new TypeError('"detached" must be a boolean');
  }

  // Validate the uid, if present.
  if (options.uid != null && !Number.isInteger(options.uid)) {
    throw new TypeError('"uid" must be an integer');
  }

  // Validate the gid, if present.
  if (options.gid != null && !Number.isInteger(options.gid)) {
    throw new TypeError('"gid" must be an integer');
  }

  // Validate the shell, if present.
  if (options.shell != null &&
      typeof options.shell !== 'boolean' &&
      typeof options.shell !== 'string') {
    throw new TypeError('"shell" must be a boolean or string');
  }

  // Validate argv0, if present.
  if (options.argv0 != null &&
      typeof options.argv0 !== 'string') {
    throw new TypeError('"argv0" must be a string');
  }

  // Validate windowsHide, if present.
  if (options.windowsHide != null &&
      typeof options.windowsHide !== 'boolean') {
    throw new TypeError('"windowsHide" must be a boolean');
  }

  // Validate windowsVerbatimArguments, if present.
  if (options.windowsVerbatimArguments != null &&
      typeof options.windowsVerbatimArguments !== 'boolean') {
    throw new TypeError('"windowsVerbatimArguments" must be a boolean');
  }

  // Make a shallow copy so we don't clobber the user's options object.
  options = Object.assign({}, options);

  // --------- [Enclose.IO Hack start] ---------
  // allow executing files within the enclosed package
  var will_extract = true;
  var args_extract = function(obj) {
    if (!will_extract) {
      return obj;
    }
    if (obj && obj.indexOf && 0 === obj.indexOf('/__enclose_io_memfs__')) {
      var file_extracted = process.__enclose_io_memfs__extract(obj);
      if (false === file_extracted) {
        debug('process.__enclose_io_memfs__extract failed with', obj, file_extracted);
        will_extract = false;
        return obj;
      } else {
        debug('process.__enclose_io_memfs__extract succeeded with', obj, file_extracted);
        return file_extracted;
      }
    } else {
      return obj;
    }
  };
  
  if ('node' === file || process.execPath === file) {
    will_extract = false;
    file = process.execPath;
  } else {
    if (process.platform === 'win32') {
      if (file && file.indexOf && 1 === file.indexOf(':\\__enclose_io_memfs__')) {
        file = file.substr(2).replace(/\\/g, '/');
      } else if (file && file.indexOf && 0 === file.indexOf('\\\\?\\__enclose_io_memfs__')) {
        file = file.substr(3).replace(/\\/g, '/');
      } else if (file && file.indexOf && 0 === file.indexOf('\\\\?\\') && 1 === file.substr(4).indexOf(':\\__enclose_io_memfs__')) {
        file = file.substr(6).replace(/\\/g, '/');
      }
    }
    if (file && file.indexOf && 0 === file.indexOf('/__enclose_io_memfs__')) {
      // shebang: looking at the two bytes at the start of an executable file
      var shebang_args = __enclose_io_memfs__node_shebang(file);
      if (false === shebang_args) {
        var file_extracted;
        if (/^win/.test(process.platform)) {
          file_extracted = process.__enclose_io_memfs__extract(file, 'exe');
        } else {
          file_extracted = process.__enclose_io_memfs__extract(file);
        }
        if (false === file_extracted) {
          debug('process.__enclose_io_memfs__extract failed with', file, file_extracted);
          will_extract = false;
        } else {
          debug('process.__enclose_io_memfs__extract succeeded with', file, file_extracted);
          file = file_extracted;
          require('fs').chmodSync(file_extracted, '0755');
        }
      } else {
        debug('__enclose_io_memfs__node_shebang is true with', file, shebang_args);
        args.unshift(file);
        if ('' !== shebang_args.trim()) {
          args.unshift(shebang_args.trim());
        }
        file = process.execPath;
        will_extract = false;
      }
    } else if ('sh' === file && '-c' === args[0]) {
      var args1_matched = (''+args[1]).match(/^(\/__enclose_io_memfs__[^\s]+)(\s*)(.*)$/);
      if (null !== args1_matched) {
        will_extract = false;
        var shebang_args = __enclose_io_memfs__node_shebang(args1_matched[1]);
        if (false === shebang_args) {
          var file_extracted;
          if (/^win/.test(process.platform)) {
            file_extracted = process.__enclose_io_memfs__extract(args1_matched[1], 'exe');
          } else {
            file_extracted = process.__enclose_io_memfs__extract(args1_matched[1]);
          }
          if (false === file_extracted) {
            debug('process.__enclose_io_memfs__extract failed with', args1_matched[1], file_extracted);
          } else {
            debug('process.__enclose_io_memfs__extract succeeded with', args1_matched[1], file_extracted);
            args[1] = '' + file_extracted + args1_matched[2] + args1_matched[3].split(' ').map(args_extract).join(' ');
            require('fs').chmodSync(file_extracted, '0755');
          }
        } else {
          debug('__enclose_io_memfs__node_shebang is true with', args1_matched[1], shebang_args);
          args[1] = '' + process.execPath + ' ' + shebang_args.trim() + ' ' + args1_matched[1] + args1_matched[2] + args1_matched[3].split(' ').map(args_extract).join(' ');
        }
      }
    }

  }

  args = args.map(args_extract);

  // allow reusing the package itself as an Node.js interpreter
  var flag_ENCLOSE_IO_USE_ORIGINAL_NODE = false;
  var command_outer = [file].concat(args).join(' ');
  var command_regexp_execPath = (process.execPath+'').replace(/[.?*+^$[\]\\(){}|-]/g, "\\$&");
  var command_regexp_json_execPath = (JSON.stringify(process.execPath)+'').replace(/[.?*+^$[\]\\(){}|-]/g, "\\$&");
  [
    new RegExp(`^${command_regexp_execPath}$`),
    new RegExp(`^${command_regexp_execPath}\\s`),
    new RegExp(`\\s${command_regexp_execPath}$`),
    new RegExp(`\\s${command_regexp_execPath}\\s`),
    new RegExp(`"${command_regexp_execPath}"`),
    new RegExp(`'${command_regexp_execPath}'`),
    new RegExp(command_regexp_json_execPath)
  ].forEach(function(element) {
    if (command_outer.match(element) !== null) {
      flag_ENCLOSE_IO_USE_ORIGINAL_NODE = true;
    }
  });
  // --------- [Enclose.IO Hack end] ---------

  if (options.shell) {
    const command = [file].concat(args).join(' ');

    if (process.platform === 'win32') {
      if (typeof options.shell === 'string')
        file = options.shell;
      else
        file = process.env.comspec || 'cmd.exe';
      args = ['/d', '/s', '/c', `"${command}"`];
      options.windowsVerbatimArguments = true;
    } else {
      if (typeof options.shell === 'string')
        file = options.shell;
      else if (process.platform === 'android')
        file = '/system/bin/sh';
      else
        file = '/bin/sh';
      args = ['-c', command];
    }
  }

  if (typeof options.argv0 === 'string') {
    args.unshift(options.argv0);
  } else {
    args.unshift(file);
  }

  var env = options.env || process.env;
  var envPairs = [];

  for (var key in env) {
    envPairs.push(key + '=' + env[key]);
  }

  _convertCustomFds(options);

  // --------- [Enclose.IO Hack start] ---------
  if (flag_ENCLOSE_IO_USE_ORIGINAL_NODE && undefined === env.ENCLOSE_IO_USE_ITSELF) {
    envPairs.push('ENCLOSE_IO_USE_ORIGINAL_NODE=1');
  }
  if (options.cwd) { // TODO no need to do this for ordinary paths
    envPairs.push(`ENCLOSE_IO_CHDIR=${options.cwd}`);
  }
  debug('normalizeSpawnArguments ends with', {
    file: file,
    args: args,
    options: options,
    envPairs: envPairs
  });
  // --------- [Enclose.IO Hack end] ---------

  return {
    file: file,
    args: args,
    options: options,
    envPairs: envPairs
  };
}


var spawn = exports.spawn = function(/*file, args, options*/) {
  var opts = normalizeSpawnArguments.apply(null, arguments);
  var options = opts.options;
  var child = new ChildProcess();

  debug('spawn', opts.args, options);

  child.spawn({
    file: opts.file,
    args: opts.args,
    cwd: options.cwd,
    windowsHide: !!options.windowsHide,
    windowsVerbatimArguments: !!options.windowsVerbatimArguments,
    detached: !!options.detached,
    envPairs: opts.envPairs,
    stdio: options.stdio,
    uid: options.uid,
    gid: options.gid
  });

  return child;
};

function spawnSync(/*file, args, options*/) {
  var opts = normalizeSpawnArguments.apply(null, arguments);

  var options = opts.options;

  var i;

  debug('spawnSync', opts.args, options);

  // Validate the timeout, if present.
  validateTimeout(options.timeout);

  // Validate maxBuffer, if present.
  validateMaxBuffer(options.maxBuffer);

  options.file = opts.file;
  options.args = opts.args;
  options.envPairs = opts.envPairs;

  // Validate and translate the kill signal, if present.
  options.killSignal = sanitizeKillSignal(options.killSignal);

  options.stdio = _validateStdio(options.stdio || 'pipe', true).stdio;

  if (options.input) {
    var stdin = options.stdio[0] = util._extend({}, options.stdio[0]);
    stdin.input = options.input;
  }

  // We may want to pass data in on any given fd, ensure it is a valid buffer
  for (i = 0; i < options.stdio.length; i++) {
    var input = options.stdio[i] && options.stdio[i].input;
    if (input != null) {
      var pipe = options.stdio[i] = util._extend({}, options.stdio[i]);
      if (isUint8Array(input)) {
        pipe.input = input;
      } else if (typeof input === 'string') {
        pipe.input = Buffer.from(input, options.encoding);
      } else {
        throw new TypeError(util.format(
          'stdio[%d] should be Buffer, Uint8Array or string not %s',
          i,
          typeof input));
      }
    }
  }

  var result = spawn_sync.spawn(options);

  if (result.output && options.encoding && options.encoding !== 'buffer') {
    for (i = 0; i < result.output.length; i++) {
      if (!result.output[i])
        continue;
      result.output[i] = result.output[i].toString(options.encoding);
    }
  }

  result.stdout = result.output && result.output[1];
  result.stderr = result.output && result.output[2];

  if (result.error) {
    result.error = errnoException(result.error, 'spawnSync ' + opts.file);
    result.error.path = opts.file;
    result.error.spawnargs = opts.args.slice(1);
  }

  util._extend(result, opts);

  return result;
}
exports.spawnSync = spawnSync;


function checkExecSyncError(ret) {
  if (ret.error || ret.status !== 0) {
    var err = ret.error;
    ret.error = null;

    if (!err) {
      var msg = 'Command failed: ';
      msg += ret.cmd || ret.args.join(' ');
      if (ret.stderr && ret.stderr.length > 0)
        msg += '\n' + ret.stderr.toString();
      err = new Error(msg);
    }

    util._extend(err, ret);
    return err;
  }

  return false;
}


function execFileSync(/*command, args, options*/) {
  var opts = normalizeSpawnArguments.apply(null, arguments);
  var inheritStderr = !opts.options.stdio;

  var ret = spawnSync(opts.file, opts.args.slice(1), opts.options);

  if (inheritStderr && ret.stderr)
    process.stderr.write(ret.stderr);

  var err = checkExecSyncError(ret);

  if (err)
    throw err;

  return ret.stdout;
}
exports.execFileSync = execFileSync;


function execSync(command /*, options*/) {
  var opts = normalizeExecArgs.apply(null, arguments);
  var inheritStderr = !opts.options.stdio;

  var ret = spawnSync(opts.file, opts.options);
  ret.cmd = command;

  if (inheritStderr && ret.stderr)
    process.stderr.write(ret.stderr);

  var err = checkExecSyncError(ret);

  if (err)
    throw err;

  return ret.stdout;
}
exports.execSync = execSync;


function validateTimeout(timeout) {
  if (timeout != null && !(Number.isInteger(timeout) && timeout >= 0)) {
    throw new TypeError('"timeout" must be an unsigned integer');
  }
}


function validateMaxBuffer(maxBuffer) {
  if (maxBuffer != null && !(typeof maxBuffer === 'number' && maxBuffer >= 0)) {
    throw new TypeError('"maxBuffer" must be a positive number');
  }
}


function sanitizeKillSignal(killSignal) {
  if (typeof killSignal === 'string' || typeof killSignal === 'number') {
    return convertToValidSignal(killSignal);
  } else if (killSignal != null) {
    throw new TypeError('"killSignal" must be a string or number');
  }
}
