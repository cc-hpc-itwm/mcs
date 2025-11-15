Intro:
======

The command line interface consists of two set of tools: The direct interface and the IOV-based interface.

The direct interface allows for creation and direct management of an instance of the mcs-iov-backend and is independent from the libioverbs.

The IOV-based interface, in contrast, uses the libioverbs to operate on data and to manage a running instance of the mcs-iov-backend.

## The direct interface consists of the tools:

- Startup:
  * `mcs_iov_backend_provider`: Starts a backend provider. This is the central entry point for an instance of the mcs-iov-backend.
  * `mcs_iov_backend_storage_provider`: Starts a storage provider and registers the storage with a running backend provider. The mcs-iov-backend supports any number of storage providers. Storage provider with different storage implementations can be mixed freely.

- Persistency:
  * `mcs_iov_backend_state`: Prints the backend provider state. That state allows to re-start a backend provider, also at a different location.

- Management of collections:
  * `mcs_iov_backend_collection_create`: Creates a collection.
  * `mcs_iov_backend_collection_delete`: Deletes a collection.

- Observation:
  * `mcs_iov_backend_list_storages`: Lists the storages that are part of the running mcs-iov-backend.
  * `mcs_iov_backend_list_collections`: Lists the collections.
  * `mcs_iov_backend_locations`: Lists detailed information about the distribution of a collection across the storages.
  * `mcs_iov_backend_range`: Prints the memory range of a collection.

- Import and export of shared data:
  * `mcs_iov_backend_export_shared`: Exports data from the mcs-iov-backend, offloading the copy operations to the storage servers.
  * `mcs_iov_backend_import_shared`: Imports data into the mcs-backend, offloading the copy operations to the storage servers.

- Connection with the libioverbs:
  * `mcs_iov_backend_make_parameter`: Creates a configuration for a running mcs-iov-backend that can be used by the libioverbs to initialize the connection to the running mcs-iov-backend.

## The IOV-based interface consists of the tools:

All IOV-based tools require a configuration file for the running mcs-iov-backend and a file that contains a meta-data database. The configuration shall contain the output of a call to `mcs_iov_backend_make_parameter`.

Please note that the direct interface from above has no knowledge of the database used by the IOV-based tools and there is the risk that the database content and the state of the mcs-iov-backend are inconsistent. Be careful when using, at the same time, both, the direct and the IOV-based interface.

- Database management:
  * `mcs_iov_backend_iov_add_entry_to_database`: Adds an entry to a database.
  * `mcs_iov_backend_iov_print_database`: Prints the content of a database.
  * `mcs_iov_backend_iov_make_empty_database`: Create an empty database.
  * `mcs_iov_backend_iov_remove_entry_from_database`: Removes an entry from a database.

- Collection management:
  * `mcs_iov_backend_iov_collection_create`: Creates a collection.
  * `mcs_iov_backend_iov_collection_delete`: Deletes a collection.
  * `mcs_iov_backend_iov_collection_exists`: Checks whether or not the collection exists.
  * `mcs_iov_backend_iov_collection_uuid`: Returns the libioverbs identifier of the collection.

- Importing and exporting data:
  * `mcs_iov_backend_iov_cat`: Exports data to the standard output.
  * `mcs_iov_backend_iov_export`: Exports data to a file.
  * `mcs_iov_backend_iov_import`: Imports data from a file.
  * `mcs_iov_backend_iov_write`: Imports data from the standard input.

The rest of this document consists of example scenarios and how they can be implemented using the command line tools.

Scenario 1: Start and use some storages, maybe non-persistent
=============================================================

### 0. Change into the binary directory

```bash
cd $MCS_INSTALL/bin
```

### 1. Choose some endpoint for the provider:

EXAMPLE: TCP, kernel chooses port

```bash
MCS_IOV_PROVIDER_ENDPOINT='ip::tcp()'
```

EXAMPLE: TCP, port given

```bash
MCS_IOV_PROVIDER_ENDPOINT='ip::tcp ("0.0.0.0", 9876)'
```

