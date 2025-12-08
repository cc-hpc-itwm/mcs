Supports libfabric in a generic way in the sense that libfabric providers (`tcp`, `verbs`, ...) can be choosen at execution time. The libfabric control messages are exchanged via the ASIO-based RPC.

For example:

```
./iml_core_bin_demo_transport_implementation_libfabric_provider      \
  'ip::tcp ("10.136.144.61", 0)'                                     \
  'verbs;ofi_rxm'                                                    \
  'ibp94s0'                                                          \
  $PROVIDER_PATH                                                     \
```

starts a provider using `verbs;ofi_rxm` as a libfabric provider over the interface `ibp94s0` for data transport and `ip::tcp ("10.136.144.61", 0)` for control messages.

The corresponding client call is

```
./iml_core_bin_demo_transport_implementation_libfabric_client        \
  'verbs;ofi_rxm'                                                    \
  'ibp94s0'                                                          \
  $PROVIDER_PATH                                                     \
```

The transport implementation itself relies on the RDM (Reliable Datagram), an implementation of the reliable-unconnected communication model of libfabric. It uses active endpoints, hence eliminating the need to manage connections through explicit listen/connect/accept calls.

When a client requests data with a `Get` command, it also sends its connection information through the RPC control channel as a libfabric address. This address is then inserted to the address vector of the provider before the corresponding transfer is initiated.

When a client wants to provide data with a `Put` command, there is no need to exchange connection information, as the libfabric address of the provider is already given to the client during initialization.

The class `core::transport::implementation::libfabric::transporter::rdm::Interface` holds the logic and resources that are common to both the provider and the client, while `core::transport::implementation::libfabric::transporter::rdm::Provider` and `core::transport::implementation::libfabric::transporter::rdm::Client` implement the provider and the client accordingly. The provider and the client are both given the requested libfabric interface on initialization, while the client is also given the libfabric address of the provider as the destination address in addition. After the initialization, both the provider and the client can send and receive data. The client always sends to the provider, and the provider is given the requested address for the send operation.

Libfabric interface required for initialization consists of the pair, provider and domain. With a fully working installation of libfabric, the `fi_info` command lists the available providers and domains in the system. Otherwise the `fi_provider` documentation contains the names of the providers, and `ip link` command shows the available domains on a unix system.
