all:
	mkdir -p bin
	cd build && make

clean:
	rm -rf ./bin/$(TARGET)
