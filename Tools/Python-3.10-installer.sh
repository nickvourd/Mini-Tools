# This script facilitates the installation of Python 3.10 on Linux systems.
# Created with <3 by @nickvourd
# Dependencies: None

#!/usr/bin/env bash

# Check if the user is root
if [ "$(id -u)" -ne 0 ]; then
    echo -e "[!] You need root privileges to run this script! Exiting...\n"
    exit 1
fi

echo -e "[~] Updating the system...\n"
apt update -y && apt upgrade -y && apt dist-upgrade -y


echo -e "[~] Installing the required dependencies to build Python 3.10 from the source...\n"
apt install build-essential zlib1g-dev libncurses5-dev libgdbm-dev libnss3-dev libssl-dev libreadline-dev libffi-dev libsqlite3-dev wget libbz2-dev -y

echo -e "[~] Downloading the latest version of Python 3.10 from the Python official release page...\n"
wget https://www.python.org/ftp/python/3.10.12/Python-3.10.12.tar.xz

echo -e "[~] Extracting the data...\n"
tar -xvf Python-3.10.12.tar.xz

cd Python-3.10.12
echo -e "[~] Running the configure script to check the required dependencies...\n"
./configure --enable-optimizations 

echo -e "[~] Initiating the Python 3.10 build process...\n"
make -j $(nproc)

echo -e "[~] Finalizing the installation of Python 3.10...\n"
make altinstall
