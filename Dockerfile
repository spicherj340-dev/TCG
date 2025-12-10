# Use Ubuntu as the base image
FROM ubuntu:22.04
RUN apt update && apt install -y g++
# Install g++ (compiler)

# Set working directory
WORKDIR /app

# Copy source files
COPY . .

# Compile C++ program directly
RUN g++ -std=c++17 -O2 -Wall -Wextra $(find . -name "*.cpp") -o app -lm

# Expose server port
EXPOSE 8080

# Run the compiled app
CMD ["./app"]
