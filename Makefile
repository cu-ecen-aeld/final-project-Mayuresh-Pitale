# Path to your Buildroot cross-compiler prefix
CROSS_COMPILE := /home/mayuresh/buildroot/output/host/bin/aarch64-buildroot-linux-gnu-

all:
	$(MAKE) -C driver
	$(MAKE) -C app CXX=$(CROSS_COMPILE)g++

clean:
	$(MAKE) -C driver clean
	$(MAKE) -C app clean
