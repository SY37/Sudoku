

#include "CGame.hpp"
#include "CSudoku.hpp"
#include <alloca.h>
#include <cstddef>
#include <cstdio>
#include <regex>

void CGame::reset() {
    playing = origin;
    history.deepCopy(CHistory());
    refresh();
}

void CGame::refresh() {
    playing.toPrettyPanel(panel, y, x, 0);

    printf("%*c \n%s \n", 9999, ' ', panel);
};

void CGame::showSudoku(CSudoku *sd) {
    sd->toPrettyPanel(panel, -1, -1, 0);

    printf("%*c \n%s \n", 9999, ' ', panel);
};

void CGame::go(int i) {
    see(i);
    refresh();
};
void CGame::goRight(int offs) {
    seeRt(offs);
    refresh();
};
void CGame::goLeft(int offs) {
    seeLf(offs);
    refresh();
};
void CGame::goDown(int offs) {
    seeDw(offs);
    refresh();
};
void CGame::goUp(int offs) {
    seeUp(offs);
    refresh();
};

void CGame::enterDigit(int d, bool record_hist) {
    if (cantAccess()) {
        return;
    }

    int old_d = playing.getDigit(y, x);
    limitDigit(d);
    playing.commitDigit(y, x, d);

    if (record_hist == true) {
        history.h_newdo(gazing, d, old_d);
    }

    refresh();
}
void CGame::clearDigit(bool record_his) {
    if (cantAccess()) {
        return;
    }

    int old_d = playing.getDigit(y, x);
    playing.commitDigit(y, x, 0);

    if (record_his == true) {
        history.h_newdo(gazing, 0, old_d);
    }

    refresh();
}

void CGame::enter(int d) {
    enterDigit(d, true);
}
void CGame::backspace() {
    clearDigit(true);
}

long long CGame::showAnswers() {
    // show answers one by one, and returns the index of answer, if the sudoku isnt right then returns `-1`.

    char buf[CSudoku::SIZE_PANEL];

    CSudoku *ans = origin.crackOBO();
    if (ans == nullptr) {
        // bad sudoku
        if (ans_i == -1) {
            return -1;
        }
        // reget the 1st answer
        ans_i = -1;
        ans = origin.crackOBO();
    }

    if (ans != nullptr) {
        ans_i++;
        ans->toPrettyPanel(buf, -1, -1, -1);
        printf("#%lld \n%s \n", ans_i, buf);
    }
    return (ans_i);
}