EXAMPLE: stream_protocol, some non-existing file for communication

```bash
MCS_IOV_PROVIDER_ENDPOINT='local::stream_protocol ("/tmp/MCS_IOV_PROVIDER_EP")'
```

### 2. Start the provider (with 1 thread) and save the connection information in a file:

```bash
MCS_IOV_PROVIDER_CONNECTABLE='/tmp/MCS_IOV_PROVIDER_CONNECTABLE'
stdbuf -oL                                                                 \
  ./mcs_iov_backend_provider                                               \
    "${MCS_IOV_PROVIDER_ENDPOINT:?}" 1                                   | \
      tee "${MCS_IOV_PROVIDER_CONNECTABLE:?}"
```

### 3. Add Storages to the provider

EXAMPLE: Add a Heap storage for at most 1GiB with storages provider using `1` thread and transport provider using `4` threads. Storages provider and transport provider use TCP and let the kernel decide about the port:

```bash
./mcs_iov_backend_storage_provider                                         \
  "Just ($(cat ${MCS_IOV_PROVIDER_CONNECTABLE:?}))"                        \
  'ip::tcp()' 1                                                            \
  'ip::tcp()' 4                                                            \
  "Heap (Limit $((2**30)))"                                                \
  "Heap::Size::Max()"                                                      \
  "Heap::Size::Used()"                                                     \
  'Heap::Segment::Create (Nothing)'                                        \
  'Heap::Segment::Remove()'                                                \
  'Heap::Chunk::Description()'                                             \
  'Heap::File::Read()'                                                     \
  'Heap::File::Write()'
```

EXAMPLE: Add a Files storage for at most 50 GiB that stores data in `/tmp/FILES` with storages provider using 1 thread and transport provider using `2` threads. Storages provider and transport provider use TCP and let the kernel decide about the port:

```bash
./mcs_iov_backend_storage_provider                                          \
  "Just ($(cat ${MCS_IOV_PROVIDER_CONNECTABLE:?}))"                         \
  'ip::tcp()' 1                                                             \
  'ip::tcp()' 2                                                             \
  "Files (Prefix \"/tmp/FILES\", Limit $((50*2**30)))"                      \
  "Files::Size::Max()"                                                      \
  "Files::Size::Used()"                                                     \
  'Files::Segment::Create (Files::Segment::OnRemove::Remove())'             \
  'Files::Segment::Remove (Nothing)'                                        \
  'Files::Chunk::Description()'                                             \
  'Files::File::Read()'                                                     \
  'Files::File::Write()'
```

EXAMPLE: Add a SHMEM storage for at most `2 GiB` that stores data in with the prefix `IOV_BACKEND_SHMEM` with storages provider using `1` thread and transport provider using `2` threads. Storages provider and transport provider use TCP and let the kernel decide about the port. The AccessMode is `784` and the memory is not going to be mlocked:

```bash
./mcs_iov_backend_storage_provider                                         \
  "Just ($(cat ${MCS_IOV_PROVIDER_CONNECTABLE:?}))"                        \
  'ip::tcp()' 1                                                            \
  'ip::tcp()' 2                                                            \
  "SHMEM (Prefix \"IOV_BACKEND_SHMEM\", Limit $((2*2**30)))"               \
  "SHMEM::Size::Max()"                                                     \
  "SHMEM::Size::Used()"                                                    \
  'SHMEM::Segment::Create (SHMEM::Segment::AccessMode (784), Nothing)'     \
  'SHMEM::Segment::Remove()'                                               \
  'SHMEM::Chunk::Description()'                                            \
  'SHMEM::File::Read()'                                                    \
  'SHMEM::File::Write()'
```

There is no limit on the number of storages added to a single provider.

### 4. Verify the addition of the storages by calling

```bash
./mcs_iov_backend_list_storages "$(cat ${MCS_IOV_PROVIDER_CONNECTABLE:?})"
```

The output is a list of all added storages with their types and
parameters.

