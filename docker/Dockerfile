FROM alpine:latest

# Habilita repos (main + community) e instala toolchain AVR
RUN set -eux; \
    echo "https://dl-cdn.alpinelinux.org/alpine/v3.21/main" > /etc/apk/repositories; \
    echo "https://dl-cdn.alpinelinux.org/alpine/v3.21/community" >> /etc/apk/repositories; \
    apk update && apk upgrade; \
    apk add --no-cache \
    make \
    gcc-avr \
    avr-libc \
    binutils-avr

WORKDIR /work
