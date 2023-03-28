#
# Makefile to create distribution archives for WinSparkle.
# Needs GNU Make.
#


# The version to package. It is assumed that binaries in the tree are
# for this version.
VERSION := $(shell git describe --always | sed -e 's/^v//g')


sources_base := WinSparkle-$(VERSION)-src
binary_base  := WinSparkle-$(VERSION)
sources_arch := $(sources_base).zip
binary_arch  := $(binary_base).zip
binary_files := \
	bin/sign_update.bat \
	bin/generate_keys.bat \
	include/winsparkle.h \
	include/winsparkle-version.h \
	Release/WinSparkle.dll \
	Release/WinSparkle.lib \
	Release/WinSparkle.pdb \
	x64/Release/WinSparkle.dll \
	x64/Release/WinSparkle.lib \
	x64/Release/WinSparkle.pdb \
	ARM64/Release/WinSparkle.dll \
	ARM64/Release/WinSparkle.lib \
	ARM64/Release/WinSparkle.pdb \
	AUTHORS COPYING NEWS README.md


all: binary sources
binary: $(binary_arch)
sources: $(sources_arch)

$(binary_arch): $(binary_files)
	@rm -rf $(binary_base) $@
	@mkdir $(binary_base)
	@mkdir -p $(binary_base)/bin
	@mkdir -p $(binary_base)/include
	@mkdir -p $(binary_base)/Release
	@mkdir -p $(binary_base)/x64/Release
	@mkdir -p $(binary_base)/ARM64/Release
	for i in $(binary_files); do cp -a $$i $(binary_base)/$$i ; done
	cp -a 3rdparty/expat/expat/COPYING $(binary_base)/COPYING.expat
	zip -9 -r $@ $(binary_base)

$(sources_arch):
	@rm -rf $@
	git ls-files --recurse-submodules | zip -9 -@ $@


clean:
	rm -f $(binary_arch)
	rm -f $(sources_arch)


.PHONY: all binary sources clean
