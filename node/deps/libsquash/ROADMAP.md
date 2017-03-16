# Libsquash Road Map

## v1.0.0

- organize core API's and freeze
- make struct's opaque
- seperate headers into private headers and public headers
- make intercepting system calls a core functionality instead of in the sample/
- test under ARM architecture as well

## v2.0.0

- embed mksquashfs logic instead of relying on outside tool
- intercept dynamically without compiling via the LD_PRELOAD trick
