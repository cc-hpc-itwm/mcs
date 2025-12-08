# v0.7

## Feature

- Add support for libfabric in the transport layer. Depends on [libfabric](https://github.com/ofiwg/libfabric) and is enabled by `-DMCS_TRANSPORT_LIBFABRIC=ON`. See [libfabric/README.d](core/include/mcs/core/transport/implementation/libfabric/README.md) for more details.

# v0.6

## API

- Removed iov_backend: Part of project https://gwdg.de/en/projects/mcse/.
- Install headers for `core`, `block_device` and `share_service`
