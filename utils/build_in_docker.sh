#!/bin/bash
set -e

IMG_NAME=gcc

# Clean start of containers
docker-compose stop $IMG_NAME
docker-compose rm -f $IMG_NAME
docker-compose build $IMG_NAME
docker-compose up $IMG_NAME
