#!/bin/sh
# SPDX-License-Identifier: MIT
# Installs, updates or removes frame-perf-redux (a fork of sasaken1102r's frame-perf-overlay) from the
# latest GitHub release. Run it on the Steam Frame as the normal user, for example in Konsole:
#
#   curl -fsSL https://raw.githubusercontent.com/flamingonads/frame-perf-redux/redux/install-redux.sh | sh
#   curl -fsSL https://raw.githubusercontent.com/flamingonads/frame-perf-redux/redux/install-redux.sh | sh -s -- uninstall
#
# Anything else after "--" is passed to the release's install.sh (for example --no-autostart).
#
# It finds the latest release, downloads its tar.gz and SHA256SUMS, refuses to go on if the checksum
# doesn't match, checks the archive holds only plain files and folders, extracts it into a temporary
# folder below the home folder, runs the release's own install.sh, and removes the temporary folder.
# No sudo; nothing is written outside the home folder.
#
# The release lookup, checksum and archive checks follow sasaken1102r's installer
# (https://github.com/sasaken1102r/frame, MIT license).
#
# Test override: FRAME_REDUX_DRY_RUN=1 does everything except running install.sh.

set -eu

repo="flamingonads/frame-perf-redux"
github="https://github.com"
app="frame-perf-overlay"

say() { printf '%s\n' "$*"; }
die() { printf 'frame-perf-redux: %s\n' "$*" >&2; exit 1; }

for tool in curl tar gzip sha256sum; do
    command -v "$tool" >/dev/null 2>&1 || die "needs $tool, which isn't installed"
done
[ "$(uname -m)" = "aarch64" ] || die "this is for the Steam Frame (aarch64); this machine is $(uname -m)"
[ "$(id -u)" -ne 0 ] || die "run it as your normal user, not with sudo"
[ -n "${HOME:-}" ] && [ -d "$HOME" ] || die "no home folder"

action=install
if [ "${1:-}" = "uninstall" ]; then
    action=uninstall
    shift
elif [ "${1:-}" = "install" ]; then
    shift
fi
[ "${1:-}" = "--" ] && shift

work=$(mktemp -d "$HOME/.frame-perf-redux-install.XXXXXX") || die "can't make a temporary folder"
trap 'rm -rf "$work"' EXIT INT TERM

fetch() { # url output
    curl --silent --show-error --fail --location --proto '=https' --connect-timeout 15 --max-time 300 \
        -o "$2" "$1"
}

# The latest release's tag, from where github.com/<repo>/releases/latest redirects to (no API rate limit)
location=$(curl --silent --show-error --head --proto '=https' --connect-timeout 15 --max-time 60 \
    -o /dev/null -w '%{redirect_url}' "$github/$repo/releases/latest") || die "can't reach GitHub"
case $location in
    "$github/$repo/releases/tag/"?*) tag=${location#"$github/$repo/releases/tag/"} ;;
    *) die "no release found at $github/$repo/releases" ;;
esac
case $tag in
    *[!A-Za-z0-9._-]*) die "unexpected release tag: $tag" ;;
esac
version=${tag#v}
asset="$app-$version.tar.gz"
base="$github/$repo/releases/download/$tag"
say "frame-perf-redux $version"

fetch "$base/$asset" "$work/$asset" || die "can't download $asset"
fetch "$base/SHA256SUMS" "$work/SHA256SUMS" || die "the release has no SHA256SUMS, so it can't be checked; not installing"

# Checksum: the line for this file must exist and match
expected=$(awk -v f="$asset" '$2 == f || $2 == "*" f { print $1; exit }' "$work/SHA256SUMS")
case $expected in
    '') die "SHA256SUMS has no entry for $asset" ;;
    *[!0-9a-f]*) die "SHA256SUMS has a malformed entry for $asset" ;;
esac
[ ${#expected} -eq 64 ] || die "SHA256SUMS has a malformed entry for $asset"
actual=$(sha256sum "$work/$asset" | cut -c1-64)
[ "$actual" = "$expected" ] || die "checksum mismatch for $asset (broken or changed download); not installing"
say "  checksum OK"

# Archive: only plain files and folders, all below the extraction folder
if command -v python3 >/dev/null 2>&1; then
    python3 - "$work/$asset" <<'EOF' || die "unsafe archive; not installing"
import sys, tarfile
with tarfile.open(sys.argv[1], "r:gz") as t:
    for m in t.getmembers():
        name = m.name
        parts = name.split("/")
        if name.startswith("/") or ".." in parts or not (m.isfile() or m.isdir()):
            sys.exit(1)
EOF
else
    tar -tzf "$work/$asset" >"$work/list" || die "can't read the archive"
    if grep -E '(^/|(^|/)\.\.(/|$))' "$work/list" >/dev/null; then die "unsafe archive; not installing"; fi
    tar -tvzf "$work/$asset" | cut -c1 | grep -v '^[-d]$' >/dev/null && die "unsafe archive; not installing"
fi

mkdir "$work/files"
tar -xzf "$work/$asset" -C "$work/files" || die "can't extract $asset"
if [ -f "$work/files/install.sh" ]; then
    dir="$work/files"
elif [ -f "$work/files/$app/install.sh" ]; then
    dir="$work/files/$app"
else
    die "no install.sh in the release"
fi

if [ "$action" = "uninstall" ]; then
    set -- --uninstall
fi
if [ "${FRAME_REDUX_DRY_RUN:-}" = "1" ]; then
    say "  [dry run] not running: ./install.sh $*"
    exit 0
fi
chmod u+x "$dir/install.sh"
(cd "$dir" && ./install.sh "$@") </dev/null
