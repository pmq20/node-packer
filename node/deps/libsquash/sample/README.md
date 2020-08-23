This sample demonstrates on how to use libsquash in an unobtrusive way.
File system calls on paths starting with `/__enclose_io_memfs__` are redirected to libsquash,
while others are kept with the original system calls.

Code of this sample are effectively used by
[Node.js Packer](https://github.com/pmq20/node-packer)
and [Ruby Packer](https://github.com/pmq20/ruby-packer).
