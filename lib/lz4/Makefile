ccflags-y += -Ofast -mcpu=cortex-a53 -mtune=cortex-a53 -mfpu=asimd-vfpv4 -march=armv8-a

subdir-ccflags-y += -mllvm -inline-threshold=3750
subdir-ccflags-y += -mllvm -unroll-threshold=2000
subdir-ccflags-y += -mllvm -inlinehint-threshold=1800

ccflags-y += -mllvm -inline-threshold=3750
ccflags-y += -mllvm -unroll-threshold=2000
ccflags-y += -mllvm -inlinehint-threshold=1800

obj-$(CONFIG_LZ4_COMPRESS) += lz4_compress.o
obj-$(CONFIG_LZ4HC_COMPRESS) += lz4hc_compress.o
obj-$(CONFIG_LZ4_DECOMPRESS) += lz4_decompress.o

obj-$(CONFIG_ARM64) += $(addprefix lz4armv8/, lz4accel.o lz4armv8.o)
