/* Copyright (C) 2023-2025 Fraunhofer ITWM */
/* License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE */

#include <mcs/share_service/tcp_provider.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Example compilation with gcc only:
//
// gcc -c tcp_provider.c -I $MCS_ROOT/include/mcs/share_service/include/
// LIBS+=$MCS_ROOT/lib/mcs/core/libmcs_core.a
// LIBS+=$MCS_ROOT/lib/mcs/share_service/libmcs_share_service.a
// LIBS+=$MCS_ROOT/lib/mcs/share_service/libmcs_share_service_tcp_provider.a
// LIBS+=$MCS_ROOT/lib/rpc/librpc.a
// LIBS+=$MCS_ROOT/lib/serialization/libserialization.a
// LIBS+=$MCS_ROOT/lib/util/libutil.a
// LIBS+=-lrt
// LIBS+=-lpthread
// LIBS+=-lstdc++
// gcc tcp_provider.o $LIBS -o tcp_provider.exe
//

#define MCS_SHARE_SERVICE_CHECKED(_exit, _f, ...)               \
  {                                                             \
    ret = _f (__VA_ARGS__);                                     \
                                                                \
    if (ret != 0)                                               \
    {                                                           \
      fprintf (stderr, "Error in '%s': ec = %i\n", #_f, ret);   \
                                                                \
      _exit;                                                    \
    }                                                           \
  }

int main (int argc, char** argv)
{
  int ret = EXIT_SUCCESS;

  if (argc != 5)
  {
    fprintf
      ( stderr
      , "usage: %s tcp_version port mcs_share_service_path number_of_threads\n"
        "\n"
        "EXAMPLE: %s 4 0 /tmp/share_service 4\n"
        "EXAMPLE: %s 6 9876 /tmp/share_service 1\n"
      , argv[0]
      , argv[0]
      , argv[0]
      );

    ret = -99;

    goto RETURN;
  }

  struct mcs_share_service_tcp_provider* provider = NULL;

  MCS_SHARE_SERVICE_CHECKED
    ( goto RETURN
    , mcs_share_service_create_tcp_provider
    , atoi (argv[1])
    , (unsigned short int) (atoi (argv[2]))
    , argv[3]
    , atoi (argv[4])
    , &provider
    );

  MCS_SHARE_SERVICE_CHECKED
    ( goto DESTROY
    , mcs_share_service_stop_tcp_provider_on_signal, provider, SIGINT
    );
  MCS_SHARE_SERVICE_CHECKED
    ( goto DESTROY
    , mcs_share_service_stop_tcp_provider_on_signal, provider, SIGTERM
    );
  MCS_SHARE_SERVICE_CHECKED
    ( goto DESTROY
    , mcs_share_service_run_tcp_provider, provider
    );

 DESTROY:
  MCS_SHARE_SERVICE_CHECKED
    ( goto RETURN
    , mcs_share_service_destroy_tcp_provider, &provider
    );

 RETURN:
  return ret;
}
