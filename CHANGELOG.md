# v0.9

## Feature

- Daemon programs now run in the background after setup has completed.
  Use `--mcs-daemon-foreground` anywhere in the command line to keep
  them in the foreground.
- The generic FUSE filesystem adds support for content types `Vector`,
  `Realloc` which store data in memory, and `Storage<Virtual>` that
  dynamically loads a storage implementation.
- Renamed `mcs_fuse_bin_storage_fs` to
  `mcs_fuse_bin_fs`. Storage-backed invocations use
  `--content Storage` with named storage parameters.

## API

- Serialization removes unused support for intrusive serialization.
- Command-line parsing and argument helpers are provided by the
  separate `mcs_util_command_line` library.
- Production tools use named parameters.
- FUSE options belong in repeatable `'[[FUSE' ... 'FUSE]]'`
  embedded sections.
- Named-option schemas and parsed results live in `mcs::util::command_line`;
  flag and parameter descriptors live in `mcs::util::command_line::option`.
  Their headers mirror these namespaces under `mcs/util/command_line/`.
  Argument storage remains in `mcs::util::arguments`.
- Production tools support `-h` and `--help` to display a help screen.
- Integer range failures in `mcs::util::cast` throw
  `mcs::util::CastOutOfRange`. Diagnostic messages are unchanged.
- Parser `mcs::core::storage::selection::select` for storage
  configurations from a list of supported storage implementations.
- `mcs::fuse::content::selection::select` for selecting `Vector`,
  `Realloc`, or `Storage` fuse content.
- `rpc::Provider::stop_accepting()` for asynchronous listener shutdown
  without terminating established connections.
- `rpc::Client::shutdown()` for asynchronous, shared, idempotent
  connection shutdown and draining of pending operations.
- RPC provider handler arguments are constrained to non-reference
  types by `rpc::is_handler_arg`.
- `share_service::Provider` now borrows storage from its caller, which
  must keep the storage alive for the provider's use.
- RPC thread-count parsing and formatting now use
  `ScopedIOContext::NumberOfThreads (N)` instead of
  `ScopedRunningIOContext::NumberOfThreads N`. Update command-line
  arguments and persisted text to the new name and tuple syntax.
- Serialization for `std::unordered_set`.
- `util::AbsolutePath` that throws when a path is not absolute.
- `util::Arguments` are now `util::arguments::View` and
  `util::arguments::Owned` has been added.
- `util::is_visitor` as concept for `std::visit` compatible visitors
  now requires the same invocation result
- Aggregate structs uniformly use names without leading underscore.
- More syscalls wrappers.
- `util::read` support for signed integers and protection from
  overflows.
- `util::read::split_at_last_whitespace` splits a string view into
  non-empty fields at its final whitespace.

## Cleanup and Fixes

- fix: ASIO demo provider selects IPv4 instead of IPv6 when `v4`
  is requested.
- fix: retry transient RPC provider accept failures, back off on
  resource exhaustion, suppress expected shutdown diagnostics, and
  report terminal listener failures through `Provider::stop_accepting()`
- fix: keep RPC provider connection state alive until pending handlers
  complete after a client disconnect
- fix: keep ASIO put source mappings alive until queued transmission
  completes
- fix: RPC non-streaming handlers no longer prevent the same connection
  from receiving subsequent requests while a handler is blocked
- fix: always zero-initialize Heap segments to avoid exposing previous
  allocator contents
- no longer define fmt instances for std types, use custom wrappers
  or instances defined in fmt/std.h
- fix: documentation and README
- fix: fuse tests now wait for the mountpoint to disappear
- fix: avoid exceptions to escape from callbacks in rpc and share_service
- fix: make sure resize and write to fuse mounted files always
  properly zeros gaps and new bytes to avoid leakage
- fix: make sure fuse mounted file sizes are consistent even when
  reallocation fails
- fuse: no longer publish the internal storage capacity assigned for a
  file in the file's extended attributes, was never part of the public
  contract
- fuse: remove support for fallocate: was never enabled and never
  reachable by users
- fix: shorten test temporary-directory names to keep within platform
  limits

## Meta

- Bump Alpine Linux to 3.22.8, 3.23.8, and 3.24.4
- Drop support for Alpine Linux 3.21
- Add support for Alpine Linux 3.24
- Add support for Clang 23
- Add support for C++26
- Bump asio to 1.39.0
- Bump fmt to 12.2.1
- Bump gtest to 1.18.0
- Bump libfuse to 3.18.3

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
