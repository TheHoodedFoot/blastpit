# syntax=docker/dockerfile:1

# Build with docker buildx build -t wine .

FROM gentoo/portage AS portage
FROM gentoo/stage3:i686-openrc AS gentoo

COPY --from=portage /var/db/repos /var/db/repos

# Set QEMU targets
COPY <<EOF /etc/portage/make.conf
COMMON_FLAGS="-O2 -march=i686 -pipe"
CFLAGS="\${COMMON_FLAGS}"
CXXFLAGS="\${COMMON_FLAGS}"
FCFLAGS="\${COMMON_FLAGS}"
FFLAGS="\${COMMON_FLAGS}"

LC_MESSAGES=C.utf8

QEMU_USER_TARGETS="aarch64"
QEMU_SOFTMMU_TARGETS="\${QEMU_USER_TARGETS}"
EOF

# Fix for Perl (as at Wed 18 Sep 19:45:02 BST 2024)
COPY <<EOF /etc/portage/package.mask/docker.mask
=dev-lang/perl-5.40.0
EOF

# Run in privileged mode to prevent Gentoo sandbox warnings
# RUN --security=insecure USE="X text -clang -gstreamer -alsa -sdl -unwind -vulkan -fontconfig -gecko -mono -mingw" emerge -pv virtual/wine app-emulation/qemu
RUN USE="X text -clang -gstreamer -alsa -sdl -unwind -vulkan -fontconfig -gecko -mono -mingw" emerge virtual/wine app-emulation/qemu

# Create an unprivileged user
RUN groupadd -g 1001 user
RUN useradd -m -u 1001 -g 1001 -G users -s /bin/bash user

# Set user password

# Default command
CMD [ "/bin/bash" ]
