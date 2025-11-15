The share service allows to zero-copy share data between applications running on the same machine. There is a C++ interface and a command line interface. Both interfaces are compatible can be used independently and together, e.g. applications using one interface can interact with applications using the same interface and, at the same time, with applications using the other interface.

The share service can store data in heap memory or in files, using the Heap or the Files storage. If the Files storage is used with a shared file system, then the share service can share data even across multiple machines.

C++ interface:
==============

Chunk:
------

Self contained description of a piece of memory. Can be serialized, stored in files and shared with other applications.

Provider:
---------

Publishes local storages to be used by the share service. Handles commands to create and to remove chunks. Also allows to "attach" to a chunk that has been created by some other entity. To attach to a chunk means to get direct access to it.

Client:
-------

Offers commands to create, remove and attach to chunks managed by a specific provider.


Command line interface:
=======================

1. Start a provider:
--------------------

Each provider stores its connection information in a "provider path". Clients will use that very same path to identify a specific provider.

```
export MCS_SHARE_SERVICE_PROVIDER_PATH=$(mktemp -d)/PROVIDER
```

Providers can use either `ip::tcp` or `local::stream_protocol` for communication. They use a number of threads to handle requests from multiple clients. Note that providers with a single thread can still serve multiple clients in parallel using cooperative multi-tasking. Using more than one thread is only necessary when the number of concurrent clients is huge.

EXAMPLE: Start a provider that communicates via `ip::tcp` and let the kernel assign a port:

```
${MCS_INSTALL_DIR}/bin/mcs_share_service_bin_provider                 \
  'ip::tcp()'                                                         \
  ${MCS_SHARE_SERVICE_PROVIDER_PATH:?}                                \
  'ScopedRunningIOContext::NumberOfThreads 1'
```

EXAMPLE: Start a provider that communicates via `ip::tcp` on port `8765`:

```
${MCS_INSTALL_DIR}/bin/mcs_share_service_bin_provider                 \
  'ip::tcp ("127.0.0.1", 8765)'                                       \
  ${MCS_SHARE_SERVICE_PROVIDER_PATH:?}                                \
  'ScopedRunningIOContext::NumberOfThreads 1'
```

EXAMPLE: Start a provider that communicates via `local::stream_protocol` with the socket name `/tmp/MCS_SHARE_SERVICE`:

```
${MCS_INSTALL_DIR}/bin/mcs_share_service_bin_provider                 \
  'local::stream_protocol ("/tmp/MCS_SHARE_SERVICE")'                 \
  ${MCS_SHARE_SERVICE_PROVIDER_PATH:?}                                \
  'ScopedRunningIOContext::NumberOfThreads 1'
```

2. Create a chunk:
------------------

Chunks of certain size can be created either in shared memory or in a file. Clients use the provider path to identify a specific provider. The output of the chunk creation is a serialized form of the chunk that can be stored in files and contains all information to identify the chunk.

EXAMPLE: Create a chunk of size `32 MiB` in shared memory, using the prefix `share_service_chunk` and store it in `chunk`:

```
${MCS_INSTALL_DIR}/bin/mcs_share_service_bin_create                   \
  ${MCS_SHARE_SERVICE_PROVIDER_PATH:?}                                \
  $((32*2**20))                                                       \
  'SHMEM (Prefix "share_service_chunk", Unlimited)'                   \
  > chunk
```

The file `chunk` will contain something like

```
> cat chunk
share_service::Chunk (ip::tcp (Address "127.0.0.1", 8765), bi_0, type_id_1, sg_0, sz_33554432)
```

and some shared memory will be allocated

```
> ls /dev/shm/
'"share_service_chunk".sg_0'
> stat --format='%s' /dev/shm/\"share_service_chunk\".sg_0
33554432
```

EXAMPLE: Create a chunk of size `32 MiB` in a file in the folder `/tmp/share_service_storage` and store it in `chunk`:

```
mkdir -p /tmp/share_service_storage
${MCS_INSTALL_DIR}/bin/mcs_share_service_bin_client                   \
  ${MCS_SHARE_SERVICE_PROVIDER_PATH:?}                                \
  $((32*2**20))                                                       \
  'Files (Prefix "/tmp/share_service_storage", Unlimited'             \
  > chunk
```

The file `chunk` will contain something like

```
> cat chunk
share_service::Chunk (ip::tcp (Address "127.0.0.1", 8765), bi_5, type_id_0, sg_0, sz_33554432)
```

and a file will be created

```
> ls /tmp/share_service_storage/
sg_0
> stat --format='%s' /tmp/share_service_storage/sg_0
33554432
```

3. Store some data in a chunk:
------------------------------

To interact with the content of the chunk, the provider path is no longer required as the chunk is self contained. Given there is some data in the file `IN` and a chunk information has been stored in the file `chunk`, the data can be stored in the chunk using

```
cat IN | ${MCS_INSTALL_DIR}/bin/mcs_share_service_bin_write "$(cat chunk)"
```

4. Retrieve data from a chunk:
------------------------------

The `cat` tool retrieves the data stored in a chunk and outputs it on the standard output channel:

```
${MCS_INSTALL_DIR}/bin/mcs_share_service_bin_cat "$(cat chunk)" > OUT
```

Check that the data has not changed

```
diff IN OUT || echo "FAILURE, data differs."
```

5. Remove a chunk:
------------------

```
${MCS_INSTALL_DIR}/bin/mcs_share_service_bin_remove "$(cat chunk)"
```
