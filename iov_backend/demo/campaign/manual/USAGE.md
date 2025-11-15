# MCSS Demo

* [Overview](#overview)
* [Requirements](#requirements)
* [step by step guide](#step-by-step-guide)
  * [1. get nodes](#1-get-nodes)
  * [2. prepare directories and files](#2-prepare-directories-and-files)
    * [build and install the binaries](#build-and-install-the-binaries)
    * [adapt `.bashrc`](#adapt-bashrc)
    * [configure `meta.env`](#configure-metaenv)
  * [3. manually set up terminals](#3-manually-set-up-terminals)
    * [`PROVIDER`](#provider)
      * [interactive terminal](#interactive-terminal)
      * [`htop` terminal](#htop-terminal)
    * [`CLIENT`](#client)
      * [interactive terminal](#interactive-terminal-1)
      * [`htop` terminal](#htop-terminal-1)
    * [`STORAGE`s](#storages)
      * [interactive terminal](#interactive-terminal-2)
      * [`htop` terminal](#htop-terminal-2)
      * [files `watch` terminal](#files-watch-terminal)
  * [4. run the demo](#4-run-the-demo)
    * [Start Provider](#start-provider)
    * [Start Storages](#start-storages)
    * [Client](#client-1)

## Overview

The setup for this demo is more manual than the beehive demo. This is mainly due to the fact, that `pdsh` is not available on seislab.
It is recommended to use at least 3 Nodes. One node hosts the provider, as well as the client. The other (arbitrary many) nodes host the storages.

__Note:__ Configuration of this demo is done at two locations:

* `bashrc`: exports `MCS_DEMO_ROOT` and `source`s the required .ENV files.
* `meta.ENV` configures the shared connection information and file locations.

(details below)

## Requirements

* `ssh` access to all nodes.
* All nodes has to be able to reach each other via IP address.
* A shared directory that is accessible on all nodes.
* `htop` installed on nodes.

## step by step guide

### 1. get nodes

A temporary allocation on beehive:

```bash
salloc -N <number_of_nodes> -t <time_in_minutes> -p <partition_name> --exclusive --x11=all
```

Or, with reservation:

```bash
 salloc --reservation=<reservation_id> -N <number_of_nodes> --x11=all
 ```

 Show all current reservations:

 ```bash
 scontrol show reservations
 ```

### 2. prepare directories and files

Create a directory that is accessible to all demo nodes and clone the mcs and iov repositories into `${MCS_DEMO_ROOT}/[mcs|iov]/src`:

```bash
export MCS_DEMO_ROOT=/path/to/shared/directory
mkdir ${MCS_DEMO_ROOT}
git clone git@gitlab.hpc.devnet.itwm.fhg.de:mcse/mcs.git ${MCS_DEMO_ROOT:?}/mcs/src
git clone git@gitlab.hpc.devnet.itwm.fhg.de:mcse/libIOVerbs.git ${MCS_DEMO_ROOT:?}/iov/src
```

#### build and install the binaries

```bash
cmake -B ${MCS_DEMO_ROOT:?}/build                                          \
      -S ${MCS_DEMO_ROOT:?}/mcs/src                                        \
      -DCMAKE_BUILD_TYPE=Release                                           \
      -DCMAKE_INSTALL_PREFIX=${MCS_DEMO_ROOT:?}/install                    \
      -DMCS_CXX_STANDARD=20                                                \
      -DMCS_IOV_BACKEND=ON                                                 \
      -DMCS_LIBIOV_REPO=${MCS_DEMO_ROOT:?}/iov/src
cmake --build ${MCS_DEMO_ROOT:?}/build --target install -j $(nproc)
```

#### adapt `.bashrc`

In order to enable the environment configuration for each node, add the following lines to the profile file that is `source`d when a login shell is opened.

```bash
export MCS_DEMO_ROOT=/path/to/shared/directory
if [ -f ${MCS_DEMO_ROOT:?}/mcs/src/mcs/core/bin/demo/campaign/manual/meta.ENV ]; then
  . ${MCS_DEMO_ROOT:?}/mcs/src/mcs/core/bin/demo/campaign/manual/meta.ENV
  . ${MCS_DEMO_ROOT:?}/mcs/src/mcs/core/bin/demo/campaign/manual/decide_env.ENV
fi
```

#### configure `meta.env`

If necessary adapt the following:

* Common part of the IP Address of all participating nodes inside the function `MCS_DEMO_IP`. Note: omit leading zeros.
* update `MCS_DEMO_PROVIDER_NODE_ID`.

### 3. manually set up terminals

For each node Type (`PROVIDER`, `CLIENT` or `STORAGE`) open the specified terminals and group them together on you screen.

#### `PROVIDER`

##### interactive terminal

```bash
ssh <provider_node>
MCS_DEMO_NODE_TYPE=PROVIDER bash
```

##### `htop` terminal

```bash
ssh <provider_node>
htop
```

#### `CLIENT`

##### interactive terminal

```bash
ssh <client_node>
MCS_DEMO_NODE_TYPE=CLIENT bash
```

##### `htop` terminal

```bash
ssh <client_node>
htop
```

#### `STORAGE`s

For each of the storage nodes:

##### interactive terminal

```bash
ssh <storage_node>
MCS_DEMO_NODE_TYPE=STORAGE bash
```

##### `htop` terminal

```bash
ssh <storage_node>
htop
```

##### files `watch` terminal

```bash
ssh <storage_node>
bash
watch -n1 ls -hs ${MCS_DEMO_FILE_STORAGE_ROOT:?}'
```

### 4. run the demo

#### Start Provider

Go to Provider CLI terminal and run `mcs_demo_start_backend_provider`.

This is blocking. The output should be the connection information. For example: `ip::tcp (Address "192.168.153.30", 9876)`.

Verification: The variable `MCS_DEMO_IOV_PROVIDER_CONNECTABLE` hold a filename. that file should have the same content as the output from `mcs_demo_start_backend_provider`. This file should be accessible from any other participating node. For example from the client node this can look like the following:

```bash
[MCS_CLIENT] ~$ cat ${MCS_DEMO_IOV_PROVIDER_CONNECTABLE:?}
ip::tcp (Address "192.168.153.30", 9876)
[MCS_CLIENT] ~$ echo ${MCS_DEMO_IOV_PROVIDER_CONNECTABLE:?}
/home/zimmermannp/mcs_demo/MCS_DEMO_IOV_PROVIDER_CONNECTABLE
```

#### Start Storages

Got to each Storage CLI terminal and start a provider of the desired type with the desired storage Limit.

Verification: The client provides the function `mcs_demo_list_storages`. All added Storages should be listed here.

#### Client

After the setup phase above the Provider and Storage terminals are not used any more. The exception is the demonstration of restarting storages.

The client provides a function `demo_loop`. This is meant to run continuously.

<!-- \todo move duplicated content into one file -->
