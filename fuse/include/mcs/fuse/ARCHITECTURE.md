The MCS filesystem in user space
================================

The libfuse (https://github.com/libfuse/libfuse) low level interface is used to present storage as a filesystem. The storage itself is not fixed and needs to implement the concept mcs/fuse/Content.

The kernel likes to talk about "inodes" which can be directories, files, symbolic links and even more kind of special files and devices. That abstraction is made visible to user land with the help of the libfuse and managed in the

## mcs/fuse/State

Holds and owns the inodes, separated into containers for directories, for files and for symbolic links. The libfuse calls into the state and the state creates, removes, looks up, and so on the inodes. All inodes have a common part (mcs/fuse/state/inode/Common) that includes attributes and reference counters.

The libfuse may use multiple threads and all access control happens in the mcs/fuse/State: All methods acquire either read or write access and use the acquired token to proof they have the correct access when calling into functions provided by the inodes.

NOTE: The access control is course grained: While there may be multiple concurrent readers, every potentially modifying access acquires exclusive access to the complete data structure. The advantage is that there is only a single lock in the complete system and that the reasoning is simplified. However, a more fine grained locking strategy may show better performance, for example if there are writes into one of the directories, then it would be enough to get exclusive access to that directory.

The specific inodes provide specific functionality:

### mcs/fuse/state/inode/Directory

Directories have "entries" (mcs/fuse/state/inode/directory/Entry) which are named inodes, the name being an arbitrary string and the inode being of any kind. Like the mcs/fuse/State, the directory organized them into separate containers, one for directories, one for files and one for symbolic links.

Directories can be traversed via opendir and readdir and the directory inode produces directory handles which are a consistent snapshot of the current content of the directory.

### mcs/fuse/state/inode/SymLink

Symbolic links are second names to (target) directory entries. They are implemented as an inode that remembers the name of their target.

### mcs/fuse/state/inode/File

Files are inodes that contain data. They have a size and allow to write and to read data. Also interfaces to flock, fallocate and ioctl are provided by files.

Files store the data using an implementation of the concept

## mcs/fuse/Content

File content has access to a state that is shared across the content of all the files in the system. Besides some basic interaction with the libfuse (the determine size and free space), the content provides size and method to read and write data. The content can safely access the shared content state, the access control and avoidance of conflicting access is managed by mcs/fuse/State and proven by access tokens.

Memory management happens in the content and there are example implementations based on std::vector, based on realloc and based on a sole mcs::core::storage::implementation. The MCS fuse layer turns into a distributed file system if the content implementation uses storages from multiple distributed machines.
