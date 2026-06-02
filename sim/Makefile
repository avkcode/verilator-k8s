TOP := packet_router
VERILATOR ?= verilator
VERILATOR_FLAGS ?= -Wall --cc --exe --build -j 0
CXXFLAGS ?= -std=c++17 -O2

.PHONY: run build clean

run: build
	./obj_dir/V$(TOP)

build: $(TOP).sv sim_main.cpp
	$(VERILATOR) $(VERILATOR_FLAGS) \
		-CFLAGS "$(CXXFLAGS)" \
		--top-module $(TOP) $(TOP).sv sim_main.cpp

clean:
	rm -rf obj_dir
