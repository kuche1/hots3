#! /usr/bin/env bash

set -euo pipefail

HERE=$(dirname -- $(readlink -f -- "$BASH_SOURCE"))

CONVERTER='ascii-image-converter --color --complex'

# TODO auto convert all image files
$CONVERTER "$HERE/lobby-ready.jpg" > "$HERE/lobby-ready.txt"
