#!/bin/bash

# Polemarch server configuration (Ubuntu 20.04)
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
# - The database is a mysql database

# Install dependencies
apt-get update
sudo apt-get install -y python3-virtualenv python3.8 python3.8-dev gcc libffi-dev libkrb5-dev libffi7 libssl-dev libyaml-dev libsasl2-dev libldap2-dev default-libmysqlclient-dev sshpass git

## Install mysql-server
sudo apt-get install -y default-mysql-server

## Create database
sudo -H mysql <<QUERY_INPUT
# uncomment this string on old MariaDB/MySQL versions
# SET @@global.innodb_large_prefix = 1;
create user db_user identified by 'db_password';
create database db_name default CHARACTER set utf8 default COLLATE utf8_general_ci;
grant all on db_name.* to 'db_user';
QUERY_INPUT

mysql_tzinfo_to_sql /usr/share/zoneinfo | sudo -H mysql mysql

## Create virtualenv
# For Debian 10 use python3.7
# For rhel/centos7 use python3.6
# In some cases use sudo for first command.
virtualenv --python=python3.8 /opt/polemarch
sudo mkdir -p /etc/polemarch
source /opt/polemarch/bin/activate

# Install polemarch
pip install -U polemarch[mysql]

# Create additional directories
mkdir /opt/polemarch/logs /opt/polemarch/pid

# Create config at /etc/polmarch/settings.ini
# See https://polemarch.readthedocs.io/en/latest/installation.html#configuration
cat <<EOF > /etc/polemarch/settings.ini
[database]
engine = django.db.backends.mysql
name = db_name
user = db_user
password = db_password

[database.options]
connect_timeout = 20
init_command = SET sql_mode='STRICT_TRANS_TABLES', default_storage_engine=INNODB, NAMES 'utf8', CHARACTER SET 'utf8', SESSION collation_connection = 'utf8_unicode_ci'

[uwsgi]
harakiri = 120
vacuum = True
http-keepalive = true
pidfile = /opt/polemarch/pid/polemarch.pid
log_file = /opt/polemarch/logs/polemarch_web.log
# Uncomment it for HTTPS and install `uwsgi` pypi package to env:
# addrport = 127.0.0.1:8080
# https = 0.0.0.0:443,/etc/polemarch/polemarch.crt,/etc/polemarch/polemarch.key

[worker]
# output will be /opt/polemarch/logs/polemarch_worker.log
logfile = /opt/polemarch/logs/{PROG_NAME}_worker.log
# output will be /opt/polemarch/pid/polemarch_worker.pid
pidfile = /opt/polemarch/pid/{PROG_NAME}_worker.pid
loglevel = INFO
EOF

# Migrate
polemarchctl migrate

# Start polemarch
polemarchctl webserver