### 5. Create collections:

EXAMPLE: Create a collection that can store at most `10 MiB` and has the collection id `101`:

```bash
./mcs_iov_backend_collection_create                                        \
  "$(cat ${MCS_IOV_PROVIDER_CONNECTABLE:?})"                               \
  101                                                                      \
  $((10*2**20))
```

The output tells the stripe pattern.

There is no limit on the number of storages added to a single provider except that the memory will be exhausted at some point.

Please note that collections are distributed equally across all storages, so the smallest storage determines the total amount of memory.

### 6. Verify the creation of collections:

```bash
./mcs_iov_backend_list_collections "$(cat ${MCS_IOV_PROVIDER_CONNECTABLE:?})"
```

The output lists all collections and their stripes.

### 7. Use:

EXAMPLE: Create a parameter struct that can be written into a file and used to start the `libioverbs_mcs_iov_backend:`

```bash
./mcs_iov_backend_make_parameter "$(cat ${MCS_IOV_PROVIDER_CONNECTABLE:?})"
```

EXAMPLE: Retrieve the total memory range of the collection with the id `101`:

```bash
./mcs_iov_backend_range                                                    \
  "$(cat ${MCS_IOV_PROVIDER_CONNECTABLE:?})"                               \
  101
```

EXAMPLE: Retrieve the stripes of the range `[500..15000)` of the collection with the id `101`:

```bash
./mcs_iov_backend_locations                                                \
  "$(cat ${MCS_IOV_PROVIDER_CONNECTABLE:?})"                               \
  101                                                                      \
  '[500..15000)'
```


Scenario 2: Start and restart persistent storages
==================================================

#### Start:

The Files storage can be used to create persistent collections. In order allow for later restart
- the endpoints for storages provider and transport provider must be fixed
- the segments must not be removed when the storage is destructed

```bash
./mcs_iov_backend_storage_provider                                         \
  "Just ($(cat ${MCS_IOV_PROVIDER_CONNECTABLE:?}))"                        \
  'ip::tcp ("0.0.0.0", 5678)' 1                                            \
  'ip::tcp ("0.0.0.0", 5679)' 2                                            \
  "Files (Prefix \"/tmp/FILES\", Limit $((50*2**30)))"                     \
  'Files::Segment::Create (Files::Segment::OnRemove::Keep())'              \
  'Files::Segment::Remove (Nothing)'                                       \
  'Files::Chunk::Description()'                                            \
  'Files::File::Read()'                                                    \
  'Files::File::Write()'
```

There is no limit on the number of persistent storages. In order to achieve persistency of (all stripes of) all data, all storages must be persistent.

The persistent storages can be used as above. At any time the current state of the provider can be queried and stored in a file:

```bash
MCS_IOV_PROVIDER_STATE='/tmp/MCS_IOV_PROVIDER_STATE'
./mcs_iov_backend_state                                                    \
  "$(cat ${MCS_IOV_PROVIDER_CONNECTABLE:?})"                               \
  > "${MCS_IOV_PROVIDER_STATE:?}"
```

Now the provider and all storages can be shut down. The save state is used to do a

#### Re-Start:

When starting up the startup order is: `provider, [storage_provider (Just provider)...]` When restarting, the startup order is revered to `[storage_provider Nothing...], provider STATE`.

The storage providers are not getting the connectable of the provider and that tells them to restart without attempting to register themselves.

```bash
./mcs_iov_backend_storage_provider                                         \
  "Nothing"                                                                \
  'ip::tcp ("0.0.0.0", 5678)' 1                                            \
  'ip::tcp ("0.0.0.0", 5679)' 2                                            \
  "Files (Prefix \"/tmp/FILES\", Limit $((50*2**30)))"                     \
  'Files::Segment::Create (Files::Segment::OnRemove::Keep())'              \
  'Files::Segment::Remove (Nothing)'                                       \
  'Files::Chunk::Description()'                                            \
  'Files::File::Read()'                                                    \
  'Files::File::Write()'
```

