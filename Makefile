.PHONY: iso 
iso: 
	./scripts/iso.sh

.PHONY: run
run:
	./scripts/qemu.sh

.PHONY: bear
bear: clean
	bear -- $(MAKE)

.PHONY: clean 
clean: 
	./scripts/clean.sh
