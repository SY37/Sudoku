
#include "CSudoku.hpp"
#include "CBigNumb.hpp"
#include "CGame.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>

//-- main

/* XOS - getch() */
#ifdef _WIN32
#include <conio.h>
#else
#include "termios.h"
#include "unistd.h"
char getch() {

    // get current terminal property
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // disable echo and canonical mode
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // readin one char
    char ch;
    ch = getchar();

    // recover property
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    // return
    return ch;
}
#endif

char some[20][256] = {
    // easy
    "600000278000700005400005003000602000002034080504000609308006400000001007000809000",
    //
    ""
    "000000000 "
    "000000000 "
    "000000000 "
    ""
    "000000000 "
    "000000000 "
    "000000000 "
    ""
    "000000000 "
    "000000000 "
    "000000000 ",

    ""
    "850 000 200"
    "000 000 480 "
    "230 040 006 "
    "300 700 000 "
    "020 904 050 "
    "009 031 000 "
    "007 000 090 "
    "000 080 005 "
    "000 200 060",

    ""
    "800 000 200"
    "000 000 480 "
    "230 040 006 "
    "300 700 000 "
    "020 904 050 "
    "009 031 000 "
    "007 000 090 "
    "000 080 005 "
    "000 200 060",

    ""
    "100 000 709 "
    "000 030 501 "
    "000 090 000 "
    "028 700 000 "
    "000 062 000 "
    "090 100 003 "
    "507 000 000 "
    "002 004 000 "
    "040 309 270",

    ""
    "003 500 004 "
    "780 000 000"
    "000 806 070"
    "000 000 000"
    "400 705 108"
    "620 003 049"
    "070 000 020"
    "090 000 600"
    "000 209 010",
};
void printSudokuLine(CSudoku *sudoku) {
    char *buf = new char[CSudoku::SIZE_LINE];

    sudoku->toPrettyLine(buf);
    printf("%s\n", buf);

    delete[] buf;
};
void printSudokuPanel(CSudoku *sudoku) {
    char *buf = new char[CSudoku::SIZE_PANEL];

    sudoku->toPrettyPanel(buf, -1, -1, -1);
    printf("%s\n", buf);

    delete[] buf;
};
void D_crackAnySudoku() {
    // load a sudoku
    CSudoku *sudoku = new CSudoku(some[3]);

    // set callback func of crack success
    auto whenCrackBingo = [](CSudoku *this_) { printSudokuPanel(this_); };
    sudoku->setCall_CrackBingo(whenCrackBingo);

    // crack
    sudoku->crack();

    // free
    delete sudoku;
}
CBigNumb count;
void D_sudokusInWorld() {
    CSudoku empty_sudoku;

    auto whenBingo = [](CSudoku *sudoku) {
        count++;
        if (count.getLow() % 10000 == 0) {
            char *str = count.toString();
            printf("%s \n", str);
            count.freeString(str);

            printSudokuPanel(sudoku);
        }
    };
    empty_sudoku.setCall_CrackBingo(whenBingo);

    count.zero();
    empty_sudoku.crack();
}
void D_crackOBO() {
    CSudoku *s = new CSudoku(some[3]);
    int cnt = 0;
    char buf[CSudoku::SIZE_PANEL];

    while (true) {
        if (' ' != getch()) {
            s->closeOBO();
            break;
        }

        CSudoku *ans = s->crackOBO();
        if (ans == nullptr) {
            printf("No more, %d answers found. \n", cnt);
            cnt = 0;
        } else {
            cnt++;
            printf("#%.d \n", cnt);
            printSudokuPanel(ans);
        }
    }
};

void play() {
    char key;
    auto popKey = [&]() {
        key = getch();
        return key;
    };
    auto info = []() {
        printf("I : To show this helpful info. \n");
        printf("; : To exit. \n");
        printf("↑↓←→ : To move cursor. \n");
        printf("123456789 : To put the digit into the cell. \n");
        printf("delete : To cancel the committed digit. \n");
        printf("spacebar : To check if any cell has conflict. \n");
        printf("H : To hint one cell. \n");
        printf("enter : To submit. \n");
        printf("R : To restart. \n");
        printf("C : To show the answers. \n");
    };

    printf("\n\n=============================\n");
    info();
    printf("=============================\n");
    printf("\nPress any key to continue... \n");
    popKey();
    while (key == '\033') {
        popKey();
        if (key == '[') {
            popKey();
        }
    }

    CGame game(some[0]);
    game.refresh();

    enum EnumKey { ANSI_DEL = 127,
                   ANSI_BS = 8 };
    bool QUIT = false;
    while (!QUIT) {
        if ('\033' == popKey()) {
            if ('[' == popKey()) {
                bool OTHER_KEY = false;
                popKey();
                switch (key) {
                    case 'A' : // ↑
                        game.goUp(1);
                        break;
                    case 'B' : // ↓
                        game.goDown(1);
                        break;
                    case 'C' : // →
                        game.goRight(1);
                        break;
                    case 'D' : // ←
                        game.goLeft(1);
                        break;
                    case '3' :
                        if ('~' == popKey()) {
                            // del
                            game.backspace();
                        } else {
                            key = '3';
                            OTHER_KEY = true;
                        }
                        break;
                    default :
                        OTHER_KEY = true;
                        break;
                }
                if (OTHER_KEY == false) {
                    continue;
                }
            }
        }

        switch (key) {
            case ';' :
                QUIT = true;
                break;
            case 'I' :
            case 'i' :
                info();
                break;
            case 'H' :
            case 'h' :
                if (false == game.hintOne()) {
                    printf("I cant help you any more zZZ... \n");
                }
                break;
            case '1' :
            case '2' :
            case '3' :
            case '4' :
            case '5' :
            case '6' :
            case '7' :
            case '8' :
            case '9' :
                game.enter(key - '0');
                break;
            case ANSI_BS :
            case ANSI_DEL :
                game.backspace();
                break;
            case 'W' :
            case 'w' :
                game.goUp(1);
                break;
            case 'S' :
            case 's' :
                game.goDown(1);
                break;
            case 'D' :
            case 'd' :
                game.goRight(1);
                break;
            case 'A' :
            case 'a' :
                game.goLeft(1);
            case 'R' :
            case 'r' :
                game.reset();
                break;
            case 'C' :
            case 'c' :
                game.showAnswers();
                break;
            case ' ' :
                if (!game.checkWrong()) {
                    printf("[^_^] No problem found! \n");
                } else {
                    printf("[O_O] Something went wrong, check it! \n");
                }
                break;
            case 'Z' :
            case 'z' :
                game.undo();
                break;
            case 'Y' :
            case 'y' :
                game.redo();
                break;
            case '\n' :
            case '\r' :
                if (game.isSuccess()) {
                    printf("[^_^] Bingo! You are so smart! \n");
                } else {
                    printf("[O_O] It is not right, try again! \n");
                }
                break;
        }
    }
};

int main(int argc, char *argv[]) {

    // D_crackAnySudoku();
    // D_sudokusInWorld();
    // D_crackOBO();
    play();

    return 0;
}
