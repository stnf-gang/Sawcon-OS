ENTRY(main)
OUTPUT_FORMAT(binary)
__START_ADDR__ = 0x5000;

SECTIONS {
    . = __START_ADDR__;
    __TEXT_START_ = .;
    .text : {*(.text)}
    __TEXT_END = .;

    __DATA_START__ = .;
    data : {*(.data)}
    __DATA_END__ = .;

    __END_ADDR__ = .;
}
