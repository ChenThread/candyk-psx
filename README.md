CandyK-PSX: A candy shop of tools and libraries for PlayStation development

This readme will need to be better fleshed-out.

## Licensing

CandyK, as a whole, is licensed under the "zlib license". Please check the LICENSE file for more information.

### Exceptions

Toolchain examples are licensed under the Creative Commons Zero license - feel absolutely free to base your code on them!

## Installation

### Binary

1. Install or compile pacman (the Arch Linux package manager).
2. Append the following to /etc/pacman.conf:

```
[candyk]
Server = http://candyk.asie.pl/repo/x86_64
downloads.devkitpro.org/packages
```

3. `pacman -Syu`
4. `pacman -S candyk-psx`

### Source

TODO

## Architecture

### Directories

* `bin/`: Target directory for host-native tools
* `lib/`: Target directory for PlayStation libraries
* `src/`: Source code for PlayStation libraries
* `toolsrc/`: Source code for host-native tools

### Makefile system

TODO: formalise this.

Basically there's a bunch of `target.make` files which are included as needed.

