#!/bin/sh

qemu-system-x86_64 -s -S -cdrom image.iso -d int,cpu_reset -monitor stdio -cpu host -enable-kvm -smp 2 -machine type=q35 -device qemu-xhci
