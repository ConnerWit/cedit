#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "terminal.h"

#define STDIN_FILENO 0
#define STDOUT_FILENO 1

static struct termios orig_termios;

static void save_terminal_state() {
    tcgetattr(STDIN_FILENO, &orig_termios);
}

void terminal_cleanup(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void terminal_init(void) {
    save_terminal_state();
    atexit(terminal_cleanup);

    struct termios raw = orig_termios;

    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);

    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void clear_terminal(void) {
    write(STDOUT_FILENO, "\x1b[H\x1b[2J", 7);
}

char read_key(void) {
    char c;
    read(STDIN_FILENO, &c, 1);
    return c;
}

WindowSize get_window(void) {
    struct winsize ws;
    WindowSize size = {24, 80};

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1) {
        size.rows = ws.ws_row;
        size.cols = ws.ws_col;
    }

    return size;
}
