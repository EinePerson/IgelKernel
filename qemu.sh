#!/bin/sh

qemu-system-x86_64 -s -S -cdrom image.iso -d int,cpu_reset -monitor stdio -no-reboot
