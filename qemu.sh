#!/bin/sh

qemu-system-x86_64 -s -S -cdrom image.iso \
 -k de -no-reboot\
 -monitor stdio \
 -smp 2 -machine type=q35 \
 -device qemu-xhci -device ahci,id=ahci -m 4G \
 -enable-kvm -cpu host #\
 #-D qemu-memory-tlb.log \
 #-drive file=drive.qcow2,if=none,id=disk0,format=qcow2 -device ide-hd,drive=disk0,bus=ahci.0
 # -d int,cpu_reset