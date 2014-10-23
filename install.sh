#!/bin/bash

adb shell mkdir -p /data/at/config/
adb shell mkdir -p /data/at/testcases/

adb push config /data/at/config
adb push testcases /data/at/testcases/

