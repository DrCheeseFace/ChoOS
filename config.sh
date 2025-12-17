SYSTEM_HEADER_PROJECTS="libc kernel"
PROJECTS="libc kernel"

export MAKE=${MAKE:-make -j}
export HOST=${HOST:-$(./default-host.sh)}

export AR=${HOST}-ar
export AS=${HOST}-as
export CC=${HOST}-gcc
export NASM=nasm

export PREFIX=/usr
export EXEC_PREFIX=$PREFIX
export BOOTDIR=/boot
export LIBDIR=$EXEC_PREFIX/lib
export INCLUDEDIR=$PREFIX/include

export CFLAGS='-O0 -ggdb \
 -DDEBUG \
 -fstack-protector-all \
 -Werror \
 -Wpointer-arith -Wcast-align -Wint-conversion -Wstrict-prototypes -Wwrite-strings \
 -Waggregate-return -Wswitch-default -Wunreachable-code \
 -Wunused-parameter -Wuninitialized -Winit-self -Wbad-function-cast -Wcast-align \
 -Wformat=2 -Wlogical-op -Wmissing-include-dirs -Wredundant-decls -Wsequence-point \
 -Wshadow -Wswitch -Wundef -Wunused-but-set-parameter \
 -Wfloat-equal -Wnested-externs \
 -Wpedantic -pedantic-errors'

export NASMFLAGS='-f elf -F dwarf -g'

export CPPFLAGS=''

# Configure the cross-compiler to use the desired system root.
export SYSROOT="$(pwd)/sysroot"
export CC="$CC --sysroot=$SYSROOT"

# Work around that the -elf gcc targets doesn't have a system include directory
# because it was configured with --without-headers rather than --with-sysroot.
if echo "$HOST" | grep -Eq -- '-elf($|-)'; then
  export CC="$CC -isystem=$INCLUDEDIR"
fi

