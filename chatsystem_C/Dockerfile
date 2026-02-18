FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    iproute2 \
    iputils-ping \
    netcat \
    tcpdump \
    sudo \
    && rm -rf /var/lib/apt/lists/*

ARG UID=1000
ARG GID=1000
RUN groupadd -g $GID ubuntu && \
    useradd -m -u $UID -g $GID -s /bin/bash ubuntu && \
    passwd -d ubuntu && usermod -L ubuntu && \
    adduser ubuntu sudo && \
    echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
USER ubuntu
WORKDIR /home/ubuntu/share
