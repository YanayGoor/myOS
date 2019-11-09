#include "../../cpu/isr.h"
#include "../ports.h"
#include "../screen.h"
#include "keyboard.h"

void print_letter(u8 scancode) {
    switch (scancode) {
        case 0x0:
            kprint("ERROR");
            break;
        case 0x76:
            kprint("ESC");
            break;
        case 0x16:
            kprint("1");
            break;
        case 0x1e:
            kprint("2");
            break;
        case 0x26:
            kprint("3");
            break;
        case 0x25:
            kprint("4");
            break;
        case 0x2e:
            kprint("5");
            break;
        case 0x36:
            kprint("6");
            break;
        case 0x3d:
            kprint("7");
            break;
        case 0x3e:
            kprint("8");
            break;
        case 0x46:
            kprint("9");
            break;
        case 0x45:
            kprint("0");
            break;
        case 0x4e:
            kprint("-");
            break;
        case 0x79:
            kprint("+");
            break;
        case 0x66:
            kprint("Backspace");
            break;
        case 0x0D:
            kprint("Tab");
            break;
        case 0x15:
            kprint("Q");
            break;
        case 0x1D:
            kprint("W");
            break;
        case 0x24:
            kprint("E");
            break;
        case 0x2d:
            kprint("R");
            break;
        case 0x2c:
            kprint("T");
            break;
        case 0x35:
            kprint("Y");
            break;
        case 0x3c:
            kprint("U");
            break;
        case 0x43:
            kprint("I");
            break;
        case 0x44:
            kprint("O");
            break;
        case 0x4d:
            kprint("P");
            break;
		case 0x54:
			kprint("[");
			break;
		case 0x5b:
			kprint("]");
			break;
		case 0x5a:
			kprint("ENTER");
			break;
		case 0x14:
			kprint("LCtrl");
			break;
		case 0x1c:
			kprint("A");
			break;
		case 0x1b:
			kprint("S");
			break;
        case 0x23:
            kprint("D");
            break;
        case 0x2b:
            kprint("F");
            break;
        case 0x34:
            kprint("G");
            break;
        case 0x33:
            kprint("H");
            break;
        case 0x3b:
            kprint("J");
            break;
        case 0x42:
            kprint("K");
            break;
        case 0x4b:
            kprint("L");
            break;
        case 0x4c:
            kprint(";");
            break;
        case 0x52:
            kprint("'");
            break;
        case 0x0e:
            kprint("`");
            break;
		case 0x12:
			kprint("LShift");
			break;
		case 0x5d:
			kprint("\\");
			break;
		case 0x1a:
			kprint("Z");
			break;
		case 0x22:
			kprint("X");
			break;
		case 0x21:
			kprint("C");
			break;
		case 0x2a:
			kprint("V");
			break;
        case 0x32:
            kprint("B");
            break;
        case 0x31:
            kprint("N");
            break;
        case 0x3a:
            kprint("M");
            break;
        case 0x41:
            kprint(",");
            break;
        case 0x49:
            kprint(".");
            break;
        case 0x4a:
            kprint("/");
            break;
        case 0x59:
            kprint("Rshift");
            break;
        case 0x7c:
            kprint("Keypad *");
            break;
        case 0x11:
            kprint("LAlt");
            break;
        case 0x29:
            kprint(" ");
            break;
        case 0xe0:
            break;
        default:
            /* 'keuyp' event corresponds to the 'keydown' + 0x80
             * it may still be a scancode we haven't implemented yet, or
             * maybe a control/escape sequence */
            break;
    }
}

int up = 0;

void keyboard_handler(registers_t r) {
/* The PIC leaves us the scancode in port 0x60 */
    u8 scancode = port_byte_in(0x60);
    char *sc_ascii;
    int_to_ascii(scancode, sc_ascii);
    if (scancode == 0xf0) {
        up = 1;
    } else if (up == 0) {
        print_letter(scancode);
        up = 0;
    } else {
        up = 0;
    }
//    kprint("Keyboard scancode: ");
//    kprint(sc_ascii);
//    kprint(", ");
//    kprint("\n");
}

int wait_for_input() {
    int i;
    int ready = 0;
    for (i = 0; i < 1000; i++) {
        if (!PS2_STATUS_INPUT(port_byte_in(PS2_STATUS))) {
            ready = 1;
            break;
        }
    }
    return ready;
}

int wait_for_output() {
    int i;
    int ready = 0;
    for (i = 0; i < 1000; i++) {
        if (!PS2_STATUS_OUTPUT(port_byte_in(PS2_STATUS))) {
            ready = 1;
            break;
        }
    }
    return ready;
}

void init_keyboard() {
    __asm__ __volatile__("cli");
    port_byte_out(PS2_CMD, 0x60);
    if (wait_for_output()) {
        port_byte_out(PS2_DATA, 0x05);
        register_interrupt_handler(PS2_IRQ, keyboard_handler);
        if (wait_for_input()) {
            port_byte_out(PS2_DATA, 0xFF);
//            if (wait_for_input()) port_byte_out(PS2_DATA, 0x60);
        }
    }

    __asm__ __volatile__("sti");
}