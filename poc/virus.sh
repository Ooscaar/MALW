#!/bin/bash

set -e

set -x

set -o pipefail

python3 tar.py 

python3 -m http.server 3000