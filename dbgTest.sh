/d/Projects/IgelKernel/iso.sh
objcopy --only-keep-debug /d/Projects/IgelKernel/kernel/bin/myos /d/Projects/IgelKernel/kernel/bin/kernel.sym
./qemu.sh

sleep 5