# Node.js Compiler Road Map

## v1.x

- Add options to select statically-linked items to deliver
- Add options to select compression-method
- Add options to generate installers
- Support writing options down to package.json
- Detect simultaneous runs of nodec
  - https://github.com/pmq20/node-compiler/issues/31
- Warn the user that some symbolic link links to the outside of the project
  - Add a check procedure at compile time
  - https://github.com/pmq20/node-compiler/issues/37
- Support arbitrary node.js runtime versions
  - https://github.com/pmq20/node-compiler/issues/40

## v2.x

- Compile to bytecode via v8 Ignition
- Support ARM architecture
- Support cross compiling
- Drop the external dependency of mksquashfs
- Support library only projects
  - https://github.com/pmq20/node-compiler/issues/39
