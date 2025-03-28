# Define a Python class to convert software breakpoints to hardware
python
import gdb

gdb_connected = False
bps = []
#del_bps = []

class BreakpointHandler(gdb.Command):

    def __init__(self):
        super(BreakpointHandler, self).__init__("handle-breakpoints", gdb.COMMAND_USER)
        # Connect the breakpoint event to the test function
        gdb.events.breakpoint_created.connect(self.on_breakpoint_set)
        #gdb.events.breakpoint_deleted.connect(self.on_breakpoint_delete)

    def on_breakpoint_set(self, bp):
        if gdb_connected:
            switch_break(bp)
        else:
            bps.append(bp)


    def on_breakpoint_delete(self, bp):
        print("Breakpoint deleted:", bp)
        if gdb_connected:
            switch_break(bp)
        else:
            bps.append(bp)



def switch_break(bp):
    try:
        if bp.type == gdb.BP_BREAKPOINT:
            # Convert software breakpoint to hardware
            gdb.execute(f"hbreak {bp.location}", from_tty=False)
            gdb.execute(f"del {bp.number}", from_tty=False)
    except RuntimeError as e:
        print(f"Err {e}")
        #pass


def delete_break(bp):
    try:
        if bp.type == gdb.BP_BREAKPOINT:
            # Convert software breakpoint to hardware
            gdb.execute(f"hbreak {bp.location}", from_tty=False)
            gdb.execute(f"del {bp.number}", from_tty=False)
    except RuntimeError as e:
        print(f"Err {e}")
        #pass


def on_continue(event):
    global gdb_connected
    gdb_connected = True
    for bp in bps:
        switch_break(bp)
    #for bp in del_bps:
        #delete_break(bp)

# Register the custom command
#ConvertBreakpointsCommand()
gdb.events.cont.connect(on_continue)
BreakpointHandler()

# Optionally, you can run the conversion immediately if there are existing breakpoints
# ConvertBreakpointsCommand().convert_breakpoints()
end
