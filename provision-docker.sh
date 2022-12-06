#!/bin/bash

# Polemarch server configuration (Ubuntu 20.04) running on Docker
#
# Considerations
# =================
# - This script is intended to be run on a fresh Ubuntu 20.04 server.
# - It will create all the necessary files and directories for Polemarch.
# - It will start the polemarch server as ROOT 
#
# Architecture
# ============
# - Polemarch is installed in a virtualenv
# - The virtualenv is located in /opt/polemarch
# - The database is a sqlite (default) database

## Create virtualenv
# For Debian 10 use python3.7
# For rhel/centos7 use python3.6
# In some cases use sudo for first command.
virtualenv --python=python3.8 /opt/polemarch
sudo mkdir -p /etc/polemarch
source /opt/polemarch/bin/activate

# Install polemarch
pip install tzdata
pip install -U polemarch==1.8.5

# Create additional directories
mkdir /opt/polemarch/logs /opt/polemarch/pid

mkdir /projects
mkdir /hooks

# Create config at /etc/polmarch/settings.ini
# See https://polemarch.readthedocs.io/en/latest/installation.html#configuration
cat <<EOF > /etc/polemarch/settings.ini
[main]
projects_dir = /projects
hooks_dir = /hooks

[uwsgi]
pidfile = /run/web.pid
addrport = 0.0.0.0:8080
vacuum = True
max-requests = 1000
max-worker-lifetime = 3600
worker-reload-mercy = 60
http-keepalive = true
http-auto-chunked = true
thread-stacksize = 1024
EOF
