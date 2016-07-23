Building
========

Dependencies
------------

### Tested on Ubuntu 14.04:

- clang

Use the PPA:

```
wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key|sudo apt-key add -
```

Add to `/etc/apt/sources.list`:

```
deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.8 main
```

Then:

```
sudo apt-get update
sudo apt-get install -y clang-3.8 lldb-3.8 clang-format-3.8
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-3.8 100
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-3.8 100
sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-3.8 100
```

- nasm

```
sudo apt-get install -y nasm
```

- grub

Should come with Ubuntu 14.04.

- xorriso (for grub)

```
sudo apt-get install -y xorriso
```

- qemu

```
sudo apt-get install -y qemu
```

- bochs

```
sudo apt-get install -y bochs bochs-sdl
```

