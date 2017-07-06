# Node.js Compiler Road Map

## v1.x

- Support writing options down to package.json
  - select the correct Node.js version via `engines` of package.json
    - Support arbitrary Node.js runtime versions
    - https://github.com/pmq20/node-compiler/issues/40
  - configure auto-update to enable/disable prompts when new versions were detected
  - enable/disable auto-update
  - a option to exclude directory's and files.
    - https://github.com/pmq20/node-compiler/issues/51
- Be able to use a custom icon and file description for the executable output. maybe an icon file in the package root directory.
  - https://github.com/pmq20/node-compiler/issues/54
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
- Cross-compile
  - https://github.com/pmq20/node-compiler/pull/36
  - Support ARM architecture
  - https://github.com/pmq20/node-compiler/issues/26
- Make Docker images for compiler environments
- Generate Windows-less Cmd-less Windows applications via /subsystem=windows

## v2.x

- Compile to bytecode via v8 Ignition
  - https://github.com/nodejs/node/issues/11842
- Drop the external dependency of mksquashfs
  - i.e. Give libsquash the ability to mksquashfs
- Support library only projects
  - https://github.com/pmq20/node-compiler/issues/39
