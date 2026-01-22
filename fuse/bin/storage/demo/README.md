Producer-Consumer Demo: Zero-Copy File Access via FUSE + SHMEM
==============================================================

This demo shows how MCS enables zero-copy data access even when the
producer is a legacy application that cannot be modified.

Architecture
------------

```
  ┌─────────────────┐          ┌─────────────────────────────────┐
  │  file_producer  │          │         mcs_consumer            │
  │  (legacy app)   │          │       (MCS-aware app)           │
  └────────┬────────┘          └───────────────┬─────────────────┘
           │                                   │
           │ write()                           │ 1. ioctl() → get parts
           │                                   │ 2. shm_open() + mmap()
           ▼                                   ▼
  ┌─────────────────────────────────────────────────────────────┐
  │                    FUSE Filesystem                          │
  │            (mcs_fuse_bin_storage_fs)                        │
  └────────────────────────────┬────────────────────────────────┘
                               │
                               ▼
  ┌─────────────────────────────────────────────────────────────┐
  │                   SHMEM Storage                             │
  │         ┌──────────┐  ┌──────────┐  ┌──────────┐            │
  │         │ Segment  │  │ Segment  │  │ Segment  │  ...       │
  │         │  sg_0    │  │  sg_1    │  │  sg_2    │            │
  │         └──────────┘  └──────────┘  └──────────┘            │
  └─────────────────────────────────────────────────────────────┘
```

**Producer flow (standard file I/O):**
1. Opens file on FUSE mountpoint
2. Writes data using standard write()
3. Data goes through FUSE → stored in SHMEM segments

**Consumer flow (zero-copy):**
1. Queries file distribution via ioctl (segment IDs + ranges)
2. Opens SHMEM segments directly with shm_open()
3. Maps segments with mmap() → **zero-copy** access to data

Building
--------

```bash
cmake -DMCS_FUSE=ON ...
make mcs_fuse_bin_storage_demo_producer mcs_fuse_bin_storage_demo_consumer
```

Running the Demo
----------------

### Step 1: Start the FUSE filesystem with SHMEM backend

```bash
export MCS_FUSE_BIN_STORAGE_DEMO_PREFIX="demo_producer_consumer"
export MCS_FUSE_BIN_STORAGE_DEMO_MOUNTPOINT=$(mktemp -d)

${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_fs                           \
    "SHMEM (Prefix \"${MCS_FUSE_BIN_STORAGE_DEMO_PREFIX:?}\", Unlimited)"  \
    'SHMEM::Size::Max()'                                                   \
    'SHMEM::Size::Used()'                                                  \
    'SHMEM::Segment::Create (SHMEM::Segment::AccessMode (384), Nothing)'   \
    'SHMEM::Segment::Remove()'                                             \
    'SHMEM::Chunk::Description()'                                          \
    'mcs_demo_fs'                                                          \
    -o auto_unmount                                                        \
    -f                                                                     \
    ${MCS_FUSE_BIN_STORAGE_DEMO_MOUNTPOINT:?} &
```

### Step 2: Run the producer (writes data to a file)

```bash
# Write 10 MiB of data
${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_demo_producer                \
    ${MCS_FUSE_BIN_STORAGE_DEMO_MOUNTPOINT:?}/data                         \
    $((10*2**20))
```

### Step 3: Check the file distribution

```bash
${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_fs_distribution              \
    ${MCS_FUSE_BIN_STORAGE_DEMO_MOUNTPOINT:?}/data
```

Output shows how the file is distributed across SHMEM segments:
```
(sg_0, [of_0..of_10485760))
```

### Step 4: Run the consumer (reads via zero-copy SHMEM access)

```bash
${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_demo_consumer                \
    "${MCS_FUSE_BIN_STORAGE_DEMO_PREFIX:?}"                                \
    ${MCS_FUSE_BIN_STORAGE_DEMO_MOUNTPOINT:?}/data
```

Expected output:
```
Reading "/tmp/xxx/data" via SHMEM (prefix: "demo_producer_consumer")
  Found part: (sg_0, [of_0..of_10485760))
SUCCESS: Verified 10485760 bytes of "/tmp/xxx/data".
```

### Cleanup

```bash
fusermount -u ${MCS_FUSE_BIN_STORAGE_DEMO_MOUNTPOINT:?}
rmdir ${MCS_FUSE_BIN_STORAGE_DEMO_MOUNTPOINT:?}
```

Key Points
----------

1. **Producer is unmodified**: Uses standard POSIX file I/O (fopen,
   fwrite, fclose). It does not know about MCS data access methods.

2. **Zero-copy access**: The consumer directly maps the SHMEM segments
   into its address space. No data is copied through the kernel.

3. **Transparent to producer**: The producer writes to what appears to
   be a normal directory. The FUSE filesystem handles all storage.

4. **MCS distribution query**: The ioctl interface allows consumers to
   discover where file content is stored without reading it.

Use Cases
---------

- **Legacy application integration**: Enable zero-copy data sharing
  with applications that cannot be modified.

- **High-throughput pipelines**: Producer writes large datasets,
  consumer processes them without copy overhead.

- **Memory efficiency**: Multiple consumers can map the same SHMEM
  segments simultaneously (read-only).

Files
-----

- `producer.cpp`: Legacy producer using standard file I/O
- `consumer.cpp`: MCS-aware consumer with zero-copy SHMEM access
- `CMakeLists.txt`: Build configuration
