# Copyright (C) 2024-2025 Fraunhofer ITWM
# License: https://raw.githubusercontent.com/cc-hpc-itwm/mcs/main/LICENSE

string (CONCAT
  MCS_TESTING_RPC_IP_VERSION_DOCSTRING
  "One of {'4', '6'}. If the version is not supported an exception with the "
  "message 'connect: Cannot assign requested address' may be thrown."
)
set (MCS_TESTING_RPC_IP_VERSION
  "4"
  CACHE
  STRING
  "${MCS_TESTING_RPC_IP_VERSION_DOCSTRING}"
)
set_property (CACHE MCS_TESTING_RPC_IP_VERSION PROPERTY STRINGS "4" "6")
