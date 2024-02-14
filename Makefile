CMAKE := $(shell command -v cmake 2>/dev/null)

all: build-debug build-release

build-debug: proj
	@echo "Building CMake project for 'debug'"
	cd tmp && \
		$(CMAKE) \
			--build . \
			--config DEBUG \
			--parallel $(NPROC)

build-release: proj
	@echo "Building CMake project for 'release'"
	cd tmp && \
		$(CMAKE) \
			--build . \
			--config MINSIZEREL \
			--parallel $(NPROC)

clean:
	@test -d tmp && rm -rf tmp || true

proj:
	@if [ ! -d tmp ]; then \
		mkdir -p tmp || exit 1; \
	fi
	cd tmp && \
		$(CMAKE) -G "Visual Studio 17 2022" \
			../
