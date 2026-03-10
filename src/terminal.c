#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include "terminal.h"

#define STDIN_FILENO 0
#define STDOUT_FILENO 1

#define ARROW_UP 1000
#define ARROW_DOWN 1001
#define ARROW_LEFT 1002
#define ARROW_RIGHT 1003
#define KEY_DELETE 1004

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

int read_key(void) {
    char c;
    read(STDIN_FILENO, &c, 1);

    if (c == '\x1b') {
        // set non-blocking
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) {
            fcntl(STDIN_FILENO, F_SETFL, 0); // restore blocking
            return '\x1b';
        }
        if (read(STDIN_FILENO, &seq[1], 1) != 1) {
            fcntl(STDIN_FILENO, F_SETFL, 0);
            return '\x1b';
        }

        fcntl(STDIN_FILENO, F_SETFL, 0); // restore blocking

        if (seq[0] == '[') {
            if (seq[1] == '3') {
                char tilde;
                read(STDIN_FILENO, &tilde, 1);
                if (tilde == '~') return KEY_DELETE;
            }
            switch (seq[1]) {
                case 'A': return ARROW_UP;
                case 'B': return ARROW_DOWN;
                case 'C': return ARROW_RIGHT;
                case 'D': return ARROW_LEFT;
            }
        }
        return '\x1b';
    }

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
