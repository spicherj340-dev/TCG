# --- STAGE 1: Build the C++ Executable (The FIX is the COPY command) ---
FROM ubuntu:22.04 AS builder

# Install g++ and necessary tools
RUN apt update && apt install -y g++

# Set working directory for compilation
WORKDIR /usr/src/app

# CORRECTED LINE: Copy ALL files from the repo root to the build directory.
# This ensures httplib.h and any other required headers are present.
COPY . .

# Compile C++ program
RUN g++ main.cpp -o app -std=c++17 -O2

# --- STAGE 2: Create the Final Runtime Image with NGINX ---
FROM nginx:stable-alpine AS final

# 1. Copy static files (HTML, CSS, frontback.jpg) to NGINX's web root. (Fixes 404)
COPY . /usr/share/nginx/html

# 2. Copy the compiled C++ app from the builder stage
COPY --from=builder /usr/src/app/app /usr/bin/

# 3. Add the NGINX configuration file (REQUIRED for NGINX to run your C++ app)
# You MUST have committed the nginx.conf file to your repo!
COPY nginx.conf /etc/nginx/conf.d/default.conf

EXPOSE 80 
CMD ["nginx", "-g", "daemon off;"]
