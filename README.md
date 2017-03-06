CandyK: A candy shop of tools and libraries for PlayStation development

This readme will need to be better fleshed-out.

By the way, you will need a MIPS cross-compiler. This toolchain assumes that you use `mipsel-none-elf` as your target name, but if you don't use this you can always change the `CROSSPREFIX` variable.

## Licences

To be decided.

### Exceptions

The `dat/isolicence.*` files are required in order to make a bootable PlayStation disc, and cannot be replaced.

`iso2raw` is of unknown copyright status.

## Quick notes for building a GCC toolchain

TODO: actually go into depth into how this is done and provide scripts so people don't have to think.

* binutils: configure, make, sudo make install
* gcc: configure, make all-gcc, sudo make install-gcc
* newlib: configure, make, sudo make install
* back to gcc: make; sudo make install

If you want C++ support, you will have to comment out the line in a libstdc++ configure script which says that the target is not supported. It builds fine. It probably works.

## Architecture

### Directories

* `bin/`: Target directory for host-native tools
* `lib/`: Target directory for PlayStation libraries
* `src/`: Source code for PlayStation libraries
* `toolsrc/`: Source code for host-native tools

### Makefile system

TODO: formalise this.

Basically there's a bunch of `target.make` files which are included as needed.

