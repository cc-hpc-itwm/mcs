The core layer of the MCS deals with chunks of data in network of storages. The core layer is a generalization of heap memory and of file I/O. The core layer scales to many machines and can be extended with custom storages and custom networks. The core layer provides functionality to build a network of storages, the control local and remote storages and to transport data between local and remote storages.

## Chunk:

Chunks are the basic data management unit. The MCS make no assumption about the size of a chunk. A chunk provides direct access to a part of the data:

```
Chunk (chunk::Description)
data (Chunk) -> Span
```

Chunks are created from a description that is delivered by and specific to a storage implementation. Chunks offer a method that returns a `Span` which is an object that describes a range of heap memory.

```
Span::begin() -> Heap::Address
Span::size() -> Size
```

An application can use a chunk just like standard heap memory. The work to make a chunk available in heap memory is done by a storage implementation. Chunks are small and cheap views into data that is owned by a storage.

## Storage:

The storage is the (physical) media that contains the chunks. Examples include `Heap`, `SHMEM`, `Files`, or `Virtual`. The MCS defines a [generic interface for storages](include/mcs/core/storage/Concepts.hpp). The crucial functionality for a storage is

```
segment_create (Parameter::Segment::Create, Size) -> SegmentID
segment_remove (Parameter::Segment::Remove, SegmentID) -> Size

chunk_description
  ( Parameter::Chunk::Description
  , SegmentID
  , Range
  ) -> Chunk::Description
```

Here `Parameter::*` describe types that are specific to storage implementations and are used to submit parameters to the implementations. For example, a `Files` storage may create `Persistent` "segments" (which would be files).

A `Chunk::Description` is also specific to a storage implementation and can be converted into a `chunk::Description`, which, in turn, can be used to create a `Chunk` with direct access to the data.

The term "segment" describes a conceptually contiguous piece of data. Chunks are part of segments.

The separation between chunks and their descriptions is made because chunks are not serializeable (they have no proper meaning on a different process) but chunk descriptions are. Chunk descriptions typically do have proper meaning on different processes on the same machine. They may or may not have proper meaning on different machines.

### C API:

For compatibility there is a ABI stable C interface for storages defined in [c_api](include/mcs/core/storage/c_api/storage.h).

The class [Implement_C_API](include/mcs/core/storage/Implement_C_API.hpp) can be used to implement the C API on top of an existing (C++) storage implementation. That is useful in order to share a storage implementation in binary form without sharing the source code.

### Standard implementations:

MCS provides a couple of storage implementations:

- [Heap](include/mcs/core/storage/implementation/Heap.hpp): Stores data in private heap memory. Useful to cache common data, e.g. using machines in the network that have a lot of heap memory.

- [SHMEM](include/mcs/core/storage/implementation/SHMEM.hpp): Stores data in heap memory and publishes data as shared memory. Useful to access the very same heap memory by multiple applications on the same machine.

- [Files](include/mcs/core/storage/implementation/Files.hpp): Stores data in files. Every segment is stored in a separate file. Useful if data must be kept persistent over multiple calls. Files in a distributed file system can be used to share data between multiple machines.

- [Virtual](include/mcs/core/storage/implementation/Virtual.hpp): Imports a storage implementation from a shared object during execution time. The shared object must implement the C API. Useful if storage implementations are loaded/unloaded during execution time. Useful to support storage implementations provided by third parties and not shared with the application developers.

- [Import_C_API](include/mcs/core/storage/implementation/Import_C_API.hpp): Lifts a storage implementation that implements the C API to the C++ interface. Useful if custom storage implementations of the C interface are known at compilation time.

- [Trace](include/mcs/core/storage/implementation/Trace.hpp): Traces storage adds a tracer to some underlying storage. The tracer implements the [trace interface](include/mcs/core/storage/trace/Concepts.hpp) that defines an interface for all management events. Example tracers to collect trace events in memory or to write them into a file are defined in [tracer](include/mcs/core/storage/tracer).

## Storages:

A heterogeneous collection of storages. Assigns an identifier to individual storages and offers functionality to manage storages in a generic way. To manage a storage includes to create and remove segments, to retrieve sizes and chunk descriptions and to offload operations to the storage. Data transport is not part of the management interface and handled independently by the transport interface.

## Control:

The control network is used to manage storages, that includes their creation and destruction, the creation and removal of segments and the retrieval of chunk descriptions.

### Provider:

A control provider offers remote access to manage a collection of storages. They offer a `local_endpoint()` that can be used by clients to contact the storages provider.

### Client:

The control client offers the management functionality of some remote control Provider. See [control/Client](include/mcs/core/control/Client.hpp) for details.

## Transport:

The transport layer provides generic methods to transport data between (local or remote) storages.

### Address:

A transport _address_ consists of the storage identifier, the segment id and the offset. Also it contains the (serialized) parameter to create chunks in the storage with the given identifier.

### Provider:

A transport provider offers remote access to a collection of local storages. The way to construct a provider depends on the concrete transport network. Transport providers offer a `connection_information()` that can be used by transport clients to contact the transport provider.

### Client:

A transport client implements a [interface](include/mcs/core/transport/client/Concepts.hpp) to _get_ or _put_ data. The signatures of get and put follow the signature of a `memcpy`

```
memory_get (Destination <- Source, size) -> future<Size>
memory_put (Destination <- Source, size) -> future<Size>
```

In `get` the destination is local and the source is remote, in `put` the destination is remote and the source is local. Both methods are asynchronous.
