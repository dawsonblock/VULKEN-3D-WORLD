SHADERS := $(wildcard *.vert *.frag)
SPV := $(SHADERS:%=%.spv)

.PHONY: all clean

all: $(SPV)

%.spv: %
	glslc $< -o $@

clean:
	rm -f $(SPV)
