#!/usr/bin/env bash
./build_android.sh &&
  fastboot erase boot &&
  fastboot flash boot boot.img &&
  fastboot erase system &&
  fastboot flash system out/target/product/achroimx/system.img &&
  fastboot reboot &&
  adb logcat | tee ANDROID_LOG.txt | grep TAEGUK
