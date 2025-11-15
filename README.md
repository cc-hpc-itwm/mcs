Memory Centric Storage:
=======================

The Memory Centric Storage (MCS) is the European open source toolbox to produce scaling and resistant domain specific memory management. Applications using the MCS access all their resources uniformly with the same interface. Applications benefit from zero copy data transport and task offloading that brings the computation to the data.

Keywords:
=========

- Flexible toolbox for domain specific memory management.
- Uniform access to multiple different distributed memories and storages.
- Zero copy data transport.
- Task offloading to bring computation to the data.

Installation:
=============

Requirements:
-------------

- [git](https://git-scm.com/)
- [cmake](https://cmake.org/), version 3.24 or higher
- a c++20 compiler
  * tested: [gcc](https://gcc.gnu.org/) version 11, 12, 13, 14, 15
  * tested: [clang](https://clang.llvm.org/) version 19, 20, 21

How to install:
---------------

```
> git clone https://github.com/cc-hpc-itwm/mcs
> cmake -S mcs -B ${MCS_BUILD_DIR:?} --install-prefix ${MCS_INSTALL_PREFIX:?}
> cmake --build ${MCS_BUILD_DIR:?}
> cmake --install ${MCS_BUILD_DIR:?}
```

Please note that the configure step will retrieve the source dependencies:

- [asio](https://github.com/chriskohlhoff/asio.git)
- [fmt](https://github.com/fmtlib/fmt.git)

Options:
--------

## The C++ standard version:

- `MCS_CXX_STANDARD`: 20, 23.
- Default: `-DMCS_CXX_STANDARD=20`

## Enable testing:

- `MCS_TEST`: ON, OFF
- Default: `-DMCS_TEST=OFF`
- Additional source dependency:
  * [gtest](https://github.com/google/googletest.git)

Components details:
===================

Please see the individual README files for details about the components.

- [core/README.md](core/README.md)
- [share_service/README.md](share_service/README.md)
- [block_device/README.md](block_device/README.md)

Background: Summary:
====================

The traditional separation between heap memory and (distributed) file storage forces applications to either manage two independent layers, using different interfaces and explicitly transporting data between them or to ask the Linux kernel to map the heap memory into the storage. None of the solutions benefit from application knowledge about the data access as they rely on archaic `malloc`/`free` and `open`/`read`/`write`/`close` interfaces. There is simply no way to tell the data management that chunks of data are never modified, or that chunks are only ever used by a single resource, or that chunks are read only one time, or... As the result the data management performance is often far from optimal and applications spend their time waiting for data rather then processing.

Architecture:
=============

The memory centric storage MCS is a generalization of both, memory and storage and deals with chunks of data in a network of storages. It consists of multiple layers:

Core layer
----------

The core provides functionality to manage (segments of) storages and to copy chunks from one location to another. The core layer has no meaning to the data and does not attempt to protect data in any way. The core layer provides direct access to chunks and scalable functionality to manage storages and to transport data between them. The core layer can be extended by custom storages and custom transport networks.

Presentation layer
------------------

The presentations layer adds common functionality to the core layer. It is the responsibility of the presentation layer to prevent from races, to prevent from "use-after-free" errors and to add meaning (e.g. "file", "object") and user semantics (e.g. "atomic", "persistent", "mutable"). The presentation layer is modular and can be extended to the semantics of specific domains.

Application layer
-----------------

The application layer provides application specific terms and semantics. Applications use components from core and/or presentation layer and allows for efficient, safe and natural data management.

Background: Details:
====================

Heap memory:
------------

Heap memory is the only memory that can be accessed directly by processing units like CPU, GPU or FPGA. In heap memory the allocation routines have signatures along the lines

```
malloc (Size) -> BasePointer
free (BasePointer) -> void
memcopy (Destination <- Source, Size) -> Destination
```

Here `Destination` and `Source` are addresses. For heap memory an address is given by a tuple of `(BasePointer, Offset)` and its value for direct access is computed by `base_pointer + offset`.

The standard interface to heap memory assumes a single memory space. If there are multiple kinds of memory, then the standard interface hits its limits: It offers no way to take heap partitioning into account. This is an ongoing topic and there are libraries like `github.com/memkind` to deal with heap partitioning into classes like High Bandwidth Memory, local NUMA memory, DAX persistent memory, PMEM memory, and so on.

Heap memory offers

- _no_ methods for access control,
- _no_ methods to prevent from data races,
- _no_ methods to tell data semantics
- _no_ methods to tell data access patterns.

File I/O:
---------

File I/O deals with (persistent) data that is larger than the heap memory and offers interfaces to work on ranges of the (persistent) data. Ranges, or "chunks", are copied from the I/O storage into the heap memory, processed and copied back from heap memory into the I/O storage. The relevant interfaces look like

```
open (name, mode) -> File
read (File, Heap::Address <- Range) -> Size
write (File, Heap::Address -> Range) -> Size
close (File) -> void
```

Here a `File` is a handle to the I/O data with a certain `name`. The heap addresses must point to enough (allocated) heap memory to hold the bytes of the chunk.

"File systems" manage hierarchical "name spaces" of files and they offer methods for access control (per user) and some basic mechanisms to prevent from races.

In high performance applications the performance often is limited by the bandwidth and/or the latency between the heap memory and the I/O storage. In many situations the performance could be improved if the file system would know more about the concrete access pattern of the application.

I/O storage offers

- _no_ methods to tell data semantics
- _no_ methods to tell data access patterns.

I/O storage requires

- _explicit_ copies from and to heap memory.

Mapped memory:
--------------

The kernel offers a way to bridge between file I/O and heap memory via the `mmap` call: The (simplified) signature of `mmap` is

```
mmap (File, Range) -> Heap::Address
```

and it maps a (range of a) file into heap memory. That allows applications to process the I/O storage data using methods for heap memory.

The kernel takes the responsibility to copy data between I/O storage and heap memory behind the scenes. The kernel can fulfill this task because heap addresses are virtual addresses. That means that the kernel maintains a mapping between the physical memory and heap addresses and knows when a virtual address is part of mapped I/O storage data.

The kernel's knowledge about the memory access pattern of the application is limited and the performance of memory mapped files is often not optimal, namely when the kernel executes too many copies between persistent and dynamic memory.

Distributed memory and distributed storage:
-------------------------------------------

Neither heap memory nor file I/O deal, by default, with distributed memories.

There are DSM (distributed shared memory) systems to extend dynamic memory to a network of machines, e.g. by installing a custom kernel page fault handler and moving parts of the management of the virtual addresses into user land. The DSM system executes data transfers between machines if necessary. While such systems allow to scale existing applications, their performance suffers from the same limitations like mapped memory: Using a simple interface does not give the underlying system information about the access patterns and as a result there will be too many data transfers and/or too restrictive synchronization.

File I/O has long been provided for large distributed machines and distributed file systems are a standard part of the software stack for high performance machines. A distributed file system stores data in storage systems that are separated from the dynamic memory of the compute nodes by an interconnect. Just like standard file systems they manage the meta data and provide them to clients that run on the compute nodes. The interface for I/O and synchronization is unchanged from local file I/O and the distributed file system executes the data transfers between the remote storage and the local head memory behind the scenes. Saying that implies that distributed file systems have no information about the data access patterns and suffer from the same (or worse) performance implications like local file I/O or kernel page interception.

There are approaches in between distributed shared memory using a heap interface and dis-aggregated persistent storage using a file I/O interface, namely dis-aggregated shared memory (persistent or dynamic) using a heap interface. E.g. the CXL standard can be used to build such system. Again, the lack of explicit information about access patterns makes it difficult to achieve best performance/efficiency.
