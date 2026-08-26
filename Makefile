BUILD_DIR := build
BUILD_TYPE ?= Release
JOBS ?= $(shell nproc)

PAYLOAD_SIZE ?= 256
BUFFER_SIZE ?=
INTERVAL ?=

PRODUCER_BIN := $(BUILD_DIR)/producer/producer
CONSUMER_BIN := $(BUILD_DIR)/consumer/consumer

PRODUCER_ARGS := $(PAYLOAD_SIZE) $(if $(BUFFER_SIZE),--buffer-size $(BUFFER_SIZE))
CONSUMER_ARGS := $(if $(INTERVAL),--interval $(INTERVAL))

.PHONY: help build start-producer start-consumer start-all test clean

help:
	@echo "Targets:"
	@echo "  make build             Configure and build (BUILD_TYPE=$(BUILD_TYPE))"
	@echo "  make start-producer    Run the producer (PAYLOAD_SIZE=$(PAYLOAD_SIZE), BUFFER_SIZE=bytes)"
	@echo "  make start-consumer    Run the consumer (INTERVAL=seconds)"
	@echo "  make start-all         Run producer in the background and consumer in the foreground"
	@echo "  make test              Build and run the unit test suite"
	@echo "  make clean             Remove the build directory"

build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cmake --build $(BUILD_DIR) -j$(JOBS)

start-producer: build
	$(PRODUCER_BIN) $(PRODUCER_ARGS)

start-consumer: build
	$(CONSUMER_BIN) $(CONSUMER_ARGS)

start-all: build
	@$(PRODUCER_BIN) $(PRODUCER_ARGS) & \
	producer_pid=$$!; \
	trap "kill $$producer_pid 2>/dev/null" EXIT INT TERM; \
	sleep 0.3; \
	$(CONSUMER_BIN) $(CONSUMER_ARGS); \
	kill $$producer_pid 2>/dev/null; \
	wait $$producer_pid 2>/dev/null

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

clean:
	rm -rf $(BUILD_DIR)
