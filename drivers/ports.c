unsigned char port_byte_in (unsigned short port) {
    unsigned char result;
    __asm__ ("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void port_byte_out (unsigned short port, unsigned char data) {
    __asm__ ("out %%al, %%dx" : : "d" (port), "a" (data));
}

unsigned short port_word_in (unsigned short port) {
    unsigned char result;
    __asm__ ("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void port_word_out (unsigned short port, unsigned short data) {
    __asm__ ("out %%ax, %%dx" : : "d" (port), "a" (data));
}

unsigned int port_dword_in (unsigned int port) {
    unsigned int result;
    __asm__ ("inl %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void port_dword_out (unsigned int port, unsigned int data) {
    __asm__ ("outl %%ax, %%dx" : : "d" (port), "a" (data));
}