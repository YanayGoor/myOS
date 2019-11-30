void memory_copy(char *source, char *dest, int nbytes) {
    int i;
    for (i=0; i<nbytes; i++) {
        dest[i] = source[i];
    }
}

void memory_set(char *source, char byte, int nbytes) {
    int i;
    for (i=0; i<nbytes; i++) {
        source[i] = byte;
    }
}

int strlen(char str[]) {
    int i = 0;
    while (str[i] != 0) {
        i++;
    }
    return i;
}

void reverse_string(char str[]) {
    int length = strlen(str);
    char temp[length];
    int i;
    for (i = 0; i < length; i++) {
        temp[i] = str[length-i-1];
    }
    for (i = 0; i < length; i++) {
        str[i] = temp[i];
    }
}


void int_to_ascii(int n, char str[]) {
    int i = 0;
    if (n < 0) {
        str[i++] = '-';
        n = -n;
    }
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    str[i] = '\0';
    reverse_string(str);
}

void print_int(int i) {
    char str[255];
    int_to_ascii(i, str);
    kprint(str);
    kprint("\n");
}

void uint_to_ascii(unsigned int n, char str[]) {
    int i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    str[i] = '\0';
    reverse_string(str);
}

void print_uint(unsigned int i) {
    char str[255];
    uint_to_ascii(i, str);
    kprint(str);
    kprint("\n");
}

void print_uint_inline(unsigned int i) {
    char str[255];
    uint_to_ascii(i, str);
    kprint(str);
}