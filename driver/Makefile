# Makefile for LSM6DSOX Character Driver Skeleton
# Target: Raspberry Pi 4 (aarch64)

# If KERNELRELEASE is defined, we've been invoked from the kernel build system
ifneq ($(KERNELRELEASE),)
	obj-m := vibra_sensor.o
else
	# Path to your Buildroot output kernel directory
	KERNELDIR ?= /home/mayuresh/buildroot/output/build/linux-custom
	
	# Full path to the Buildroot cross-compiler toolchain
	CROSS_COMPILE ?= /home/mayuresh/buildroot/output/host/bin/aarch64-buildroot-linux-gnu-
	
	# Current working directory
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) clean

endif
