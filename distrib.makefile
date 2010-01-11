#
# Makefile to create distribution archives for WinSparkle.
# Needs GNU Make.
#


# The version to package. It is assumed that binaries in the tree are
# for this version.
VERSION := $(shell git describe | sed -e 's/^v//g')


sources_base := WinSparkle-$(VERSION)-src
binary_base  := WinSparkle-$(VERSION)
sources_arch := $(sources_base).7z
binary_arch  := $(binary_base).zip
binary_files := \
	AUTHORS COPYING NEWS README \
	Release/WinSparkle.dll \
	Release/WinSparkle.lib \
	include/winsparkle.h \
	include/winsparkle-version.h \




all: $(binary_arch) $(sources_arch)

$(binary_arch): $(binary_files)
	@rm -f $(binary_base) $@
	@mkdir $(binary_base)
	cp -ra $(binary_files) $(binary_base)
	zip -9 -r  $@ $(binary_base)
	@rm -rf $(binary_base) $(binary_base).tar

$(sources_arch):
	@rm -rf $(sources_base) $@
	git archive --prefix=$(sources_base)/ -o $(sources_base).tar HEAD
	tar xf $(sources_base).tar
	7z a -m0=lzma -mx=9 $@  $(sources_base)
	@rm -rf $(sources_base) $(sources_base).tar


clean:
	rm -f $(binary_arch)
	rm -f $(sources_arch)


.PHONY: all clean
