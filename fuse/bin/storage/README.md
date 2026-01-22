The MCS filesystem in userspace: StorageFS
==========================================

Present a local MCS storage as a file system. Supported are the storage implementations `Heap`, `Files` and `SHMEM`. The file system supports directories, files, hard links, symbolic links and almost all standard file system operations. The meta data is kept in the (non persistent!) working memory of the user space program and file content is stored in the MCS storage.

Developer interested in the architecture please see [ARCHITECTURE.md](../include/mcs/fuse/ARCHITECTURE.md).

1. Start a filesystem:
----------------------

The storage is owned and created by the filesystem.

EXAMPLE: Start a file system on top of a `Heap` storage that can store up to `8 GiB` of data. The file system will be mounted to the temporary path `${MCS_FUSE_BIN_STORAGE_MOUNTPOINT}`. The file system implementation will run in foreground (`-f`) and automatically unmounted on exit. System utilities will show assign the name `mcs_storage_fs_HEAP` to it.

```
export MCS_FUSE_BIN_STORAGE_MOUNTPOINT=$(mktemp -d)
${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_fs                      \
    "Heap (Limit $((8*2**30)))"                                       \
    'Heap::Size::Max()'                                               \
    'Heap::Size::Used()'                                              \
    'Heap::Segment::Create (Nothing)'                                 \
    'Heap::Segment::Remove()'                                         \
    'Heap::Chunk::Description()'                                      \
    'mcs_storage_fs_HEAP'                                             \
    -o auto_unmount                                                   \
    -f                                                                \
    ${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}
```

EXAMPLE: Start a file system on top of a `Files` storage that can store up to `80 GiB` of data in `/tmp/FILES`. The file system will be mounted to the temporary path `${MCS_FUSE_BIN_STORAGE_MOUNTPOINT}`. The file system implementation will run in foreground (`-f`) and automatically unmounted on exit. System utilities will show assign the name `mcs_storage_fs_HEAP` to it.

```
export MCS_FUSE_BIN_STORAGE_MOUNTPOINT=$(mktemp -d)
${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_fs                      \
    "Files (Prefix \"/tmp/FILES\", Limit $((80*2**30)))"              \
    'File::Size::Max()'                                               \
    'Files::Size::Used()'                                             \
    'Files::Segment::Create (Files::Segment::OnRemove::Remove())'     \
    'Files::Segment::Remove (Nothing)'                                \
    'Files::Chunk::Description()'                                     \
    'mcs_storage_fs_FILES'                                            \
    -o auto_unmount                                                   \
    -f                                                                \
    ${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}
```

2. Use as file system:
----------------------

- The file system just created has all space available:
```
> df -h ${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}
Filesystem           Size  Used Avail Use% Mounted on
mcs_storage_fs_HEAP  8.0G     0  8.0G   0% ...
```

- Reserve space for a file `DATA`:
```
truncate --size $((8*2**30)) ${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}/DATA
```

- Write `8 Ki` many blocks of size `1 MiB` into `DATA`:
```
> dd if=/dev/zero                                                     \
     of=${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}/DATA                     \
     bs=$((2**20))                                                    \
     count=$((8*2**10))                                               \
     conv=notrunc
8192+0 records in
8192+0 records out
8589934592 bytes (8.6 GB, 8.0 GiB) copied, 3.00304 s, 2.9 GB/s
```

- Check the space, nothing is left:
```
> df -h ${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}
Filesystem           Size  Used Avail Use% Mounted on
mcs_storage_fs_HEAP  8.0G  8.0G     0 100% ...
```

- Remove `DATA`:
```
unlink ${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}/DATA
```

2. Query the MCS distribution of a file:
----------------------------------------

The content of files is stored in segments of the underlying MCS storage. In order to access it using the MCS interface the distribution of a file in the MCS storage can be queried.

Assume the empty file system from above and reserve `1 GiB` for a file `DATA`

```
truncate --size $((1*2**30)) ${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}/DATA
```

Now query the distribution in the MCS storage:

```
> ${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_fs_distribution \
    ${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}/DATA
(sg_0, [of_0..of_1073741824))
```

The output says that the `1 GiB` is stored in the segment with the id `sg_0`. Now truncate the file to a size of `2 GiB` and check the distribution again:

```
> truncate --size $((2*2**30)) ${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}/DATA
> ${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_fs_distribution       \
    ${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}/DATA
(sg_0, [of_0..of_1073741824))
(sg_1, [of_1073741824..of_2147483648))
```

A second segment has been created, also `1 GiB` in size. Now write `2 GiB` of data and check again:

```
> dd if=/dev/zero                                                     \
     of=${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}/DATA                     \
     bs=$((2**20))                                                    \
     count=$((2*2**10))                                               \
     conv=notrunc
2048+0 records in
2048+0 records out
2147483648 bytes (2.1 GB, 2.0 GiB) copied, 0.952338 s, 2.3 GB/s
> ${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_fs_distribution       \
    ${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}/DATA
(sg_0, [of_0..of_1073741824))
(sg_1, [of_1073741824..of_2147483648))
```

The distribution has not changed. However, if `DATA` is overwritten (note the missing `conv=notrunc`, then the distribution changes:

```
> dd if=/dev/zero                                                     \
     of=${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}/DATA                     \
     bs=$((2**20))                                                    \
     count=$((2*2**10)
2048+0 records in
2048+0 records out
2147483648 bytes (2.1 GB, 2.0 GiB) copied, 1.20973 s, 1.8 GB/s
> ${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_fs_distribution       \
    ${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}/DATA
(sg_2, [of_0..of_1048576))
(sg_3, [of_1048576..of_2097152))
...
(sg_2048, [of_2145386496..of_2146435072))
(sg_2049, [of_2146435072..of_2147483648))
> ${MCS_INSTALL_DIR:?}/bin/mcs_fuse_bin_storage_fs_distribution       \
    ${MCS_FUSE_BIN_STORAGE_MOUNTPOINT:?}/DATA                       | \
    wc -l
2048
```

Each write has created a separate segment.
