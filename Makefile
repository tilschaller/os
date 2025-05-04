.PHONY: run
run:
	./scripts/qemu.sh

bear: clean
	bear -- $(MAKE)

.PHONY: clean 
clean: 
	./scripts/clean.sh
