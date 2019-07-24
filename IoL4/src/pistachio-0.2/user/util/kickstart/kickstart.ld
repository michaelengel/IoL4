SECTIONS
{
        .text : {
        *(.mbh)
        *(.text)
        *(.*data*)
        _edata = .;
        }
        _end = .;
}
