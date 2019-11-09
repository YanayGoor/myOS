#include "screen.h"
#include "ports.h"
#include "../kernel/utils.h"


// define private methods
void kprint_char(char c);
void set_offset(int offset);
int get_offset();
int row_from_offset(int offset);
int col_from_offset(int offset);
int offset_from_coords(int col, int row);

void clear_screen() {
    int offset;
    for (offset = 0; offset < MAX_COLS*MAX_ROWS*2; offset+=2) {
        ((char *)VIDEO_ADDRESS)[offset] = ' ';
        ((char *)VIDEO_ADDRESS)[offset + 1] = WHITE_ON_BLACK;

    }
    set_offset(0);
}

void kprint_at(char *message, int col, int row) {
    if (col >= 0 && row >= 0) {
        set_offset(offset_from_coords(col, row));
    }

    int message_offset = 0;
    while (message[message_offset] != 0) {
        kprint_char(message[message_offset]);
        message_offset++;
    }
}

void kprint(char *message) {
    kprint_at(message, -1, -1);
}


void kprint_char(char c) {
    int offset = get_offset();

    if (c == '\n') {
        int row = row_from_offset(offset);
        offset = offset_from_coords(0, row + 1);
    } else {
        ((char *)VIDEO_ADDRESS)[offset] = c;
        offset += 2;
    }
    if (offset >= MAX_ROWS * MAX_COLS * 2) {
        //scroll down console
        int last_line_offset = (MAX_ROWS - 1) * MAX_COLS * 2;
        memory_copy((char *)(VIDEO_ADDRESS+MAX_COLS * 2), (char *)(VIDEO_ADDRESS), last_line_offset);
        int i;
        for (i = 0; i < MAX_COLS; i++) {
            ((char *)VIDEO_ADDRESS)[last_line_offset + i * 2] = ' ';
        }
        set_offset(last_line_offset);
    } else {
        set_offset(offset);
    }
}

int offset_from_coords(int col, int row) {
    return (row * MAX_COLS + col) * 2;
}

int row_from_offset(int offset) {
    return offset / (MAX_COLS * 2);
}

int col_from_offset(int offset) {
    return (offset - row_from_offset(offset)) / 2;
}

void set_offset(int offset) {
    offset /= 2;
    unsigned char higher = (unsigned char)(offset >> 8);
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, higher);

    unsigned char lower = (unsigned char)(0xff & offset);
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, lower);
}

int get_offset() {
    port_byte_out(0x3d4, 14);
    int position = port_byte_in(0x3d5);
    position = position << 8; /* high byte */

    port_byte_out(0x3d4, 15);
    position += port_byte_in(0x3d5);

    int offset_from_vga = position * 2;
    return offset_from_vga;
}



