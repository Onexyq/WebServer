all:
	mkdir -p bin
	cd build && $(MAKE)

.PHONY:
clean:
	rm -rf ./bin/$(TARGET)
