# Use Ubuntu as the base image
FROM ubuntu:22.04

# Install g++ (compiler)
RUN apt update && apt install -y g++

# Set working directory
WORKDIR /app

# Copy source files
COPY . .

# Compile C++ program directly
RUN g++ main.cpp -o app -std=c++17 -O2

# Expose server port
EXPOSE 8080

# Run the compiled app
CMD ["./app"]
