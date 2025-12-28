#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/pissos.kernel isodir/boot/pissos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "pissos" {
	multiboot /boot/pissos.kernel
}
EOF
grub-mkrescue -o pissos.iso isodir
