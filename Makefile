DIRS = "./MP - Platinum Rift/source" "./Algorithms"

compile:
	for i in $(DIRS); do make -c $$i; done