The MCS filesystem in userspace
===============================

`mcs_fuse_bin_fs` exposes one selected MCS FUSE content implementation as a
filesystem. It supports `Vector`, `Realloc`, and storage-backed content.
Storage-backed content independently selects one `Files`, `Heap`, `SHMEM`, or
`Virtual` core storage implementation.

The filesystem supports directories, files, hard links, symbolic links, and
almost all standard filesystem operations. Its metadata is kept in the
non-persistent working memory of the userspace process.

Developers interested in the architecture should see
[ARCHITECTURE.md](../include/mcs/fuse/ARCHITECTURE.md).

1. Start a filesystem:
----------------------

Select `Vector` for file content stored in one `std::vector<std::byte>` per
file:

```
export MCS_FUSE_BIN_MOUNTPOINT=$(mktemp -d)
${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_fs                                  \
  --content 'Vector'                                                      \
  --name 'mcs_fs_VECTOR'                                                  \
  --mountpoint ${MCS_FUSE_BIN_MOUNTPOINT:?}                               \
  '[[FUSE' -o auto_unmount -f 'FUSE]]'
```

Select `Realloc` for manually allocated process-memory content:

```
${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_fs                                  \
  --content 'Realloc'                                                     \
  --name 'mcs_fs_REALLOC'                                                 \
  --mountpoint ${MCS_FUSE_BIN_MOUNTPOINT:?}                               \
  '[[FUSE' -o auto_unmount -f 'FUSE]]'
```

Select `Storage` and a `Heap` storage with an eight-gibibyte limit:

```
${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_fs                                  \
  --content 'Storage'                                                     \
  --storage "Heap (Limit $((8*2**30)))"                                   \
  --storage-size-max 'Heap::Size::Max()'                                  \
  --storage-size-used 'Heap::Size::Used()'                                \
  --storage-segment-create 'Heap::Segment::Create (Nothing)'              \
  --storage-segment-remove 'Heap::Segment::Remove()'                      \
  --storage-chunk-description 'Heap::Chunk::Description()'                \
  --name 'mcs_fs_STORAGE_HEAP'                                            \
  --mountpoint ${MCS_FUSE_BIN_MOUNTPOINT:?}                               \
  '[[FUSE' -o auto_unmount -f 'FUSE]]'
```

Select `Storage` and a `Files` storage with an 80-gibibyte limit:

```
mkdir -p /tmp/FILES
${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_fs                                  \
  --content 'Storage'                                                     \
  --storage "Files (Prefix \"/tmp/FILES\", Limit $((80*2**30)))"          \
  --storage-size-max 'Files::Size::Max()'                                 \
  --storage-size-used 'Files::Size::Used()'                               \
  --storage-segment-create 'Files::Segment::Create (Files::Segment::OnRemove::Remove())' \
  --storage-segment-remove 'Files::Segment::Remove (Nothing)'             \
  --storage-chunk-description 'Files::Chunk::Description()'               \
  --name 'mcs_fs_STORAGE_FILES'                                           \
  --mountpoint ${MCS_FUSE_BIN_MOUNTPOINT:?}                               \
  '[[FUSE' -o auto_unmount -f 'FUSE]]'
```

Select `Storage` and a `SHMEM` storage with an eight-gibibyte limit:

```
${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_fs                                  \
  --content 'Storage'                                                     \
  --storage "SHMEM (Prefix \"mcs-fuse\", Limit $((8*2**30)))"             \
  --storage-size-max 'SHMEM::Size::Max()'                                 \
  --storage-size-used 'SHMEM::Size::Used()'                               \
  --storage-segment-create 'SHMEM::Segment::Create (SHMEM::Segment::AccessMode (384), Nothing)' \
  --storage-segment-remove 'SHMEM::Segment::Remove()'                     \
  --storage-chunk-description 'SHMEM::Chunk::Description()'               \
  --name 'mcs_fs_STORAGE_SHMEM'                                           \
  --mountpoint ${MCS_FUSE_BIN_MOUNTPOINT:?}                               \
  '[[FUSE' -o auto_unmount -f 'FUSE]]'
```

Select `Storage` and a dynamically loaded storage implementation. The shared
object must export the MCS storage C API. Each `[]` below is the byte-vector
parameter for the corresponding operation; replace it when required by the
loaded implementation:

```
${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_fs                                  \
  --content 'Storage'                                                     \
  --storage 'Virtual ("/path/to/libmcs_storage.so", [])'                  \
  --storage-size-max 'Import_C_API::Size::Max([])'                        \
  --storage-size-used 'Import_C_API::Size::Used([])'                      \
  --storage-segment-create 'Import_C_API::Segment::Create ([])'           \
  --storage-segment-remove 'Import_C_API::Segment::Remove ([])'           \
  --storage-chunk-description 'Import_C_API::Chunk::Description([])'      \
  --name 'mcs_fs_STORAGE_VIRTUAL'                                         \
  --mountpoint ${MCS_FUSE_BIN_MOUNTPOINT:?}                               \
  '[[FUSE' -o auto_unmount -f 'FUSE]]'
```

The `Storage` form requires six named core storage options:

```text
--storage EXPRESSION
--storage-size-max EXPRESSION
--storage-size-used EXPRESSION
--storage-segment-create EXPRESSION
--storage-segment-remove EXPRESSION
--storage-chunk-description EXPRESSION
```

