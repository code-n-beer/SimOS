# step 1: build x86_64-elf-binutils and x86_64-elf-gcc
FROM archlinux/base as toolbuild

COPY ./tools /tools

RUN pacman -Sy --noconfirm archlinux-keyring \
    && pacman -Syu --noconfirm --needed base-devel sudo \
    && pacman -Scc --noconfirm \
    && useradd -M -s /bin/sh builder \
    && echo 'builder ALL = NOPASSWD: /usr/bin/pacman' >> /etc/sudoers \
    && chown -R builder:builder /tools

# makepkg must be run as a separate user
USER builder
WORKDIR /tools
RUN makepkg -msip PKGBUILD.binutils --needed --noconfirm --config /tools/makepkg.conf \
    && makepkg -msp PKGBUILD.gcc --needed --noconfirm --config /tools/makepkg.conf

# step 2: build the actual build environment
FROM archlinux/base

COPY --from=toolbuild /tools/packages /tools

# base-devel needs to be installed because otherwise meson will complain
RUN pacman -Sy --noconfirm archlinux-keyring \
    && pacman -Syu --noconfirm --needed base-devel meson ninja \
    && pacman -Scc --noconfirm \
    && pacman -U --noconfirm /tools/*.tar.xz \
    && rm -rf /tools

RUN pacman -Sy --noconfirm mtools grub xorriso

# keep running
CMD ["tail", "-f",  "/dev/null"] 
