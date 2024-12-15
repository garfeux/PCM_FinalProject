# Use the x86 Ubuntu image for cross-compilation
FROM --platform=linux/amd64 ubuntu:22.04 AS builder

# Install essential build tools and dependencies
RUN apt update && apt install -y \
    build-essential \
    gcc g++ \
    libatomic1

# Set the working directory
WORKDIR /src

# Copy the source code into the container
COPY . .

# Clean and build the static binary
RUN make clean || true

# Set the working directory and entrypoint
WORKDIR /app

