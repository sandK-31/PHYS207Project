build_directory := build

all:
	cmake -S . -B $(build_directory)
	cmake --build $(build_directory) -j$(nproc)
	@echo "Run this to enable the executable:"
	@echo "source $(CURDIR)/$(build_directory)/setup.sh"

clean:
	rm -rf $(build_directory)
