#!/bin/bash


sudo rmmod tc_pcie_driver
# make clean ; make ; 
sudo insmod tc_pcie_driver.ko
dmesg | grep tc_


#if [ "$1"=="rm" ]; then
#  echo rmmod tc_pcie_driver
#else if [ "$1"="mk" ]; then
#  make clean ; make ; insmod tc_pcie_driver
#else if [ "$1"="ins" ]; then
#  insmod tc_pcie_driver
#else if [ "$1"="msg" ]; then
#  dmesg -c | grep -c tc_
#else
#  echo "mk, ins, rm, msg"
#fi
#fi
#fi
#fi

