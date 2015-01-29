DIRS = MP_Platinum_Rift/source Algorithms MP_Platinum_Rift_2

compile:
	for i in $(DIRS); do make -C $$i; done