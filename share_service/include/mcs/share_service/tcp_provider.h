/* Copyright (C) 2023-2025 Fraunhofer ITWM */
/* License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE */

#ifndef MCS_SHARE_SERVICE_TCP_PROVIDER_H
#define MCS_SHARE_SERVICE_TCP_PROVIDER_H

// NOLINTBEGIN (modernize-use-trailing-return-type), it is C

#ifdef __cplusplus
extern "C"
{
#endif

  // A TCP provider for the MCS share service, usable in C programs.
  //
  // EXAMPLE usage: See mcs/share_service/bin/tcp_provider.c
  //
  struct mcs_share_service_tcp_provider;

  // Creates an tcp_provider that uses number_of_threads many threads
  // and sets up resources.
  //
  // tcp_version - any of {4,6}
  // port        - communication port, 0 for auto select
  // path        - the file system path that contains the provider
  //               information
  //
  // Expects: provider != NULL
  // Expects: *provider == NULL
  // Ensures: *provider != NULL
  //
  // Returns:
  //    0 success
  //   -1 invalid provider parameter (provider is NULL or *provider is not NULL)
  //   -2 failure
  //   -3 invalid tcp version
  //
  int mcs_share_service_create_tcp_provider
    ( int tcp_version
    , unsigned short int port
    , char const* path
    , int number_of_threads
    , struct mcs_share_service_tcp_provider** provider
    );

  // Destroys an tcp_provider and cleans up used resources.
  //
  // Expects: provider != NULL
  // Expects: *provider != NULL
  // Ensures: *provider == NULL
  //
  // Returns:
  //    0 success
  //   -1 invalid provider parameter (provider is NULL or *provider is NULL)
  //
  int mcs_share_service_destroy_tcp_provider
    ( struct mcs_share_service_tcp_provider** provider
    );

  // Adds a signal to the set of signals that stops the provider.
  //
  // Expects: provider != NULL
  //
  // Returns:
  //    0 success
  //   -1 invalid provider parameter (provider is NULL)
  //
  int mcs_share_service_stop_tcp_provider_on_signal
    ( struct mcs_share_service_tcp_provider* provider
    , int signal
    );

  // Blocking call: Runs the provider.
  //
  // Expects: provider != NULL
  //
  // Returns:
  //    0 success
  //   -1 invalid provider parameter (provider is NULL)
  //
  int mcs_share_service_run_tcp_provider
    ( struct mcs_share_service_tcp_provider* provider
    );

  // Stops a provider.
  //
  // Expects: provider != NULL
  //
  // Returns:
  //    0 success
  //   -1 invalid provider parameter (provider is NULL)
  //
  int mcs_share_service_stop_tcp_provider
    ( struct mcs_share_service_tcp_provider* provider
    );

#ifdef __cplusplus
}
#endif

// NOLINTEND (modernize-use-trailing-return-type)

#endif
