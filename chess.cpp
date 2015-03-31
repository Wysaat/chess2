/*
 *
 * Assume the program always takes Black
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <list>
#include <array>
#include <map>

#pragma GCC diagnostic ignored "-Wwrite-strings"

#define INF 2147483647

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))

#define LOG(x) syslog(LOG_INFO, "%s", x)

enum PieceType {
    pnil = 0,
    wpawn = 1,
    wrook = 2,
    wknight = 3,
    wbishop = 4,
    wqueen = 5,
    wking = 6,
    bpawn = 7,
    brook = 8,
    bknight = 9,
    bbishop = 10,
    bqueen = 11,
    bking = 12,
};

typedef std::array<std::array<enum PieceType, 8>, 8> BOARD;

int notation_dec(char *notation, int *decoded);
void change_board(int ox, int oy, int nx, int ny);
std::list<BOARD> next_boards(BOARD board, int color);
int minimax(BOARD board, int depth, int maximize_player, int color);
void get_move(BOARD board, BOARD new_board, int *move);
char *move_to_str(int *move);

void flog(char *X, ...) {
    va_list args;
    va_start(args, X);
    FILE *f = fopen("log", "a");
    vfprintf(f, X, args);
    va_end(args);
    fclose(f);
};

int is_white(enum PieceType type) {
    if (type == pnil)
        return 0;
    else if (type <= 6)
        return 1;
    return 0;
}

int is_black(enum PieceType type) {
    if (type == pnil)
        return 0;
    else if (type >= 7)
        return 1;
    return 0;
}

std::array<std::array<enum PieceType, 8>, 8> board = {{
    { wrook, wknight, wbishop, wqueen, wking, wbishop, wknight, wrook, },
    { wpawn, wpawn, wpawn, wpawn, wpawn, wpawn, wpawn, wpawn, },
    { },
    { },
    { },
    { },
    { bpawn, bpawn, bpawn, bpawn, bpawn, bpawn, bpawn, bpawn, },
    { brook, bknight, bbishop, bqueen, bking, bbishop, bknight, brook, },
}};

char *board_to_str(BOARD board) {
    char *retptr = (char *)malloc(sizeof(char)*66);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            retptr[i*8+j] = board[i][j] + '0';
        }
    }
    retptr[64] = '\n';
    retptr[65] = 0;
    return retptr;
}

int main()
{
    system("rm -f log");
    openlog("CHESS-ENGINE", LOG_PID, LOG_USER);
    setbuf(stdout, 0);
    signal(SIGINT, SIG_IGN);
    char *line = 0;
    size_t n;
    getline(&line, &n, stdin);  // should be "xboard"
    printf("\n");
    flog(line);
    getline(&line, &n, stdin);  // "protover 2"
    printf("\n");
    flog(line);
    printf("feature done=0\n");
    getline(&line, &n, stdin);
    flog(line);
    printf("feature sigint=0\n");
    getline(&line, &n, stdin);
    flog(line);
    printf("feature colors=0\n");
    getline(&line, &n, stdin);
    flog(line);
    printf("feature done=1\n");
    getline(&line, &n, stdin);
    flog(line);

    getline(&line, &n, stdin);  // "new"
    flog(line);
    getline(&line, &n, stdin);  // "random"
    flog(line);
    getline(&line, &n, stdin);  // "level 40 5 0" (time control)
    flog(line);
    getline(&line, &n, stdin);  // "post"
    flog(line);
    getline(&line, &n, stdin);  // "hard"
    flog(line);

    int i = 0;
    while (1) {
        getline(&line, &n, stdin);
        int decoded[4];
        if (!strncmp(line, "time", 4) || !strncmp(line, "otim", 4))
            ;// flog(line);
        else if (!strcmp(line, "force\n"))
            flog(line);
        else if (!strcmp(line, "computer\n"))
            flog(line);
        else if (notation_dec(line, decoded)) {
            flog(line);
            exit(-1);
        }
        else {
            flog(line);
            int decoded[4];
            notation_dec(line, decoded);
            change_board(decoded[0], decoded[1], decoded[2], decoded[3]);
            BOARD next_board;
            int val = -INF;
            std::list<BOARD> boards = next_boards(board, 1);
            flog("**************************************************\n");
            for (BOARD new_board : boards) {
                int new_val = minimax(new_board, 3, 0, 0);
                flog("evaluation is %06d, %s", new_val, board_to_str(new_board));
                for (BOARD b : next_boards(new_board, 0))
                    flog("    %s", board_to_str(b));
                if (new_val > val) {
                    next_board = new_board;
                    val = new_val;
                }
            }
            flog("..................................................\n");
            int move[4];
            get_move(board, next_board, move);
            printf("move ");
            printf("%s\n", move_to_str(move));
            board = next_board;
        }
    }

    closelog();
    return 0;
}

char *move_to_str(int *move) {
    char *retptr = (char *)malloc(sizeof(char)*5);
    retptr[0] = move[1] + 'a';
    retptr[1] = move[0] + '1';
    retptr[2] = move[3] + 'a';
    retptr[3] = move[2] + '1';
    retptr[4] = 0;
    return retptr;
}

void get_move(BOARD board, BOARD new_board, int *move) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] != new_board[i][j]) {
                if (new_board[i][j] == pnil) {
                    move[0] = i;
                    move[1] = j;
                }
                else {
                    move[2] = i;
                    move[3] = j;
                }
            }
        }
    }
}

int notation_dec(char *notation, int *decoded) {
    if (notation[0]<'a' || notation[0]>'h')
        return -1;
    if (notation[1]<'1' || notation[1]>'8')
        return -1;
    if (notation[2]<'a' || notation[2]>'h')
        return -1;
    if (notation[3]<'1' || notation[3]>'8')
        return -1;
    decoded[0] = notation[1] - '1';
    decoded[1] = notation[0] - 'a';
    decoded[2] = notation[3] - '1';
    decoded[3] = notation[2] - 'a';
    return 0;
}

void change_board(int ox, int oy, int nx, int ny) {
    enum PieceType p = board[ox][oy];
    board[ox][oy] = pnil;
    board[nx][ny] = p;
}

int evaluate(BOARD board) {
    int i, j, value = 0;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            switch (board[i][j]) {
                case bpawn: value += 100; break;
                case bknight: value += 320; break;
                case bbishop: value += 330; break;
                case brook: value += 500; break;
                case bqueen: value += 900; break;
                case bking: value += 40000; break;
                case wpawn: value -= 100; break;
                case wknight: value -= 320; break;
                case wbishop: value -= 330; break;
                case wrook: value -= 500; break;
                case wqueen: value -= 900; break;
                case wking: value -= 20000; break;
            }
        }
    }
    return value;
}

int game_over(BOARD board) {
    int kings = 0;
    int i, j;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            if (board[i][j] == wking || board[i][j] == bking)
                kings++;
        }
    }
    if (kings == 2)
        return 0;
    return 1;
}

std::list<BOARD> next_boards(BOARD board, int color) {
    enum PieceType pawn, rook, knight, bishop, queen, king;
    enum PieceType op_pawn, op_rook, op_knight, op_bishop, op_queen, op_king;
    int forward;
    int (*is_same_color)(enum PieceType);
    int (*is_opposite_color)(enum PieceType);
    if (color == 0) {
        pawn = wpawn, op_pawn = bpawn;
        rook = wrook, op_rook = brook;
        knight = wknight, op_knight = bknight;
        bishop = wbishop, op_bishop = bbishop;
        queen = wqueen, op_queen = bqueen;
        king = wking, op_king = bking;
        forward = 1;
        is_same_color = is_white;
        is_opposite_color = is_black;
    }
    else {
        pawn = bpawn, op_pawn =wpawn;
        rook = brook, op_rook = wrook;
        knight = bknight, op_knight = wknight;
        bishop = bbishop, op_bishop = wbishop;
        queen = bqueen, op_queen = wqueen;
        king = bking, op_king = wking;
        forward = -1;
        is_same_color = is_black;
        is_opposite_color = is_white;
    }
    std::list<BOARD> boards;
    int i, j;
    for (i = 0; i <= 7; i++) {
        for (j = 0; j <= 7; j++) {
            if (board[i][j] == pnil)
                ;
            else if (board[i][j] == pawn) {
                if ((i+forward)>=0 && (i+forward)<=7) {
                    if (board[i+forward][j] == pnil) {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[i+forward][j] = pawn;
                        boards.push_back(new_board);
                    }
                    if (j+1<=7 && is_opposite_color(board[i+forward][j+1])) {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[i+forward][j+1] = pawn;
                        boards.push_back(new_board);
                    }
                    if (j-1>=0 && is_opposite_color(board[i+forward][j-1])) {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[i+forward][j-1] = pawn;
                        boards.push_back(new_board);
                    }
                }
            }
            else if (board[i][j] == rook) {
                for (int x = i+1; x <= 7; x++) {
                    if (is_same_color(board[x][j]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[x][j] = rook;
                        boards.push_back(new_board);
                        if (board[x][j] != pnil)
                            break;
                    }
                }
                for (int x = i-1; x >= 0; x--) {
                    if (is_same_color(board[x][j]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[x][j] = rook;
                        boards.push_back(new_board);
                        if (board[x][j] != pnil)
                            break;
                    }
                }
                for (int y = j+1; y <= 7; y++) {
                    if (is_same_color(board[i][y]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[i][y] = rook;
                        boards.push_back(new_board);
                        if (board[i][y] != pnil)
                            break;
                    }
                }
                for (int y = j-1; y >= 0; y--) {
                    if (is_same_color(board[i][y]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[i][y] = rook;
                        boards.push_back(new_board);
                        if (board[i][y] != pnil)
                            break;
                    }
                }
            }
            else if (board[i][j] == knight) {
                for (int x = i-2; x <= i+2; x++) {
                    for (int y = j-2; y <= j+2; y++) {
                        if (abs(x-i)+abs(y-j)==3 && x>=0 && x<=7 && y>=0 && y<=7 && !is_same_color(board[x][y])) {
                            BOARD new_board = board;
                            new_board[i][j] = pnil;
                            new_board[x][y] = knight;
                            boards.push_back(new_board);
                        }
                    }
                }
            }
            else if (board[i][j] == bishop) {
                for (int x=i+1, y=j+1; x<=i+7 && x<=7 && y<=j+7 && y<=7; x++, y++) {
                    if (is_same_color(board[x][y]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[x][y] = bishop;
                        boards.push_back(new_board);
                        if (board[x][y] != pnil)
                            break;
                    }
                }
                for (int x=i+1, y=j-1; x<=i+7 && x<=7 && y>=j-7 && y>=0; x++, y--) {
                    if (is_same_color(board[x][y]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[x][y] = bishop;
                        boards.push_back(new_board);
                        if (board[x][y] != pnil)
                            break;
                    }
                }
                for (int x=i-1, y=j-1; x>=i-7 && x>=0 && y>=j-7 && y>=0; x--, y--) {
                    if (is_same_color(board[x][y]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[x][y] = bishop;
                        boards.push_back(new_board);
                        if (board[x][y] != pnil)
                            break;
                    }
                }
                for (int x=i-1, y=j+1; x>=i-7 && x>=0 && y<=j+7 && y<=7; x--, y++) {
                    if (is_same_color(board[x][y]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[x][y] = bishop;
                        boards.push_back(new_board);
                        if (board[x][y] != pnil)
                            break;
                    }
                }
            }
            else if (board[i][j] == queen) {
                for (int x = i+1; x <= 7; x++) {
                    if (is_same_color(board[x][j]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[x][j] = queen;
                        boards.push_back(new_board);
                        if (board[x][j] != pnil)
                            break;
                    }
                }
                for (int x = i-1; x >= 0; x--) {
                    if (is_same_color(board[x][j]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[x][j] = queen;
                        boards.push_back(new_board);
                        if (board[x][j] != pnil)
                            break;
                    }
                }
                for (int y = j+1; y <= 7; y++) {
                    if (is_same_color(board[i][y]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[i][y] = queen;
                        boards.push_back(new_board);
                        if (board[i][y] != pnil)
                            break;
                    }
                }
                for (int y = j-1; y >= 0; y--) {
                    if (is_same_color(board[i][y]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[i][y] = queen;
                        boards.push_back(new_board);
                        if (board[i][y] != pnil)
                            break;
                    }
                }
                for (int x=i+1, y=j+1; x<=i+7 && x<=7 && y<=j+7 && y<=7; x++, y++) {
                    if (is_same_color(board[x][y]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[x][y] = queen;
                        boards.push_back(new_board);
                        if (board[x][y] != pnil)
                            break;
                    }
                }
                for (int x=i+1, y=j-1; x<=i+7 && x<=7 && y>=j-7 && y>=0; x++, y--) {
                    if (is_same_color(board[x][y]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[x][y] = queen;
                        boards.push_back(new_board);
                        if (board[x][y] != pnil)
                            break;
                    }
                }
                for (int x=i-1, y=j-1; x>=i-7 && x>=0 && y>=j-7 && y>=0; x--, y--) {
                    if (is_same_color(board[x][y]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[x][y] = queen;
                        boards.push_back(new_board);
                        if (board[x][y] != pnil)
                            break;
                    }
                }
                for (int x=i-1, y=j+1; x>=i-7 && x>=0 && y<=j+7 && y<=7; x--, y++) {
                    if (is_same_color(board[x][y]))
                        break;
                    else {
                        BOARD new_board = board;
                        new_board[i][j] = pnil;
                        new_board[x][y] = queen;
                        boards.push_back(new_board);
                        if (board[x][y] != pnil)
                            break;
                    }
                }
            }
            else if (board[i][j] == king) {
                for (int x = max(i-1,0); x <= min(i+1,7); x++) {
                    for (int y = max(j-1,0); y <= min(j+1,7); y++) {
                        if (!is_same_color(board[x][y])) {
                            BOARD new_board = board;
                            new_board[i][j] = pnil;
                            new_board[x][y] = king;
                            boards.push_back(new_board);
                        }
                    }
                }
            }
        }
    }
    return boards;
}

int minimax(BOARD board, int depth, int maximize_player, int color) {
    if (depth == 0 || game_over(board)) {
        return evaluate(board);
    }
    if (maximize_player) {
        int best_value = -INF;
        std::list<BOARD> boards = next_boards(board, color);
        for (BOARD next_board : boards) {
            int value = minimax(next_board, depth-1, 0, 1-color);
            best_value = max(best_value, value);
        }
        return best_value;
    }
    else {
        int best_value = INF;
        std::list<BOARD> boards = next_boards(board, color);
        for (BOARD next_board : boards) {
            int value = minimax(next_board, depth-1, 1, 1-color);
            best_value = min(best_value, value);
        }
        return best_value;
    }
}
