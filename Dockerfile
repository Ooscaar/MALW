# Use the official Ubuntu image as a base
FROM ubuntu:20.04

# First run apt-get update to get the latest package lists
RUN apt-get update

# Install the packages you need
RUN apt-get install -y \
    sudo \
    python3-virtualenv \
    python3.8 \
    python3.8-dev \
    gcc \
    libffi-dev \
    libkrb5-dev \
    libffi7 \
    libssl-dev \
    libyaml-dev \
    libsasl2-dev \
    libldap2-dev \
    default-libmysqlclient-dev \
    sshpass \
    git

# Copy the provision.sh script to the container
COPY provision-docker.sh /usr/local/bin/

# Make the provision.sh script executable
RUN chmod +x /usr/local/bin/provision-docker.sh

# Run the provision.sh script when the container starts
RUN /usr/local/bin/provision-docker.sh

# Install some packages
RUN apt-get install -y procps htop net-tools iproute2

# Change entrypoint
ENTRYPOINT ["/opt/polemarch/bin/polemarchctl"]


ENV POLEMARCH_LOG_LEVEL=DEBUG

ENV WORKER=ENABLE 
ENV LC_ALL=en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV POLEMARCH_LOG_LEVEL=DEBUG
ENV POLEMARCH_DEBUG=true

ENV COLUMNS=80
ENV LINES=40

CMD ["dockerrun"]