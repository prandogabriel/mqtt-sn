#!/bin/bash

git clone --recursive https://github.com/eclipse/mosquitto.rsmb.git

cp build.sh mosquitto.rsmb
cp config.conf mosquitto.rsmb/rsmb/src

cd mosquitto.rsmb 

docker run --rm -v "$PWD":/usr/src/app -w /usr/src/app gcc:9 sh build.sh

cd rsmb/src

./broker_mqtts config.conf