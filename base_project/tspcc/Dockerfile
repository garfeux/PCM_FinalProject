# Use the x86 Ubuntu image for cross-compilation
FROM --platform=linux/amd64 ubuntu:22.04

# Install essential build tools and dependencies
# Add --no-install-recommends to avoid installing unnecessary packages

RUN apt-key adv --fetch-keys https://keyserver.ubuntu.com/pks/lookup?op=get&search=0x3b4fe6acc0b21f32

RUN apt-get clean  \
RUN apt-get update  \
RUN apt-get install -y\
    build-essential \
    gcc g++ \
    libatomic1 \
    gnupg2 \
    ca-certificates && \
    rm -rf /var/lib/apt/lists/*

# Update GPG keys for package repositories

# Set the working directory for the build
WORKDIR /src

# Copy the source code into the container
COPY . .

# Clean the previous build (if applicable) and build the static binary
#RUN make TARGET=Linux

# Set the working directory and entrypoint
#WORKDIR /app

# Optionally copy the built binary to a new directory for runtime
# COPY /src/build/your_binary_name /app/
