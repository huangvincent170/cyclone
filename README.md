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

##Code structure

git submodule update --init --recursive
