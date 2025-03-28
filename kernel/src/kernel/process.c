//
// Created by igel on 11.03.25.
//

#include "kernel/process.h"

static uint64_t highestPID;

struct process spawn_process(uint64_t flags) {
    struct process process;
    process.pid = highestPID;
    highestPID++;
    process.flags = flags;
    process.superviser = false;

}