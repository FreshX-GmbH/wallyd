#!/bin/sh
echo Sending ON
echo "on 0" | cec-client -s -d 1
echo Switching to HDMI
echo "as" | cec-client -s -d 1
