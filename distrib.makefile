#
# Makefile to create distribution archives for WinSparkle.
# Needs GNU Make.
#


# The version to package. It is assumed that binaries in the tree are
# for this version.
VERSION := $(shell bzr tags | head -n 1 | sed -e 's/release-\([0-9.a-zA-Z_-]*\).*/\1/g')


sources_arch := WinSparkle-$(VERSION)-src.7z
binary_arch := WinSparkle-$(VERSION).zip
binary_files := \
	AUTHORS COPYING NEWS README \
	Release/WinSparkle.dll \
	Release/WinSparkle.lib \
	include/winsparkle.h \
	include/winsparkle-version.h \




all: $(binary_arch) $(sources_arch)


$(binary_arch): $(binary_files)
	rm -f $@
	zip -9 --junk-paths $@ $(binary_files)

$(sources_arch):
	rm -rf WinSparkle-$(VERSION) $@
	bzr export --format=dir WinSparkle-$(VERSION)
	7z a -m0=lzma -mx=9 $@ WinSparkle-$(VERSION)
	rm -rf WinSparkle-$(VERSION)


clean:
	rm -f $(binary_arch)
	rm -f $(sources_arch)


.PHONY: all clean
