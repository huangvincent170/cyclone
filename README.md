## Blizzard - Reliable and persistent data-structures

Managing persistent state has always been a challenge. The traditional block based
persistent storage devices such as SSDs are orders of magnitude slower than DRAM memory.
Furthermore, these devices introduce impdence mismatch between application structures and
persistent data.

Byte-addressable persistent memory (a.k.a Non-volatile memory, NVM)  allow maintaining persistent state at the granularity
of a byte. These new devices have DRAM like latencies, therefore can blur the line between
volatile and persistent application state. In this project we explore an efficient
mechanism to reliably maintain persistent application state. The common wisdom is to use 
NVM with RDMA based replicaiton to provide data durability and reliability. This we identify
as physical operation replication, where each and every update to NVM get replicated over to remote nodes
for reliability. In this project we explore the feasibility of using logical replication for achieving the
same. We replicate logical operation to the remote node, as opposed to value replication.

We show that with carefully optimized NVM aware operation replication layer + NVM aware data structures, we can provide
both persistent and reliability gurantees for application data with minimal overhead.

## Git Branches

* tcp_tunnel - Blizzard replication engine with TCP/IP as the transport protocol.

## Code structure

We are using Intel's Cyclone codebase as our starting point. The Cylone implements a efficient log replication
layer based on Intel DPDK user-space network stack. However, the original Intel software was supporting DRAM + SSD
as their storage engine. We port them to be NVM aware.
We only describe important directories. Some of the directories are not used anymore.

* core -  Intel DPDK based NVM aware log replication layer. We extend the software with NVM awareness. 
* raft - RAFT replication engine used for operation replication. 
* utils-arch-cluster - Contains, configuration files for RAFT cluster configuration
* utils-gloabl - Scripts for server side cluster deployment and client deployment.
* test - Workloads/data-structures implmentations that work with operation replication.
* pmdk - An external dependency for Intel's PMDK code base. We use pmem transactional objects as a dependency.

## Dependencies.

### Software
* pkg-config
* Cmake
* Intel DPDK software stack.
Install DPDK network stack on server/client machines using DPDK installation [guide](http://core.dpdk.org/doc/quick-start/).

### Hardware

* 10/100GigE ethernet connected machines. Minimum number of machines needed is two (Server and client).
NVM persistent memory modules. Alternatively, we can emulate NVM using DRAM.

## Build code

First we clone external code in to project git tree by;

```git submodule update --init --recursive```

Form the top level directory, issue

```make```

This will build the ```raft``` and Blizzard ```core``` libraries. 
Next, build the Blizzard applications/data-structures by issueing ```make``` from the 
```test``` directory.

## Run code

First, configure the cluster details in ```utils-arch-cluster``` directory.
* Configuring machine details in ```cluster-dpdk.ini```
* Configuring cluster configurations in ```example.ini```

Next,from ```utils-gloabl``` directory, execute;

```numbers.sh```

The script is a driver for ```runscript.py``` file. Test

