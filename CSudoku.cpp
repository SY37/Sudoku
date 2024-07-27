
#include "CSudoku.hpp"
#include <cstddef>

CSudoku::CSudoku() {

    // initialize conflicts
    initConflicts();
}
CSudoku::CSudoku(char *str)
    : CSudoku() {
    readin(str);
}
bool CSudoku::loadStr(const char *sudoku_str, bool clr_obo) {
    // load sudoku from the string and returns false when the digit-char count is less then 81 or when the sudoku is not right

    auto ch2d = [](char ch) {
        // if the char is a digit then returns its value or `-1`

        if (('0' <= ch) && (ch <= '9')) {
            return (int)(ch - '0');
        }

        return -1;
    };

    auto checkDigit = [&](int indx, int digt) {
        // compare the digit of self and mates, and return `true` if the digit is enabled.
        // must (1 <= digit <= 9)
        if (digt == 0) {
            return true;
        }

        // each range
        for (int r = RANGE_ROW; r <= RANGE_BLO; r++) {
            // the index of the 1st cell in this range
            int first_i = rangeFirst(indx, r);
            // each mate-cell in this range
            for (int n = 0; n < 9; n++) {
                // index of mate-cell
                int mate_i = inRange(first_i, n, r);
                // skip self
                if (mate_i == indx) {
                    continue;
                };
                // found conflict
                if (digt == getDigit(mate_i)) {
                    // return the digit is not enabled
                    return false;
                };
            }
        }
        // return the digit is enabled
        return true;
    };

    if (sudoku_str == NULL) {
        return false;
    }

    // backup sudoku (for reverting sudoku if the string is invalid)
    char bp[82];
    toString(bp);

    // initialize conflicts (to clear the conflicts of the previous sudoku)
    initConflicts();

    // readin
    const char *pch = sudoku_str;
    int digit, index;
    int count = 0;
    bool BAD_SUDOKU = false;
    for (count = 0; true;) {
        // get one digit from input string
        digit = ch2d(*pch);
        pch++;
        // commit this digit
        if (digit >= 0) {
            index = count;
            count++;
            // the digit conflicts with some matecell
            if (false == checkDigit(index, digit)) {
                BAD_SUDOKU = true;
                break;
            }
            // commit this digit
            commitCell(index, digit);
        }
        // readin finished when 81 digits has been readin or when the string ended
        if ((count == 81) || (*pch == '\0')) {
            break;
        }
    }

    // failed
    if ((count < 81) || (BAD_SUDOKU == true)) {
        // revert sudoku
        loadStr(bp, false);
        // return
        return false;
    }
    // succeed
    else {
        // clear the OBO mem of the prev sudoku
        if (clr_obo == true) {
            finaOBO();
        }
        return true;
    }
}
void CSudoku::initConflicts() {
    for (int y = 0; y < 9; y++) {
        for (int x = 0; x < 9; x++) {
            for (int d = 0; d < 9; d++) {
                // the digit is enabled
                conflicts[y][x][d][0] = DIGIT_ENABLED;
                // no conflict cells(index)
                conflicts[y][x][d][1] = -1;
                conflicts[y][x][d][2] = -1;
                conflicts[y][x][d][3] = -1;
            }
        }
    }
};
void CSudoku::clrOrMarkMates(int indx, bool clr_or_mark, int prev_digit) {
    // clear status of of mate-cells and set digit to `0`

    bool &clrMode = clr_or_mark;
    int now_digt = getDigit(indx);

    // the previous digit is `0`, this means it marked no status before, so nothing need to do.
    if ((clrMode == true) && (prev_digit == 0)) {
        return;
    }

    // clear or mark this digit status of mate-cells
    auto clrOrMark = [&](int mate_i, int range) {
        // clr mode
        if (clrMode == true) {
            // if the digit of mate-cell was disabled by self-cell then cancel it
            if (indx == getStatus(mate_i, prev_digit, range)) {
                setStatus(mate_i, prev_digit, range, -1);
            }
            // check if the digit is available now
            if (-1 == getStatus(mate_i, prev_digit, RANGE_ROW)) {
                if (-1 == getStatus(mate_i, prev_digit, RANGE_COL)) {
                    if (-1 == getStatus(mate_i, prev_digit, RANGE_BLO)) {
                        setStatus(mate_i, prev_digit, 0, DIGIT_ENABLED);
                    };
                };
            };
        }
        // mark mode
        else {
            // mark this digit disabled by self-cell
            setStatus(mate_i, now_digt, range, indx);
            // mark this digit status of mate-cell as disabled
            setStatus(mate_i, now_digt, 0, DIGIT_DISABLED);
        }
    };

    /* auto eachRowMates = [&](int self_i) {
        int first_i = firstRi(self_i);
        for (int i = 0; i < 9; i++) {
            int mate_i = inRow(first_i, i);
            if (mate_i == self_i) {
                continue;
            }
            clrOrMarkStatus(mate_i, RANGE_ROW);
        }
    };
    auto eachColMates = [&](int self_i) {
        int first_i = firstCi(self_i);
        for (int i = 0; i < 9; i++) {
            int mate_i = inCol(first_i, i);
            if (mate_i == self_i) {
                continue;
            }
            clrOrMarkStatus(mate_i, RANGE_COL);
        }
    };
    auto eachBloMates = [&](int self_i) {
        int first_i = firstBi(self_i);
        for (int i = 0; i < 9; i++) {
            int mate_i = inBlo(first_i, i);
            if (mate_i == self_i) {
                continue;
            }
            clrOrMarkStatus(mate_i, RANGE_BLO);
        }
    };

    // clear or mark mate-cells digit status
    eachRowMates(indx);
    eachColMates(indx);
    eachBloMates(indx);
 */

    for (int range = RANGE_ROW; range <= RANGE_BLO; range++) {
        int first_i = rangeFirst(indx, range);
        for (int i = 0; i < 9; i++) {
            int mate_i = inRange(first_i, i, range);
            if (mate_i == indx) {
                continue;
            }
            clrOrMark(mate_i, range);
        }
    }
};
int CSudoku::seekDigit(int cell_index) {
    // returns the certain digit of the cell or `0` if the digit is unsure or `-1` if the cell is dead.

    int indx = cell_index;

    // get all enabled digits of this cell
    int enabled_digits[9] = {0};
    int enabled_count = 0;
    for (int d = 1; d <= 9; d++) {
        if (DIGIT_ENABLED == getStatus(indx, d, 0)) {
            enabled_digits[enabled_count] = d;
            enabled_count++;
        }
    }

    // this cell has no enabled digit, means sudoku is not right
    if (enabled_count == 0) {
        return SUDOKU_BAD;
    }

    // Fate: only one enabled digit left, so confirm this digit
    if (enabled_count == 1) {
        return enabled_digits[0];
    }

    // Destiny: check if the enabled digit was disabled in all mate-cells
    for (int enabled_i = 0; enabled_i < enabled_count; enabled_i++) {
        // get one enabled digit
        int digit = enabled_digits[enabled_i];
        // check if all mate-cells within the same range disable this digit
        auto ifAllRangeMatesDisabled = [&](int range) {
            int first_i = rangeFirst(indx, range);
            for (int i = 0; i < 9; i++) {
                int mate_i = inRange(first_i, i, range);
                if (mate_i != indx) {
                    if (true == isDigitEnabled(mate_i, digit)) {
                        return false;
                    }
                }
            }
            return true;
        };
        for (int range = RANGE_ROW; range <= RANGE_BLO; range++) {
            // all range-mate-cells disable this digit
            if (true == ifAllRangeMatesDisabled(range)) {
                return digit;
            };
        }
    }

    // cannot confirm any digit
    return SUDOKU_UNSURE;
};
void CSudoku::commitCell(int cell_index, int digit_to_commit) {
    // commit a new digit to the cell and clear the previous digit-conflict-status of mate-cells
    // notice that this func doesnot check if the digit to be committed is enabled

    int indx = cell_index;
    int digt = digit_to_commit;
    int prev_d = getDigit(indx);
    int new_d = digt;

    // clear previous digit status of mate-cells when the previous digit isnot `0`
    if (prev_d != 0) {
        // clear previous digit status of all mate-cells
        clrMatesStatus(indx, prev_d);
    }

    // set the new digit to digit-member
    setDigit(indx, digt);

    // mark the new non-zero digit status of mate-cells
    if (new_d != 0) {
        markMatesStatus(indx);
    }

    //
    return;
};
CSudoku::EnumSudokuStatus CSudoku::commitAllCertains() {
    // commit all certain digits of the sudoku
    // returns `0` when some cells cannt be committed or `-1` when the sudoku isnot right or `1` when all cells were committed.

    // var
    int new_committed = 0;
    int all_committed = 0;

    // scan sudoku panel to seek all ascertainable digits of cells
    bool AGAIN = false;
    for (int i = 0; i < 81; i++) {
        int d = getDigit(i);
        if (d == 0) {
            int certain_d = seekDigit(i);
            // sudoku isnot right
            if (certain_d == SUDOKU_BAD) {
                return SUDOKU_BAD; // func failed, cos the sudoku is not right
            }
            // found certain digit
            if (certain_d > 0) {
                commitCell(i, certain_d);
                new_committed++;
                all_committed++;
                AGAIN = true; // mark to scan again
            }
        } else {
            // check if this digit committed is wrong
            if (false == isDigitEnabled(i, d)) {
                return SUDOKU_BAD;
            }

            all_committed++;
        }
        // sacn the panel again when any new certain digit has been committed
        if ((i == 80) && (AGAIN == true)) {
            // reset flag
            AGAIN = false;
            i = -1;
            all_committed = 0;
        }
    }

    // return
    if (all_committed == 81) {
        // success
        return SUDOKU_BINGO;
    } else {
        // failed
        return SUDOKU_UNSURE;
    }
};

