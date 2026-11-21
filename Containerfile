FROM registry.gitlab.steamos.cloud/proton/sniper/sdk:3.0.20250519.130773-0

ARG MESON_VERSION=1.7.0
RUN curl -sSfL "https://github.com/mesonbuild/meson/releases/download/${MESON_VERSION}/meson-${MESON_VERSION}.tar.gz" \
        | tar -xz -C /opt \
    && ln -sf "/opt/meson-${MESON_VERSION}/meson.py" /usr/local/bin/meson \
    && meson --version

RUN dpkg --add-architecture i386 && apt-get update && apt-get install -y --no-install-recommends \
        libsqlite3-dev libsqlite3-dev:i386 \
        libpsl-dev libpsl-dev:i386 \
        libbrotli-dev libbrotli-dev:i386 \
    && rm -rf /var/lib/apt/lists/*

RUN set -eu; \
    apt-get update; \
    cd /tmp; \
    apt-get download libnghttp2-dev=1.43.0-1+deb11u1; \
    dpkg-deb -x libnghttp2-dev_*.deb /tmp/nghttp2; \
    cp -a /tmp/nghttp2/usr/include/. /usr/include/; \
    for triplet in x86_64-linux-gnu i386-linux-gnu; do \
        ln -sf libnghttp2.so.14 "/usr/lib/$triplet/libnghttp2.so"; \
        sed -e "s|^libdir=.*|libdir=/usr/lib/$triplet|" \
            /tmp/nghttp2/usr/lib/x86_64-linux-gnu/pkgconfig/libnghttp2.pc \
            > "/usr/lib/$triplet/pkgconfig/libnghttp2.pc"; \
        PKG_CONFIG_LIBDIR="/usr/lib/$triplet/pkgconfig" pkg-config --modversion libnghttp2; \
    done; \
    rm -rf /tmp/nghttp2 /tmp/libnghttp2-dev_*.deb /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y --no-install-recommends \
        libffi-dev libffi-dev:i386 \
        libpcre3-dev libpcre3-dev:i386 \
        zlib1g-dev zlib1g-dev:i386 \
        libmount-dev libmount-dev:i386 \
        libselinux1-dev libselinux1-dev:i386 \
        gettext \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y --no-install-recommends \
        libwayland-dev libwayland-dev:i386 \
        libwayland-bin \
        libxkbcommon-dev libxkbcommon-dev:i386 \
        libxkbregistry-dev libxkbregistry-dev:i386 \
    && rm -rf /var/lib/apt/lists/*

RUN set -eu; \
    printf '%s\n' \
        '[binaries]' \
        "c = ['gcc', '-m32']" \
        "cpp = ['g++', '-m32']" \
        "ar = 'ar'" \
        "strip = 'strip'" \
        "pkg-config = 'pkg-config'" \
        '' \
        '[properties]' \
        'needs_exe_wrapper = false' \
        "pkg_config_libdir = '/usr/lib/i386-linux-gnu/pkgconfig:/usr/share/pkgconfig'" \
        '' \
        '[host_machine]' \
        "system = 'linux'" \
        "cpu_family = 'x86'" \
        "cpu = 'i686'" \
        "endian = 'little'" \
        > /tmp/i386.cross; \
    curl -sSfL https://download.gnome.org/sources/glib/2.70/glib-2.70.5.tar.xz | tar -xJ -C /tmp; \
    cd /tmp/glib-2.70.5; \
    meson setup _b64 --prefix=/usr --libdir=lib/x86_64-linux-gnu \
        --buildtype=release -Dtests=false -Dselinux=disabled; \
    ninja -C _b64 install; \
    meson setup _b32 --prefix=/usr --libdir=lib/i386-linux-gnu \
        --buildtype=release -Dtests=false -Dselinux=disabled \
        --cross-file /tmp/i386.cross; \
    ninja -C _b32 install; \
    cd /; rm -rf /tmp/glib-2.70.5 /tmp/i386.cross; \
    ldconfig; \
    pkg-config --modversion glib-2.0; \
    PKG_CONFIG_LIBDIR=/usr/lib/i386-linux-gnu/pkgconfig pkg-config --modversion glib-2.0

RUN apt-get update && apt-get install -y --no-install-recommends \
        patchelf sassc librsvg2-bin librsvg2-dev \
    && rm -rf /var/lib/apt/lists/*

# The base image's cargo predates Rosemary's 2024 edition.
ENV RUSTUP_HOME=/usr/local/rustup \
    CARGO_HOME=/usr/local/cargo \
    PATH=/usr/local/cargo/bin:$PATH
ARG RUST_CHANNEL=1.100.0
RUN apt-get update && apt-get install -y --no-install-recommends \
        libcurl4-openssl-dev zlib1g-dev libxkbcommon-dev \
    && rm -rf /var/lib/apt/lists/* \
    && curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs \
        | sh -s -- -y --no-modify-path --profile minimal --default-toolchain $RUST_CHANNEL \
    && rustc --version && cargo --version
