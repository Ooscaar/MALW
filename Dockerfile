# Use the official Ubuntu image as a base
FROM ubuntu:20.04

# Copy the provision.sh script to the container
COPY provision-docker.sh /usr/local/bin/

# Make the provision.sh script executable
RUN chmod +x /usr/local/bin/provision-docker.sh

# Run the provision.sh script when the container starts
RUN /usr/local/bin/provision-docker.sh

# Change entrypoint
ENTRYPOINT ["/opt/polemarch/bin/polemarchctl"]

# Run the web service on container startup
CMD ["dockerrun"]