#ifndef __CGAME
#define __CGAME

#include "CSudoku.hpp"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <random>

class CGame {
    CSudoku origin;
    CSudoku playing;
    bool cantAccess() {
        return (origin.getDigit(y, x) != 0);
    };

    int gazing = 0; // i
    int y = 0;
    int x = 0;
    void limitGazing() {
        if (gazing < 0) {
            gazing *= -1;
            gazing %= 81;
            gazing = 81 - gazing;
        } else {
            gazing %= 81;
        }
    };
    void updateXY() {
        CSudoku::i2xy(y, x, gazing);
    }
    void see(int i) {
        gazing = i;
        limitGazing();
        updateXY();
    };
    void seeRt(int offs) {
        gazing += offs;
        limitGazing();
        updateXY();
    };
    void seeLf(int offs) {
        gazing -= offs;
        limitGazing();
        updateXY();
    };
    void seeDw(int offs) {
        gazing += offs * 9;
        limitGazing();
        updateXY();
    };
    void seeUp(int offs) {
        gazing -= offs * 9;
        limitGazing();
        updateXY();
    };

    char panel[CSudoku::SIZE_PANEL] = {0};

    void limitDigit(int &d) {
        if (d < 1) {
            d = 1;
        }
        if (d > 9) {
            d = 9;
        }
    };

    class CHistory {
        struct SOperation {
            char indx;
            char new_d;
            char old_d;
        };
        int oprt_i = -1;
        int oprt_cnt = 0;
        int oprt_cnt_MAX = 0;
        SOperation *oprts = NULL;
        void renew() {
            int new_MAX = (oprt_cnt + 9) / 10 * 10;
            if (new_MAX == oprt_cnt_MAX) {
                return;
            }

            SOperation *new_oprts = new SOperation[new_MAX];
            int less_size = new_MAX <= oprt_cnt_MAX ? new_MAX : oprt_cnt_MAX;
            if ((less_size != 0) && (oprts != NULL)) {
                memmove(new_oprts, oprts, less_size * sizeof(SOperation));
            }

            delete[] oprts;
            oprts = new_oprts;
            oprt_cnt_MAX = new_MAX;
        }
        void getCurrent(int *i, int *new_d, int *old_d) {
            if (i != NULL) {
                *i = oprts[oprt_i].indx;
            }

            if (new_d != NULL) {
                *new_d = oprts[oprt_i].new_d;
            }

            if (old_d != NULL) {
                *old_d = oprts[oprt_i].old_d;
            }
        }

    public:
        ~CHistory() {
            delete[] oprts;
        };
        void deepCopy(CHistory one) {
            delete[] oprts;
            *this = one;
            if (oprts != NULL) {
                oprts = new SOperation[oprt_cnt_MAX];
                memmove(oprts, one.oprts, oprt_cnt_MAX * sizeof(SOperation));
            }
        };
        void h_newdo(int indx, int new_d, int old_d) {
            oprt_i++;
            oprt_cnt = oprt_i + 1;
            renew();

            oprts[oprt_i].indx = indx;
            oprts[oprt_i].new_d = new_d;
            oprts[oprt_i].old_d = old_d;
        };
        bool h_undo(int &i, int &old_digit) {
            if (oprt_i == -1) {
                return false;
            }

            getCurrent(&i, NULL, &old_digit);
            oprt_i--;

            return true;
        }
        bool h_redo(int &i, int &new_digit) {
            if (oprt_i == (oprt_cnt + 1)) {
                return false;
            }

            oprt_i++;
            getCurrent(&i, &new_digit, NULL);

            return true;
        }
    };
    CHistory history;
    void enterDigit(int d, bool record_hist);
    void clearDigit(bool record_his);

    CGame()
        : origin(),
          playing(){

          };
    void bindSudoku(CSudoku *p) {
        char *bp = new char[CSudoku::SIZE_STRING];
        p->toString(bp);

        origin.readin(bp);
        playing.readin(bp);

        delete[] bp;
        bp = nullptr;
    };

    long long ans_i = -1;

public:
    ~CGame() {
        origin.closeOBO();
        playing.closeOBO();
    };

    CGame(char *sudoku_str)
        : origin(sudoku_str),
          playing(sudoku_str){

          };

    void reset();

    void refresh();
    void showSudoku(CSudoku *sd);
    void go(int i);
    void goRight(int offs);
    void goLeft(int offs);
    void goDown(int offs);
    void goUp(int offs);
    void commitCell(int i, int d, bool h) {
        go(i);
        if (d == 0) {
            clearDigit(h);
        } else {
            enterDigit(d, h);
        }
    };
    void enter(int d);
    void backspace();
    void undo() {
        int i, d;
        bool hasOperation = history.h_undo(i, d);
        if (hasOperation == false) {
            return;
        }

        commitCell(i, d, false);
    };
    void redo() {
        int i, d;
        bool hasOperation = history.h_redo(i, d);
        if (hasOperation == false) {
            return;
        }

        commitCell(i, d, false);
    };

    bool isSuccess() {
        return (playing.isBingo() == true);
    }
    bool checkWrong() {
        return (playing.hasWrong() == true);
    }
    bool hintOne() {
        // commit a cell with certain digit and returns `true`, or returns `false` when none cell is ascertainable directly

        static int next_i = 0;
        auto findCertain = [&](int i) {
            // if this cell could be committed then commit it
            int y, x, d;
            playing.i2xy(y, x, i);
            d = playing.getDigit(y, x);
            if (d == 0) {
                d = playing.certainDigit(y, x);
                if (d > 0) {
                    go(i);
                    enter(d);
                    next_i = i + 1;
                    return true;
                }
            }
            return false;
        };

        for (int i = next_i; i < 81; i++) {
            if (true == findCertain(i)) {
                return true;
            }
        }

        for (int i = 0; i < next_i; i++) {
            if (true == findCertain(i)) {
                return true;
            }
        }

        // none
        return false;
    };
    long long showAnswers();
};

#endif