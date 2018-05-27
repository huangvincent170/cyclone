all:
	(cd raft && make clean && make;yes)
	(cd nvml;make clean;make )
	(cd core;make clean;make)
	(cd test;make clean;make)
