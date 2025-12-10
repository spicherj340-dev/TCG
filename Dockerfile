# --- STAGE 1: Build the C++ Executable ---
FROM ubuntu:22.04 AS builder
RUN apt update && apt install -y g++
WORKDIR /usr/src/app
COPY main.cpp .
RUN g++ main.cpp -o app -std=c++17 -O2

# --- STAGE 2: Create the Final Runtime Image with NGINX ---
FROM nginx:stable-alpine AS final

# 1. Copy static files (HTML, CSS, frontback.jpg) to NGINX's web root. This fixes the 404.
COPY . /usr/share/nginx/html

# 2. Copy the compiled C++ app from the builder stage
COPY --from=builder /usr/src/app/app /usr/bin/

# 3. Add the NGINX configuration file (REQUIRED to forward traffic to C++ app)
COPY nginx.conf /etc/nginx/conf.d/default.conf

EXPOSE 80 
CMD ["nginx", "-g", "daemon off;"]
