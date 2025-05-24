.PHONY: run
run:
	./scripts/qemu.sh

exception: 
	./scripts/qemu-exception.sh

bear: clean
	bear -- $(MAKE)

.PHONY: clean 
clean: 
	./scripts/clean.sh
