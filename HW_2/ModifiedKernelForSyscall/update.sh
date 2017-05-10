#!/usr/bin/env bash

KERNEL_ROOT=/work/achroimx_kernel
TARGET_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/achroimx_kernel

pushd $KERNEL_ROOT > /dev/null
cp --parents arch/arm/include/asm/unistd.h $TARGET_DIR
cp --parents arch/arm/kernel/calls.S $TARGET_DIR
cp --parents include/linux/syscalls.h $TARGET_DIR
cp --parents kernel/homework2.c $TARGET_DIR
cp --parents kernel/Makefile $TARGET_DIR
popd > /dev/null
