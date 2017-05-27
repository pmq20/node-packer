# Node.js Compiler Road Map

## v1.x

- Drop the external dependency of mksquashfs
- Support writing those options down to package.json
- Add options to select statically-linked items to deliver
- Add options to select compression-method
- Add options to generate installers
- detect simultaneous runs of nodec
  - https://github.com/pmq20/node-compiler/issues/31
- add a check procedure at compile time to warn the user that some symbolic link links to the outside of the project
  - https://github.com/pmq20/node-compiler/issues/37

## v2.0.0

- Compile to bytecode via v8 Ignition
- Support ARM architecture
- Support cross compiling