They describe one storage instance for the complete lifetime of the mount.
Storage cannot be added to or removed from a running filesystem. `Vector` and
`Realloc` reject storage options and have no configurable capacity.

All MCS options accept `--name value` and `--name=value`. Native libfuse
arguments belong in repeatable `'[[FUSE' ... 'FUSE]]'` sections, not after
`--`. Sections are opaque and require a closing tag. See the shared
[command-line conventions](../../README.md#command-line-conventions).

Content selection
-----------------

The public `mcs::fuse::content::selection::select` operation in
`mcs/fuse/content/select.hpp` selects `Vector`, `Realloc`, or `Storage`.
For `Storage`, it delegates parameter parsing to the public core
`mcs::core::storage::selection::select` operation.

The generic executable and `mcs_tiered_storage_fs` each supply their own list
of supported core storage implementations. Both currently support `Files`,
`Heap`, `SHMEM`, and `Virtual`, but their storage policies can evolve
independently.

2. Use as file system:
----------------------

- The file system just created has all space available:
```
> df -h ${MCS_FUSE_BIN_MOUNTPOINT:?}
Filesystem           Size  Used Avail Use% Mounted on
mcs_storage_fs_HEAP  8.0G     0  8.0G   0% ...
```

- Reserve space for a file `DATA`:
```
truncate --size $((8*2**30)) ${MCS_FUSE_BIN_MOUNTPOINT:?}/DATA
```

- Write `8 Ki` many blocks of size `1 MiB` into `DATA`:
```
> dd if=/dev/zero                                                         \
     of=${MCS_FUSE_BIN_MOUNTPOINT:?}/DATA                                 \
     bs=$((2**20))                                                        \
     count=$((8*2**10))                                                   \
     conv=notrunc
8192+0 records in
8192+0 records out
8589934592 bytes (8.6 GB, 8.0 GiB) copied, 3.00304 s, 2.9 GB/s
```

- Check the space, nothing is left:
```
> df -h ${MCS_FUSE_BIN_MOUNTPOINT:?}
Filesystem           Size  Used Avail Use% Mounted on
mcs_storage_fs_HEAP  8.0G  8.0G     0 100% ...
```

- Remove `DATA`:
```
unlink ${MCS_FUSE_BIN_MOUNTPOINT:?}/DATA
```

3. Query the MCS distribution of `Storage` content:
----------------------------------------------------

`mcs_fuse_bin_storage_fs_distribution` is suited only for files using the
`Storage` content implementation. Their bytes are stored in segments of the
selected core storage, and the tool queries that segment mapping.

The tool does not work with `Vector` or `Realloc` content because those
implementations do not expose a core storage distribution. Querying such a
file fails with an unsupported-ioctl error.

Assume the empty file system from above and reserve `1 GiB` for a file `DATA`

```
truncate --size $((1*2**30)) ${MCS_FUSE_BIN_MOUNTPOINT:?}/DATA
```

Now query the distribution in the MCS storage:

```
> ${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_fs_distribution           \
    --file ${MCS_FUSE_BIN_MOUNTPOINT:?}/DATA
(sg_0, [of_0..of_1073741824))
```

The output says that the `1 GiB` is stored in the segment with the id `sg_0`. Now truncate the file to a size of `2 GiB` and check the distribution again:

```
> truncate --size $((2*2**30)) ${MCS_FUSE_BIN_MOUNTPOINT:?}/DATA
> ${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_fs_distribution           \
    --file ${MCS_FUSE_BIN_MOUNTPOINT:?}/DATA
(sg_0, [of_0..of_1073741824))
(sg_1, [of_1073741824..of_2147483648))
```

A second segment has been created, also `1 GiB` in size. Now write `2 GiB` of data and check again:

```
> dd if=/dev/zero                                                         \
     of=${MCS_FUSE_BIN_MOUNTPOINT:?}/DATA                                 \
     bs=$((2**20))                                                        \
     count=$((2*2**10))                                                   \
     conv=notrunc
2048+0 records in
2048+0 records out
2147483648 bytes (2.1 GB, 2.0 GiB) copied, 0.952338 s, 2.3 GB/s
> ${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_fs_distribution           \
    --file ${MCS_FUSE_BIN_MOUNTPOINT:?}/DATA
(sg_0, [of_0..of_1073741824))
(sg_1, [of_1073741824..of_2147483648))
```

The distribution has not changed. However, if `DATA` is overwritten (note the missing `conv=notrunc`, then the distribution changes:

```
> dd if=/dev/zero                                                         \
     of=${MCS_FUSE_BIN_MOUNTPOINT:?}/DATA                                 \
     bs=$((2**20))                                                        \
     count=$((2*2**10))
2048+0 records in
2048+0 records out
2147483648 bytes (2.1 GB, 2.0 GiB) copied, 1.20973 s, 1.8 GB/s
> ${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_fs_distribution           \
    --file ${MCS_FUSE_BIN_MOUNTPOINT:?}/DATA
(sg_2, [of_0..of_1048576))
(sg_3, [of_1048576..of_2097152))
...
(sg_2048, [of_2145386496..of_2146435072))
(sg_2049, [of_2146435072..of_2147483648))
> ${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_fs_distribution           \
    --file                                                                \
      ${MCS_FUSE_BIN_MOUNTPOINT:?}/DATA                                 | \
    wc -l
2048
```

Each write has created a separate segment.
