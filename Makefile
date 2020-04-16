all:
	(cd raft.git && make clean && make)
	(cd nvml;make clean;make)
#	(cd core;make clean;make)
#	(cd test;make clean;make)
