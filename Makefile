.PHONY: iso 
iso: 
	./scripts/iso.sh

.PHONY: run
run:
	./scripts/qemu.sh

.PHONY: exception
exception: 
	./scripts/qemu-exception.sh

.PHONY: bear
bear: clean
	bear -- $(MAKE)

.PHONY: clean 
clean: 
	./scripts/clean.sh