bool CSudoku::isDigitRight(int indx) {
    // check if the committed digit is right
    // notice that make sure all cells have been committed

    // get the committed digit
    int digt = getDigit(indx);

    // cmp with mate-cells
    for (int r = RANGE_ROW; r < RANGE_BLO; r++) {
        int fi = rangeFirst(indx, r);
        for (int n = 0; n < 9; n++) {
            int mi = inRange(fi, n, r);
            // skip self
            if (mi == indx) {
                continue;
            }
            // the same digit as mate
            if (digt == getDigit(mi)) {
                // reutrn this digit isnot right
                return false;
            }
        }
    }

    // the digit is right
    return true;
};
bool CSudoku::isBingo() {
    // check if the sudoku is a answer

    for (int i = 0; i < 81; i++) {
        int d = getDigit(i);
        if (d == 0) {
            return false;
        }
        if (false == isDigitRight(i)) {
            return false;
        }
    }

    return true;
};
bool CSudoku::hasWrong() {
    // check if any committed cell is wrong

    for (int i = 0; i < 81; i++) {
        int d = getDigit(i);
        if (d == 0) {
            continue;
        }
        if (false == isDigitEnabled(i, d)) {
            return true;
        }
    }
    return false;
};

bool CSudoku::crack(bool init_recursion) {

    static int answer_count;
    static int last_tried_indx;
    static int lv;
    if (init_recursion == true) {
        last_tried_indx = -1;
        lv = -1;
    }

    lv++;
    bool FOUND = false;
    int sudoku_status;
    sudoku_status = commitAllCertains();
    switch (sudoku_status) {
        // failed
        case SUDOKU_BAD :
            FOUND = false;
            break;
        // success
        case SUDOKU_BINGO :
            if (__whenCrackBingo == NULL) {
                __whenCrackBingo = __whenCrackBingo_;
            }
            __whenCrackBingo(this);
            FOUND = true;
            break;
        // Zombie: guess all enabled digit of one cell
        case SUDOKU_UNSURE :
            // get the next uncommitted cell index
            int indx = -1;
            for (int i = last_tried_indx + 1; i < 81; i++) {
                if (0 == getDigit(i)) {
                    indx = i;
                    break;
                }
            }
            if (indx == -1) {
                break;
            }
            // get one enabled digit of this cell
            for (int digt = 1; digt <= 9; digt++) {
                if (true == isDigitEnabled(indx, digt)) {
                    // backpu current sudoku
                    char *bp = new char[81];
                    toString(bp);
                    // try to crack with this digit
                    commitCell(indx, digt);
                    last_tried_indx = indx;
                    bool b = crack(false);
                    if (b == true) {
                        FOUND = true;
                    }
                    lv--;
                    // recover sudoku
                    readinOLD(bp);
                    delete[] bp;
                };
            }
            break;
    }

    // return
    return FOUND;
};
CSudoku *CSudoku::crackOBO() {

    COBOLocal **&LOCAL = obo_local;
    CSudoku *&SUDOKU = obo_suduku;
    char &LEVEL = obo_level;
    bool &FOUND = obo_found;
    bool TEST = true;

    auto goUpLevel = [&]() {
        // go back to prev recursion level
        if (LEVEL > 0) {
            // update level
            LEVEL--;
            TEST = false;
            // recover sudoku to this level
            char *&bp = LOCAL[LEVEL]->try_bp;
            SUDOKU->readinOLD(bp);
            //
            return true;
        } else {
            // cant go back, cos no more level in the front
            finaOBO();
            return false;
        }
    };

    initOBO();
    // check last return, if it found answer
    if (FOUND == true) {
        FOUND = false;
        if (false == goUpLevel()) {
            return nullptr;
        }
    }
    // zombie on
    while (true) {
        if (TEST) {
            int sudoku_status = SUDOKU->commitAllCertains();
            if (sudoku_status == SUDOKU_BINGO) {
                FOUND = true;
                return SUDOKU;
            }
            if (sudoku_status == SUDOKU_BAD) {
                if (false == goUpLevel()) {
                    return nullptr;
                }
            }
        }

        // beginning of this level
        if (LOCAL[LEVEL] == nullptr) {
            // init var
            LOCAL[LEVEL] = new COBOLocal();
            // get the cell index of this level
            char &i = LOCAL[LEVEL]->try_indx;
            int iB = 0;
            if (LEVEL > 0) {
                iB = LOCAL[LEVEL - 1]->try_indx + 1;
            }
            for (i = iB; i < 81; i++) {
                if (0 == SUDOKU->getDigit(i)) {
                    break;
                }
            }
            // backpu current sudoku before testing some digits
            char *&b = LOCAL[LEVEL]->try_bp;
            b = new char[82];
            SUDOKU->toString(b);
            //
            char &d = LOCAL[LEVEL]->try_digt;
            d = 0;
        }
        char &cur_indx = LOCAL[LEVEL]->try_indx;
        char &cur_digt = LOCAL[LEVEL]->try_digt;
        char *&bp = LOCAL[LEVEL]->try_bp;

        // get one enabled digit of this cell to test
        cur_digt++;
        for (; cur_digt <= 9; cur_digt++) {
            if (true == SUDOKU->isDigitEnabled(cur_indx, cur_digt)) {
                break;
            }
        }
        // try this digit
        if (cur_digt <= 9) {
            // commit it
            SUDOKU->commitCell(cur_indx, cur_digt);
            // goto the next level
            TEST = true;
            LEVEL++;
            continue;
        }

        // ending of this level
        if (cur_digt > 9) {
            // clear local var
            cur_indx = -1;
            cur_digt = 0;
            delete[] bp;
            bp = nullptr;
            delete LOCAL[LEVEL];
            LOCAL[LEVEL] = nullptr;
            // go back to the prev level
            if (false == goUpLevel()) {
                return nullptr;
            }
            continue;
        }
    }
};

