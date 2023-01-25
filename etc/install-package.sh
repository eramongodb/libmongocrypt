#!/usr/bin/env sh

# Install the named packages using the platform's package manager.
# This script "just does the right thing" to install a package as a step in a
#   caching container build, as a bare "pkg-tool install" might be insufficient
#   or subtly broken, and we only really care about installing a list of packages
#
# Usage: __install [pkgs...]

set -eu

if test -f /etc/debian_version ; then
    # We *must* do a repo update as part of the install step in a single cache
    # layer, as a cached apt-get update can become stale while a later
    # apt-get install is invalidated
    apt-get -y update
    # DEBIAN_FRONTEND suppresses all interactivity. For some reason some
    # packages like to prompt as part of their config, even if we're not on
    # a TTY.
    env DEBIAN_FRONTEND=noninteractive \
        apt-get -y install -- "$@"
elif test -f /etc/redhat-release || grep 'ID="amzn"' /etc/os-release >/dev/null 1>&2; then
    if test -f /usr/bin/dnf; then
        # 'dnf' will "do the right thing"
        dnf install -y -- "$@"
    elif test -f /usr/bin/yum; then
        yum install -y -- "$@"
        # 'yum' happily ignores missing packages. Use 'rpm -q' to check that
        # everything we requested actually got installed.
        if ! rpm -q -- "$@"; then
            echo "__install: Failing because one or more packages requested are not available"
            exit 1
        fi
    else
        echo "No package manager here?" 1>&2
        exit 1
    fi
elif test -f /etc/SuSE-brand \
     || (test -f /etc/os-release && grep "opensuse" /etc/os-release); then
    zypper --non-interactive install "$@"
elif test -f /etc/arch-release; then
    pacman -Sy --noconfirm -- "$@"
elif test -f /etc/alpine-release; then
    apk add -- "$@"
else
    echo "We don't know how to manage packages on this system" 1>&2
    exit 1
fi
