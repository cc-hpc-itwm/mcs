The block device aggregates multiple (remote) storages into a single space of blocks of fixed size. Blocks are identified by a single number.

EXAMPLE: A block device with 51 blocks aggregated from 4 different storages of size 8, 8, 5 and 30 respectively: The block with the number 18 is the block 2 in storage s_2.

```
   s_0     s_1     s_2  s_3
   :       :       :    :
   [-------)       :    :
   01234567[-------)    :
           01234567[----)
                   01234[-----------------------------)
                     ^  0         1         2
                     ^  012345678901234567890123456789
                     ^
   0         1       ^ 2         3         4         5
   012345678901234567890123456789012345678901234567890
```

The block device provides methods to transfer blocks between the block device storage and a local buffer storage.

There is a C++ interface and a command line interface. Both interfaces are compatible and can be used independently and together, e.g. applications using  one interface can interact with applications using the same interface and, at the same time, with applications using the other interface.

C++ interface:
==============

Storage:
--------

A single storage in the network of storages that the block device aggregates.

meta_data/{Provider, Client}:
-----------------------------

The `meta_data/Provider` is the central entry point of a block device. Storages can be added and the location of a block with a given id can be retrieved from the provider. The provider knowns about the block size, the number of blocks and the list of blocks in the block device. Ranges of blocks can be removed from the block device. See [Blocks](include/mcs/block_device/meta_data/Blocks.hpp) for more details about the semantics of the commands handled by the provider.

The `meta_data/Client` allows to use a remote `meta_data/Provider`.

Block:
------

A block in heap memory. A block has a transport address in the block device and provides direct access to the data. Please note that the knowledge of the transport address allows to access the block content using standard core layer functionality and independent from the transport functionality of the block device.

Buffer:
-------

Applications typically process arbitrary many blocks in a fixed amount of space. In the same heap memory many blocks are read from the block device, processed and maybe written back to the block device. The block buffer provides functionality to manage a fixed number of buffered blocks within some storage.

Reader/Writer:
--------------

The reader and writer class are helpers to read blocks from the block device into a buffer and to write blocks from a buffer into the block device.

Command line interface:
=======================

See [test/block_device/bin](../test/block_device/bin/test.sh.in) for an extended example usage of the command line interface.

1. Start a meta data provider:
------------------------------

Each provider stores its connection information in a "provider path". Clients will use that very same path to identify a specific provider.

```
export MCS_BLOCK_DEVICE_PROVIDER_PATH=$(mktemp -d)/PROVIDER
```

Providers can use either `ip::tcp` or `local::stream_protocol` for communication. They use a number of threads to handle requests from multiple clients. Note that providers with a single thread can still serve multiple clients in parallel using cooperative multi-tasking. Using more than one thread is only necessary when the number of concurrent clients is huge.

EXAMPLE: Start a provider with a single thread for blocks of size 4 MiByte. The provider communicates via `ip::tcp` and the kernel assigns a port:

```
${MCS_INSTALL_DIR}/bin/mcs_block_device_bin_meta_data_provider        \
  'ip::tcp()'                                                         \
  ${MCS_BLOCK_DEVICE_PROVIDER_PATH:?}                                 \
  "mcs::block_device::block::Size ($((4*2**20)))"                     \
  1
```

EXAMPLE: Start a provider with a single thread for blocks of size 4 MiByte. The provider communicates via `ip::tcp` on port `8765`:

```
${MCS_INSTALL_DIR}/bin/mcs_block_device_bin_meta_data_provider        \
  'ip::tcp ("127.0.0.1", 8765)'                                       \
  ${MCS_BLOCK_DEVICE_PROVIDER_PATH:?}                                 \
  "mcs::block_device::block::Size ($((4*2**20)))"                     \
  1
```

EXAMPLE: Start a provider with a single thread for blocks of size 4 MiByte. The provider communicates via `local::stream_protocol` with the socket name `/tmp/MCS_BLOCK_DEVICE`:

```
${MCS_INSTALL_DIR}/bin/mcs_block_device_bin_meta_data_provider        \
  'local::stream_protocol ("/tmp/MCS_BLOCK_DEVICE")'                  \
  ${MCS_BLOCK_DEVICE_PROVIDER_PATH:?}                                 \
  "mcs::block_device::block::Size ($((4*2**20)))"                     \
  1
```

Check that the service is running and has no blocks available:

```
> ${MCS_INSTALL_DIR}/bin/mcs_block_device_bin_block_size              \
    ${MCS_BLOCK_DEVICE_PROVIDER_PATH:?}
mcs::block_device::block::Size (4194304)
```

```
> ${MCS_INSTALL_DIR}/bin/mcs_block_device_bin_number_of_blocks        \
    ${MCS_BLOCK_DEVICE_PROVIDER_PATH:?}
mcs::block_device::block::Count (0)
```

```
> ${MCS_INSTALL_DIR}/bin/mcs_block_device_bin_blocks                  \
    ${MCS_BLOCK_DEVICE_PROVIDER_PATH:?}
[]
```

2. Add storage to the aggregation:
----------------------------------

