
#include "keyboard_map.h"
#include "keyboard_driver.h"
#include "key_codes.h"
#include "stdint.h"

struct IDT_entry IDT[IDT_SIZE];
volatile char char_buffer = 0;
volatile uint8_t scan_code = 0;
volatile uint8_t key_status[256] = {0};

void idt_init()
{
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	/* populate IDT entry of keyboard's interrupt */
	keyboard_address = (unsigned long)keyboard_handler;
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
	IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = INTERRUPT_GATE;
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	/*     Ports
	*	 PIC1	PIC2
	*Command 0x20	0xA0
	*Data	 0x21	0xA1
	*/

	/* ICW1 - begin initialization */
	write_port(0x20, 0x11);
    	write_port(0xA0, 0x11);

	/* ICW2 - remap offset address of IDT */
	/*
	* In x86 protected mode, we have to remap the PICs beyond 0x20 because
	* Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
	*/
	write_port(0x21 , 0x20);
	write_port(0xA1 , 0x28);

	/* ICW3 - setup cascading */
	write_port(0x21 , 0x00);
	write_port(0xA1 , 0x00);

	/* ICW4 - environment info */
	write_port(0x21 , 0x01);
	write_port(0xA1 , 0x01);
	/* Initialization finished */

	/* mask interrupts */
	write_port(0x21 , 0xff);
	write_port(0xA1 , 0xff);

	/* fill the IDT descriptor */
	idt_address = (unsigned long)IDT;
    	idt_ptr[0] = (sizeof(struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
    	idt_ptr[1] = idt_address >> 16;

	load_idt(idt_ptr);
}

void kb_init()
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	write_port(0x21 , 0xFD);
}

uint8_t is_key_pressed(uint8_t scancode)
{
    if(scancode >= 128)
        return 0;
    return key_status[scancode];
}


char translate_char_to_char_WITH_SHIFT(char character);
void keyboard_handler_main()
{
    unsigned char status;
    uint16_t keycode;

    /* write EOI */
    write_port(0x20, 0x20);

    status = read_port(KEYBOARD_STATUS_PORT);
    if (status & 0x01) {
        keycode = read_port(KEYBOARD_DATA_PORT);

        uint8_t is_pressed = !(keycode & 0x80);
        uint8_t raw_keycode = keycode & 0x7F;

        key_status[raw_keycode] = is_pressed;

        if (is_pressed) {
		if (raw_keycode == ENTER_KEY_CODE) {
			char_buffer = '\n';
		}
		if (raw_keycode == BACKSPACE_KEY_CODE) {
			char_buffer = '\b';
		}
		if (is_key_pressed(SHIFT_KEY_CODE)) {
			char_buffer = translate_char_to_char_WITH_SHIFT(keyboard_map[raw_keycode]);
		} else {
			char_buffer = keyboard_map[raw_keycode];
		}	
		scan_code = raw_keycode;
            }
        }
}

char translate_char_to_char_WITH_SHIFT(char character)
{
	if(character >= 'a' && character <= 'z') {
		return character - 32;
	}
	switch(character) {
		case '1': return '!';
		case '2': return '@';
		case '3': return '#';
		case '4': return '$';
		case '5': return '%';
		case '6': return '^';
		case '7': return '&';
		case '8': return '*';
		case '9': return '(';
		case '0': return ')';
		case '-': return '_';
		case '=': return '+';
		case '[': return '{';
		case ']': return '}';
		case '\\': return '|';
		case ';': return ':';
		case '\'': return '"';
		case ',': return '<';
		case '.': return '>';
		case '/': return '?';
		case '`': return '~';
		default: break;
	}
}
