# MCSS Demo

* [Overview](#overview)
* [Requirements](#requirements)
* [step by step guide](#step-by-step-guide)
  * [1. get nodes](#1-get-nodes)
  * [2. prepare ssh access from orchestration node to participating nodes](#2-prepare-ssh-access-from-orchestration-node-to-participating-nodes)
  * [3. prepare directories and files](#3-prepare-directories-and-files)
    * [build and install the binaries](#build-and-install-the-binaries)
    * [adapt `.bashrc`](#adapt-bashrc)
    * [configure `meta.env`](#configure-metaenv)
    * [Adapt orchestrator.ENV](#adapt-orchestratorenv)
  * [4. run the demo](#4-run-the-demo)
    * [Open all Terminals](#open-all-terminals)
    * [Start Provider](#start-provider)
    * [Start Storages](#start-storages)
    * [Client](#client)

## Overview

This demo relies on at least 4 "demo nodes", these are grouped into one "orchestration node" and arbitrary many "participating nodes".

The orchestration node is the node used by the presenting person to run (`start_mcs_demo`, `stop_mcs_demo`) and present the demo.

`start_mcs_demo` will open possibly multiple terminals per participating node and establishes a ssh connection. The amount of open terminals and the available commands are dependent on the type of participating node. These are one of the following:

* __One__ Provider: the `mcs_iov_backend_provider` runs on this node.
* __Multiple__ Storage(s): one or more (currently 4) `mcs_iov_backend_storage_provider` that are registered to the `mcs_iov_backend_provider`.
* __One__ Client: the client node that gets access to the storages via the state queried from the Provider.

The appearance and content for each of these terminals is defined in `orchestration.env`

__Note:__ Configuration of this demo is done at two locations:

* `bashrc`: exports `MCS_DEMO_ROOT`.
* `meta.ENV` configures everything else.

(details below)

The participating nodes rely on TCP/IP communication among each other.

## Requirements

* `ssh` access to all nodes.
* All nodes has to be able to reach each other via IP address.
* A shared directory that is accessible on all nodes.
* `pdsh` installed on orchestration node
* `htop` installed on participating nodes.
* `xterm` installed on all participating nodes.

## step by step guide

### 1. get nodes

A temporary allocation on beehive:

```bash
salloc -N7 -t <time_in_minutes> --exclusive --x11=all
```

Or, with reservation on beehive:

```bash
 salloc --reservation=<reservation_id> -N 7 --x11=all
 ```

 Show all current reservations:

 ```bash
 scontrol show reservations
 ```

### 2. prepare ssh access from orchestration node to participating nodes

1) Make sure you have password-less access from the orchestration node to all participating nodes.
2) Additionally it is recommended to prevent strict host key checking. Otherwise windows might pop up and ask if the node should be added to the list of known hosts.

In order to do both add following section to `~/.ssh/config`

```bash
Host XXX.XXX.XXX.* # the common part of the IP address of each node.
  User <your_user_name>
  IdentityFile </path/to/identity/file>
  PreferredAuthentications publickey
  StrictHostKeyChecking no
```

You can omit `StrictHostKeyChecking no` and still prevent the checking by manually connecting once to each participating nodes.

### 3. prepare directories and files

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
if [ -f ${MCS_DEMO_ROOT:?}/mcs/src/mcs/core/bin/demo/campaign/orchestrated/meta.ENV ]; then
  . ${MCS_DEMO_ROOT:?}/mcs/src/mcs/core/bin/demo/campaign/orchestrated/meta.ENV
  . ${MCS_DEMO_ROOT:?}/mcs/src/mcs/core/bin/demo/campaign/orchestrated/decide_env.ENV
fi
```

#### configure `meta.env`

If necessary adapt the following:

* `MCS_DEMO_IP`: common part of the IP Address of all participating nodes inside the function `MCS_DEMO_IP`. Note: omit leading zeros.
* `MCS_DEMO_PARTICIPATING_NODE_IDS`: list all node ids, except the orchestration node id.
* `MCS_DEMO_STORAGE_NODE_IDS`: which of the participating nodes are to be storages.
* `MCS_DEMO_PROVIDER_NODE_ID`: which of the participating nodes is to be the provider.
* `MCS_DEMO_CLIENT_NODE_ID`: which of the participating nodes is to be the client.

#### Adapt orchestrator.ENV

The current setup is optimized for a 32 inch screen. If necessary you can change the geometries of the windows.

You can move the windows to the desired location and use `xwininfo` to print out its geometry.

### 4. run the demo

#### Open all Terminals

Open an ssh connection to the orchestration node (with `-XY`) and execute `start_mcs_demo`. All widows open and connect to the respective machines.

#### Start Provider

Go to Provider interactive terminal and run `mcs_demo_start_backend_provider`.

This is blocking. The output should be the connection information. For example: `ip::tcp (Address "192.168.153.30", 9876)`.

Verification: The variable `MCS_DEMO_IOV_PROVIDER_CONNECTABLE` hold a filename. that file should have the same content as the output from `mcs_demo_start_backend_provider`. This file should be accessible from any other participating node. For example from the client node this can look like the following:

```bash
[MCS_CLIENT] ~$ cat ${MCS_DEMO_IOV_PROVIDER_CONNECTABLE:?}
ip::tcp (Address "192.168.153.30", 9876)
[MCS_CLIENT] ~$ echo ${MCS_DEMO_IOV_PROVIDER_CONNECTABLE:?}
/home/zimmermannp/mcs_demo/MCS_DEMO_IOV_PROVIDER_CONNECTABLE
```

#### Start Storages

Got to each Storage interactive terminal and start a provider of the desired type with the desired storage Limit.

Verification: The client provides the function `mcs_demo_list_storages`. All added Storages should be listed here.

#### Client

After the setup phase above the Provider and Storage terminals are not used any more. The exception is the demonstration of restarting storages.

The client provides a function `demo_loop`. This is meant to run continuously.

<!-- \todo move duplicated content into one file -->
