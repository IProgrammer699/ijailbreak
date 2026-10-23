#include <stddef.h>
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t*)0xB8000)

static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = 0x0B; /* bright cyan on black */

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

static void clear_screen(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_MEMORY[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}

static void scroll_if_needed(void) {
    if (terminal_row < VGA_HEIGHT) return;

    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_MEMORY[(y - 1) * VGA_WIDTH + x] = VGA_MEMORY[y * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    }
    terminal_row = VGA_HEIGHT - 1;
}

static void put_char(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        scroll_if_needed();
        return;
    }

    if (c == '\r') {
        terminal_column = 0;
        return;
    }

    VGA_MEMORY[terminal_row * VGA_WIDTH + terminal_column] = vga_entry(c, terminal_color);
    terminal_column++;

    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
        scroll_if_needed();
    }
}

static void print(const char *s) {
    while (*s) put_char(*s++);
}

static void print_line(const char *s) {
    print(s);
    put_char('\n');
}

static void print_prompt(void) {
    terminal_color = 0x0A;
    print("A:\\>");
    terminal_color = 0x0F;
}

static int string_equal(const char *a, const char *b) {
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return 0;
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}

static int starts_with_echo(const char *s) {
    const char *prefix = "ECHO ";
    for (int i = 0; prefix[i]; i++) {
        char c = s[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        if (c != prefix[i]) return 0;
    }
    return 1;
}

/* US keyboard layout, basic unshifted keys */
static char scancode_to_ascii(uint8_t sc) {
    static const char map[128] = {
        0, 27, '1','2','3','4','5','6','7','8','9','0','-','=',
        '\b','\t','q','w','e','r','t','y','u','i','o','p','[',']',
        '\n', 0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
        0, '\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' '
    };
    if (sc < 128) return map[sc];
    return 0;
}

static char command[128];
static size_t command_len = 0;

static void execute_command(void) {
    command[command_len] = '\0';
    put_char('\n');

    if (command_len == 0) {
        /* nothing */
    } else if (string_equal(command, "HELP")) {
        print_line("NanoSoft DOS commands:");
        print_line("  HELP       Show this help");
        print_line("  VER        Show version");
        print_line("  CLS        Clear screen");
        print_line("  ECHO text  Print text");
        print_line("  ABOUT      About NanoSoft DOS");
        print_line("  REBOOT     Reboot the virtual machine");
    } else if (string_equal(command, "VER")) {
        print_line("NanoSoft DOS version 1.0 (experimental)");
    } else if (string_equal(command, "CLS")) {
        clear_screen();
    } else if (string_equal(command, "ABOUT")) {
        print_line("NanoSoft DOS");
        print_line("A tiny hobby operating system made for learning.");
        print_line("This is an early prototype, not MS-DOS.");
    } else if (starts_with_echo(command)) {
        print(command + 5);
        put_char('\n');
    } else if (string_equal(command, "REBOOT")) {
        print_line("Rebooting...");
        outb(0x64, 0xFE);
        for (;;) __asm__ volatile ("hlt");
    } else {
        print("Bad command or file name: ");
        print_line(command);
        print_line("Type HELP for available commands.");
    }

    command_len = 0;
    command[0] = '\0';
    print_prompt();
}

void kernel_main(void) {
    clear_screen();

    terminal_color = 0x0B;
    print_line("================================================");
    print_line("                 NanoSoft DOS");
    print_line("================================================");
    terminal_color = 0x07;
    print_line("NanoSoft DOS version 1.0");
    print_line("Experimental x86 hobby OS");
    print_line("Type HELP to see available commands.");
    put_char('\n');
    print_prompt();

    for (;;) {
        /* PS/2 keyboard controller: bit 0 means a key byte is ready. */
        if (inb(0x64) & 1) {
            uint8_t sc = inb(0x60);

            /* Ignore key-release scancodes. */
            if (sc & 0x80) continue;

            char c = scancode_to_ascii(sc);
            if (!c) continue;

            if (c == '\n') {
                execute_command();
            } else if (c == '\b') {
                if (command_len > 0) {
                    command_len--;
                    if (terminal_column > 0) terminal_column--;
                    VGA_MEMORY[terminal_row * VGA_WIDTH + terminal_column] =
                        vga_entry(' ', terminal_color);
                }
            } else if (c >= 32 && c <= 126 && command_len < sizeof(command) - 1) {
                command[command_len++] = c;
                put_char(c);
            }
        }
    }
}
