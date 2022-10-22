#!/bin/bash
set -e
set -x

docker rm polemarch -f

docker run -d --name polemarch --restart always \
    -v /opt/polemarch/projects:/projects \
    -v /opt/polemarch/hooks:/hooks \
    -p 8080:8080 \
    vstconsulting/polemarch:1.8.5