Multiple storages of any type can be added to the aggregation. The command line interface provides the `storage_provider` tool that creates, owns and registers storage. The `storage_provider` itself is an ordinary storage provider using an arbitrary transport protocol. The `storage_provider` publishes its connection information and can be used independent from the block device.

EXAMPLE: First create a single threaded Heap storage of size 1 GiB that uses `ip::tcp()` for data transport and stores its connection information in `/tmp/bd-heap`:

```
> ${MCS_INSTALL_DIR}/bin/mcs_block_device_bin_storage_provider        \
    ${MCS_BLOCK_DEVICE_PROVIDER_PATH}                                 \
    'ip::tcp()'                                                       \
    /tmp/bd-heap                                                      \
    1                                                                 \
    $((2**30))                                                        \
    'Heap (Unlimited)'
new blocks: mcs::block_device::block::Range (mcs::block_device::block::ID (0), mcs::block_device::block::ID (256))
```

The output tells about the new block ids. Check that the service has the correct information:

```
> ${MCS_INSTALL_DIR}/bin/mcs_block_device_bin_number_of_blocks        \
    ${MCS_BLOCK_DEVICE_PROVIDER_PATH:?}
mcs::block_device::block::Count (256)
```

```
> ${MCS_INSTALL_DIR}/bin/mcs_block_device_bin_blocks                  \
    ${MCS_BLOCK_DEVICE_PROVIDER_PATH:?}
[mcs::block_device::block::Range (mcs::block_device::block::ID (0), mcs::block_device::block::ID (256))]
```

Second create a single threaded Files storage also of size 1 GiB that also uses `ip::tcp()` for data transport and stores its connection information in `/tmp/bd-files` and data in `/tmp/bs-files-storage`:

```
> mkdir -p /tmp/bs-files-storage
> ${MCS_INSTALL_DIR}/bin/mcs_block_device_bin_storage_provider        \
    ${MCS_BLOCK_DEVICE_PROVIDER_PATH}                                 \
    'ip::tcp()'                                                       \
    /tmp/bd-heap                                                      \
    1                                                                 \
    $((2**30))                                                        \
    'Files (Prefix "/tmp/bd-files-storage", Unlimited)'
new blocks: mcs::block_device::block::Range (mcs::block_device::block::ID (256), mcs::block_device::block::ID (512))
```

Now the block device contains 512 blocks, half of them in Heap and half of them in Files storage.

3. Storage data in a block:
---------------------------

To storage data from file `IN` in the block with the number 300 use:

```
cat IN                                                              | \
  ${MCS_INSTALL_DIR}/bin/mcs_block_device_bin_write                   \
    ${MCS_BLOCK_DEVICE_PROVIDER_PATH}                                 \
    'mcs::block_device::block::ID (300)'
```

The old content of the block is overwritten with the input data. If the data is too large or too small, then the write command will issue warnings.

4. Retrieve data from a block:
------------------------------

The `cat` tool retrieves the data stored in a block and outputs it on the standard output channel:

```
${MCS_INSTALL_DIR}/bin/mcs_block_device_bin_cat                       \
    ${MCS_BLOCK_DEVICE_PROVIDER_PATH}                                 \
    'mcs::block_device::block::ID (300)'                              \
    > OUT
```

Check that the data has not changed

```
diff IN OUT || echo "FAILURE, data differs."
```

5a) Remove complete storages:
-----------------------------

By stopping a `storage_provider` (e.g. by sending a SIGINT) the storage is taken out of the aggregation.

EXAMPLE: If the Heap provider from the example above is stopped, the output is

```
reclaimed [mcs::block_device::Storage (ip::tcp (...), bi_0, Parameter [], sg_0, [of_0..of_1073741824))]
```

5b) Remove some blocks:
-----------------------

To remove some blocks but not the complete storage provider is also possible.

EXAMPLE: To remove the blocks with the ids between 400 and 500 use:

```
> ${MCS_INSTALL_DIR}/bin/mcs_block_device_bin_remove                  \
    ${MCS_BLOCK_DEVICE_PROVIDER_PATH}                                 \
    'mcs::block_device::block::Range (mcs::block_device::block::ID (400), mcs::block_device::block::ID (500))'
[mcs::block_device::Storage (ip::tcp (...), bi_0, Parameter [], sg_1, [of_603979776..of_1023410176))]
```

After 5a and 5b, the block device now has 156 blocks left:

```
> ${MCS_INSTALL_DIR}/bin/mcs_block_device_bin_number_of_blocks        \
    ${MCS_BLOCK_DEVICE_PROVIDER_PATH:?}
mcs::block_device::block::Count (156)
```

```
> ${MCS_INSTALL_DIR}/bin/mcs_block_device_bin_blocks                  \
    ${MCS_BLOCK_DEVICE_PROVIDER_PATHv:?}
[mcs::block_device::block::Range (mcs::block_device::block::ID (256), mcs::block_device::block::ID (400)), mcs::block_device::block::Range (mcs::block_device::block::ID (500), mcs::block_device::block::ID (512))]
```
