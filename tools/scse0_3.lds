OUTPUT_ARCH(mips)
/*
Set the architecture to mips.
*/
ENTRY(_start)
/*
Set the ENTRY point of the program to _start.
*/
SECTIONS
{
	. = 0x080010000;
	.text : { *(.text) }
	.data : { *(.data) }
	.bss : { *(.bss) }
	. = 0x80400000;
	

/*To do:
  fill in the correct address of the key section
  such as text data bss ...
*/


end = . ;
}
