FROM alpine:latest AS build
WORKDIR /app/toolchain
RUN apk add --no-cache wget
RUN wget https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz
RUN tar -xvf gcc-13.2.0.tar.gz
RUN wget https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.gz
RUN tar -xvf binutils-2.41.tar.gz
ARG PREFIX="/app/Toolchain/i686-elf"
ARG TARGET="i686-elf"
ARG PATH="${PREFIX}/bin:$PATH"
RUN apk add --no-cache mtools alpine-sdk build-base gcc git bash texinfo g++ zlib-dev
RUN apk add --no-cache gmp-dev mpfr-dev mpc1-dev
RUN apk add --no-cache nasm
WORKDIR /app/toolchain/binutils-build
RUN ../binutils-2.41/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
RUN make -j8
RUN make install
WORKDIR /app/toolchain/gcc-build
RUN ../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
RUN make -j8 all-gcc
RUN make install-gcc
RUN make -j8 all-target-libgcc
RUN make install-target-libgcc 

RUN apk add --no-cache dosfstools parted
WORKDIR /app
COPY . /app
ARG MAKE_CMD
RUN make ${MAKE_CMD}

FROM scratch AS export-stage
COPY --from=build /app/build .
