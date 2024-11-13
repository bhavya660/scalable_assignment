# Dockerfile for User Service

# Use a base image with Ubuntu
FROM ubuntu:latest

# Set environment variables to avoid interactive prompts during the installation process
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies (g++, FastCGI, SQLite3, etc.)
RUN apt-get update && \
    apt-get install -y \
    systemd \
    g++ \
    curl \
    vim \
    make \
    lighttpd \
    libfcgi-dev \
    libsqlite3-dev \
    libjsoncpp-dev \
    libssl-dev \
    libcrypt-dev \
    && apt-get clean

# Enable lighttpd service (you will start it via systemd later)
#RUN systemctl enable lighttpd.service

# Create a working directory in the container
WORKDIR /app

# Copy the source code into the container
COPY . .
COPY lighttpd.conf /etc/lighttpd/lighttpd.conf

# Build the application using the Makefile
RUN make

RUN chown -R www-data:www-data /app
RUN chmod -R 755 /app

# Expose the port the FastCGI server will be running on (e.g., 9000)
EXPOSE 9000

# Command to run the FastCGI application
#CMD ["./user_service"]
CMD ["/lib/systemd/systemd"]

# This will allow the container to run interactively and process incoming FastCGI requests

