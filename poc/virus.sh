#!/bin/bash

set -e

set -x

set -o pipefail

python3 tar.py 

python3 -m http.server --bind 172.17.0.1 3000