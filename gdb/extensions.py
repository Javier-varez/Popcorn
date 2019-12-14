from gdb.printing import PrettyPrinter, register_pretty_printer
import gdb

class TCBPrettyPrinter(object):
    """Print 'task_control_block'"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return ("struct task_control_block:\n"
                "\tName:\t\t%s\n"
                "\tFunction:\t%s\n"
                "\tPriority:\t%s\n"
                "\tState:\t\t%s\n"
                "\tStack ptr:\t0x%08x\n"
                "\tArgument ptr:\t0x%08x\n" %
                (self.val["name"].string(),
                 self.val["func"],
                 self.val["priority"],
                 self.val["state"],
                 self.val["stack_ptr"],
                 self.val["arg"]))

class TaskEventPrettyPrinter(object):
    """Print 'task_event'"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return ("struct task_event:\n"
                "\twakeup_timestamp:\t\t%d\n"
                "\ttask_ptr:\t\t\t0x%08x" %
                (self.val["requested_wakeup_timestamp"],
                 self.val["task"]))

class LinkedListParser(gdb.Command):
    """Parse a LinkedList"""

    def __init__(self):
        super(LinkedListParser, self).__init__(
            "ParseLinkedList", gdb.COMMAND_USER
        )

    def FindListOffset(self, type):
        for field in type.fields():
            if str(field.type) == "LinkedList_t":
                return int(field.bitpos / 8)
        return 0

    def complete(self, text, word):
        return gdb.COMPLETE_SYMBOL

    def invoke(self, args, from_tty):
        splitargs = args.split(",")

        list_ptr_val = gdb.parse_and_eval(splitargs[0])
        if str(list_ptr_val.type) != "LinkedList_t *":
            print("Expected argument of type (LinkedList_t *)")
            return

        element_type = gdb.lookup_type(splitargs[1].strip())
        list_offset = self.FindListOffset(element_type)
        self.ParseList(list_ptr_val, element_type, list_offset)

    def ParseList(self, head, type, offset):
        ll_p = head
        while ll_p != 0:
            element = ll_p.cast(gdb.lookup_type("uint8_t").pointer()) - offset
            print("Found element at address 0x%08x, printing:" % element)
            gdb.execute("p *(%s)0x%x" % (type.pointer(), element))
            ll_p = ll_p.dereference()["next"]

class CustomPrettyPrinterLocator(PrettyPrinter):
    """Given a gdb.Value, search for a custom pretty printer"""

    def __init__(self):
        super(CustomPrettyPrinterLocator, self).__init__(
            "Cortex-M Scheduler Pretty Printers", []
        )

    def __call__(self, val):
        """Return the custom formatter if the type can be handled"""

        typename = gdb.types.get_basic_type(val.type).tag
        if typename is None:
            typename = val.type.name

        if typename == "task_control_block":
            return TCBPrettyPrinter(val)
        elif typename == "task_event":
            return TaskEventPrettyPrinter(val)

if __name__ == "__main__":
    register_pretty_printer(None, CustomPrettyPrinterLocator(), replace=True)
    LinkedListParser()