The restart must use the very same endpoints as the initial start did. The other parameters can be chosen differently, e.g. the storage can be turned into a non-persistent storage.

After all storages that are mentioned in the saved `State` have been restarted, the provider is restarted with the saved state as additional parameter:

```bash
stdbuf -oL                                                                 \
  ./mcs_iov_backend_provider                                               \
    "${MCS_IOV_PROVIDER_ENDPOINT:?}" 1                                     \
    "${MCS_IOV_PROVIDER_STATE:?}"                                        | \
      tee "${MCS_IOV_PROVIDER_CONNECTABLE:?}"
```

The provider will read the state and check that
- all mentioned storages are reachable
- all mentioned collections do exist

In case that not all storages are reachable or that not all collections exists, the provider startup will fail.

Otherwise the system is ready to be used as if it has been started (rather than re-started).

#### Read and write data:

For an instance of the iov_backend first create a configuration file by

```bash
MCS_IOV_BACKEND_CONFIGURATION_FILE='/tmp/MCS_IOV_BACKEND_CONFIGURATION_FILE'
./mcs_iov_backend_make_parameter                                           \
  "$(cat ${MCS_IOV_PROVIDER_CONNECTABLE:?})"                             > \
  "${MCS_IOV_BACKEND_CONFIGURATION_FILE:?}"
```
and an empty database by

```bash
MCS_IOV_BACKEND_DATABASE_FILE='/tmp/MCS_IOV_BACKEND_DATABASE'
./mcs_iov_backend_iov_make_empty_database                                  \
  "${MCS_IOV_BACKEND_DATABASE_FILE:?}"
```

Please note the options provided by `mcs_iov_backend_make_parameter` and adjust to your needs.

Some tools update the database, its content can be displayed using

```bash
./mcs_iov_backend_iov_print_database                                       \
  "${MCS_IOV_BACKEND_DATABASE_FILE:?}"
```

TODO: Explain `iov_collection_create` and `iov_collection_delete`: Those are the iov-way to create and delete collections and update the database.

Now, using the configuration file and the database, data can be written and read using `mcs_iov_backend_iov_write` and `mcs_iov_backend_iov_cat`.

EXAMPLE: Write the first `10 GiB` bytes from a `FILE` into the range `[5GiB..15GiB)` of the collection with the id `101`, using `2` concurrent writer threads that share `8` buffers of size `128 MiB` each:

```bash
cat FILE | head -c $((10*2**30))                                         | \
  ./mcs_iov_backend_iov_write                                              \
    "${MCS_IOV_BACKEND_CONFIGURATION_FILE:?}"                              \
    "${MCS_IOV_BACKEND_DATABASE_FILE:?}"                                   \
    101                                                                    \
    "[$((5*2**30))..$((15*2**30)))"                                        \
    2                                                                      \
    $((128*2**20))                                                         \
    8
```

EXAMPLE: Read and print to stdout the first `8GiB` many bytes from the collection with the id `19`, using `2` concurrent reader threads that share `4` buffers of size `32 MiB` each:

```bash
./mcs_iov_backend_iov_cat                                                  \
    "${MCS_IOV_BACKEND_CONFIGURATION_FILE:?}"                              \
    "${MCS_IOV_BACKEND_DATABASE_FILE:?}"                                   \
    19                                                                     \
    "[0..$((8*2**30)))"                                                    \
    2                                                                      \
    $((32*2**20))                                                          \
    4
```

#### Import data from a file:

In principle a file `DATA` can be imported using `mcs_iov_backend_iov_write`, e.g.

```bash
./mcs_iov_backend_iov_write                                                \
    "${MCS_IOV_BACKEND_CONFIGURATION_FILE:?}"                              \
    "${MCS_IOV_BACKEND_DATABASE_FILE:?}"                                   \
    23                                                                     \
    "[0..$((8*2**30)))"                                                    \
    4                                                                      \
    $((128*2**20))                                                         \
    6                                                                      \
    < DATA
```

