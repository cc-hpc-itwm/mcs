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