void CSudoku::toString(char *str) {
    for (int i = 0; i < 81; i++) {
        str[i] = getDigit(i) + '0';
    }
    str[81] = '\0';
}
void CSudoku::toPrettyLine(char *buffer) {
    // print sudoku as one line to the buffer
    // style like this: 123 456 789 | 234 567 891 | ...

    // write one char to the buffer
    char *b = buffer;
    auto pushB = [&b](char c) { *(b++) = c; };

    // get the string of this sudoku to write
    char *str = new char[SIZE_STRING]();
    toString(str);
    char *c = str;

    // set the char of unknow cells
    for (int i = 0; i < 81; i++) {
        if (str[i] == '0') {
            str[i] = '.';
        }
    }

    int c3 = 0;
    int b3 = 0;
    for (int i = 0; i < 81; i++) {
        char ch = *(c++);
        pushB(ch);
        c3++;

        if (c3 == 3) {
            c3 = 0;
            pushB(' ');
            b3++;
        }

        if (b3 == 3) {
            b3 = 0;
            pushB(' ');
            pushB('|');
            pushB(' ');
        }
    }

    b -= 3;
    pushB('\0');
    pushB('\0');
    pushB('\0');
    delete[] str;
}
void CSudoku::toPrettyPanel(char *buffer, int gazed_index, int gazed_style) {
    // print sudoku as one panel to the buffer
    /* style like this
"
 +-----------+-----------+-----------+ \n
 |  2  6  3  |  5  1  7  |  8  9  4  | \n
 |  7  8  1  |  9  4  2  |  3  5  6  | \n
 |  9  4  5  |  8  3  6  |  2  7  1  | \n
 +-----------+-----------+-----------+ \n
 |  5  1  8  |  6  9  4  |  7  3  2  | \n
 |  4  3  9  |  7  2  5  |  1  6  8  | \n
 |  6  2  7  |  1  8  3  |  5  4  9  | \n
 +-----------+-----------+-----------+ \n
 |  1  7  4  |  3  6  8  |  9  2  5  | \n
 |  3  9  2  |  4  5  1  |  6  8  7  | \n
 |  8  5  6  |  2  7  9  |  4  1  3  | \n
 +-----------+-----------+-----------+ \n
0x0000000147004330 " +-----------+-----------+-----------+ \n |  2  6  3  |  5  1  7  |  8  9  4  | \n |  7  8  1  |  9  4  2  |  3  5  6  | \n |  9  4  5  |  8  3  6  |  2  7  1  | \n +-----------+-----------+-----------+ \n |  5  1  8  |  6  9  4  |  7  3  2  | \n |  4  3  9  |  7  2  5  |  1  6  8  | \n |  6  2  7  |  1  8  3  |  5  4  9  | \n +-----------+-----------+-----------+ \n |  1  7  4  |  3  6  8  |  9  2  5  | \n |  3  9  2  |  4  5  1  |  6  8  7  | \n |  8  5  6  |  2  7  9  |  4  1  3  | \n +-----------+-----------+-----------+ \n"
"


*/

    // the chars to draw the panel
    char ch_space = ' ';
    char ch_vertical = '|';
    char ch_horizon = '-';
    char ch_corner = '+';
    char ch_unsure_digit = ' ';
    char ch_gl;
    char ch_gr;
    auto getGazedStyle = [](int gazed_i, char &gl, char &gr) {
        // highlight style of the gazed cell

        char gls[] = "<[(>_*|-=~";
        char grs[] = ">])<_*|-=~";
        int i_max = sizeof(gls) - 1 - 1;

        if (gazed_i < 0) {
            gazed_i = 0;
        }
        if (gazed_i > i_max) {
            gazed_i = i_max;
        }

        int i = gazed_i;
        gl = gls[i];
        gr = grs[i];
    };
    if (gazed_index != -1) {
        getGazedStyle(gazed_style, ch_gl, ch_gr);
    } else {
        ch_gl = ch_space;
        ch_gr = ch_space;
    }

    // write one char to the buffer
    char *b = buffer;
    auto pushB = [&b](char c) { *(b++) = c; };

    // get the string of sudoku
    char *str = new char[SIZE_STRING];
    toString(str);

    // print the sudoku panel
    int i = 0;
    char ch_1, ch_2, ch_3;
    // print lines
    for (int ln = 0; ln < 13; ln++) {
        // confirm if the current line is border line
        bool border_line = false;
        if (ln % 4 == 0) {
            border_line = true;
        }

        // print the left border of one line
        ch_1 = ch_vertical;
        if (border_line == true) {
            ch_1 = ch_corner;
        }
        pushB(' ');
        pushB(ch_1);

        // print 3 blos of one line
        for (int bl = 0; bl < 3; bl++) {
            // print left border of one blo
            ch_1 = ch_space;
            if (border_line == true) {
                ch_1 = ch_horizon;
            }
            pushB(ch_1);

            //  print 3 cells of one blo
            for (int cl = 0; cl < 3; cl++) {
                if (border_line == true) {
                    ch_1 = ch_horizon;
                    ch_2 = ch_horizon;
                    ch_3 = ch_horizon;
                } else {
                    ch_1 = ch_space;
                    ch_3 = ch_space;
                    if (i == gazed_index) {
                        ch_1 = ch_gl;
                        ch_3 = ch_gr;
                    }
                    ch_2 = str[i];
                    if (ch_2 == '0') {
                        ch_2 = ch_unsure_digit;
                    }
                    i++;
                }
                pushB(ch_1);
                pushB(ch_2);
                pushB(ch_3);
            }
            // print the right border of one blo
            ch_1 = ch_space;
            ch_2 = ch_vertical;
            if (border_line == true) {
                ch_1 = ch_horizon;
                ch_2 = ch_corner;
            }
            pushB(ch_1);
            pushB(ch_2);
        }
        // print one line finished
        pushB(' ');
        pushB('\n');
    }
    // print \0
    pushB('\0');

    // return
    delete[] str;
}
