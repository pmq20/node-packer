# Node.js Compiler Road Map

## v1.x

- Eliminate dependending on an outside Node.js and npm when compiling
- Add options to generate installers
- Support writing options down to package.json
  - select the correct Node.js version via `engines` of package.json
    - Support arbitrary Node.js runtime versions
    - https://github.com/pmq20/node-compiler/issues/40
  - configure auto-update to enable/disable prompts when new versions were detected
  - enable/disable auto-update
- Add options to select items to deliver
  - opt out zlib/openssl for system libraries
  - Incl and ICU
  - debug facilities
- Add options to select compression-method
  - optionally xz the final product
- Detect simultaneous runs of nodec
  - https://github.com/pmq20/node-compiler/issues/31
- Warn the user that some symbolic link links to the outside of the project
  - Add a check procedure at compile time
  - https://github.com/pmq20/node-compiler/issues/37
- Use a temporary directory name with nodec version when compiling
  - https://github.com/pmq20/node-compiler/issues/42
- Cross-compile
  - https://github.com/pmq20/node-compiler/pull/36
- Make Docker images for compiler environments
- Generate Windows-less Cmd-less Windows applications via /subsystem=windows
- a option to exclude directory's and files.
  - https://github.com/pmq20/node-compiler/issues/51

## v2.x

- Compile to bytecode via v8 Ignition
- Support ARM architecture
- Support cross compiling
- Drop the external dependency of mksquashfs
- Support library only projects
  - https://github.com/pmq20/node-compiler/issues/39