will import the initial `8 GiB` of the file `DATA` into the collection with the id `23` using `4` writer threads that share `6` buffers of size `128 MiB` each.

While that works fine and the number of parallel writers can be set by users, the number of readers is fixed to just one as the input is provided sequentially via the standard input.

Knowing the data is stored in a file, the `mcs_iov_backend_iov_import` command can be used to speed up the data import:

```bash
./mcs_iov_backend_iov_import                                               \
    "${MCS_IOV_BACKEND_CONFIGURATION_FILE:?}"                              \
    "${MCS_IOV_BACKEND_DATABASE_FILE:?}"                                   \
    23                                                                     \
    "[0..$((8*2**30)))"                                                    \
    DATA                                                                   \
    "[0..$((8*2**30)))"                                                    \
    4                                                                      \
    4                                                                      \
    $((128*2**20))                                                         \
    6
```

This command will (just as above) import the range `[0..8 GiB)` from the file `DATA` into the range `[0..8 GiB)` of the collection with the id `23`. Again, there are `4` writer threads and `6` shared buffers of size `128 MiB`. In contrast to `mcs_iov_backend_iov_cat` the number of reader threads can now be set, too, and the example uses `4` reader threads.

The command `mcs_iov_backend_iov_import` reads _and_ writes in parallel. Also, it tries to process different parts of the data in order to use multiple storages at the same time.

#### Export data to a file:

Similar to importing, the tool `mcs_iov_backend_iov_export` exploits the knowledge that the data is written into a file and allows for multiple parallel write operations. The call

```bash
./mcs_iov_backend_iov_export                                               \
    "${MCS_IOV_BACKEND_CONFIGURATION_FILE:?}"                              \
    "${MCS_IOV_BACKEND_DATABASE_FILE:?}"                                   \
    23                                                                     \
    "[0..$((8*2**30)))"                                                    \
    DATA                                                                   \
    "[0..$((8*2**30)))"                                                    \
    4                                                                      \
    4                                                                      \
    $((128*2**20))                                                         \
    6
```

is the "inverse" of the above described import operation.

#### Offloading data import and export

The tools `mcs_iov_backend_iov_import` and `mcs_iov_backend_iov_export` both exploit the fact data is stored in a file. However, both tools are limited by the network bandwidth of the single node that executes the command. If the data file is located on a shared filesystem and the storage servers can access the data file, then there is an even more performant solution: The import and export can be offloaded to the storage providers. That eliminates the bottleneck of the single network interface. Instead, all storage providers will, in parallel, load or store their part of the (shared) data file.

The tools `mcs_iov_backend_import_shared` and `mcs_iov_backend_export_shared` implement the offload to the storage provider: The call

```bash
./mcs_iov_backend_import_shared                                            \
  "$(cat ${MCS_IOV_PROVIDER_CONNECTABLE:?})"                               \
  101                                                                      \
  "[0..$((20*2**30)))"                                                     \
  SHARED_DATA                                                              \
  "[0..$((20*2**30)))"
```

will load the first `20 GiB` from the shared file `SHARED_DATA` into the collection `101` and

```bash
./mcs_iov_backend_export_shared                                            \
  "$(cat ${MCS_IOV_PROVIDER_CONNECTABLE:?})"                               \
  101                                                                      \
  "[0..$((20*2**30)))"                                                     \
  SHARED_DATA                                                              \
  "[0..$((20*2**30)))"
```

is the corresponding export operation.

Note: Using `mcs_iov_backend_iov_import` and `mcs_iov_backend_iov_export` is a great way to implement the stage-in/out of a campaign storage.

Note: The tools `mcs_iov_backend_iov_import` and `mcs_iov_backend_iov_export` are not only offloading their operations to the storage provider but their implementation also uses the syscall `copy_file_range` to _further_ offload the copy operation to the underlying file system and might be executed in a time that does _not_ depend on the size of the imported or exported data.

todo: add words about the fixed workspace name
todo: add words about the general architecture
todo: add list_collection/workspace to the iov interface as well
