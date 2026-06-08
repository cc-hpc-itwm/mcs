# v0.8.1

## Cleanup and Fixes

- Make the fair mutex standard conform, it can now be used with
  std::{unique, shared}_lock
- fix: test cleanup for block device no longer recurses
- fix: test daemonize now properly wait
- fix: fuse session shutdown event loop is kept alive until after
  unmount
- fix: fuse destruction is deferred until nlookup and st_nlink both
  are zero
- fix: serialization support for multi-byte strings

## Meta

- Drop support for Ubuntu 25.10
- Add support for Ubuntu 26.04
- Add support for Alpine Linux 3.23
- Add support for Clang 22
- Bump asio to 1.38.0
- Bump libfuse to 3.18.2

# v0.8

## Feature

- Add FUSE presentation layer for MCS storage. Present MCS storage content as a POSIX filesystem using libfuse3. Depends on [libfuse3](https://github.com/libfuse/libfuse) version 3.18.1 or later and is enabled by `-DMCS_FUSE=ON`. The kernel must support FUSE and have the fuse module loaded. See [fuse/bin/storage/README.md](fuse/bin/storage/README.md) for usage details and [fuse/include/mcs/fuse/ARCHITECTURE.md](fuse/include/mcs/fuse/ARCHITECTURE.md) for architecture documentation.

# v0.7

## Feature

- Add support for libfabric in the transport layer. Depends on [libfabric](https://github.com/ofiwg/libfabric) and is enabled by `-DMCS_TRANSPORT_LIBFABRIC=ON`. See [libfabric/README.d](core/include/mcs/core/transport/implementation/libfabric/README.md) for more details.

# v0.6

## API

- Removed iov_backend: Part of project https://gwdg.de/en/projects/mcse/.
- Install headers for `core`, `block_device` and `share_service`
