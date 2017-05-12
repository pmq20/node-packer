# Libsquash Road Map

## v1.0.0

- Benchmark and tweet cache size and block size of libsquash
- Organize core API's and freeze
- Make public struct's opaque
- Seperate headers into private headers and public headers
- Make intercepting system calls a core functionality instead of in the sample/
- Test under ARM architecture as well
- Support Unicode paths

## v2.0.0

- Embed mksquashfs logic instead of relying on outside tool
- Intercept dynamically without compiling via the LD_PRELOAD trick
