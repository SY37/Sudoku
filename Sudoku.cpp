
#include "Sudoku.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#define DEBUG 0

#define CHECK_DEBUG   \
    if (DEBUG != 1) { \
        return;       \
    }

static char lastString[510];

auto limitNumbRange = [](int &n, int min, int max) {
    if (n < min) {
        n = min;
    }
    if (n > max) {
        n = max;
    }
};

bool Sudoku::readin(const char *sudoku_str) {
    auto DEBUG_readin_bad = [this](int i, int d) {
        CHECK_DEBUG;
        printf("\n"
               "@@ @readin_badDigitAt@ [%d:%d] \n"
               "@@: \n",
               i, d);
        printPanel();
        printf("\n");
    };
    // if char is digit or the specified char meaning any digit
    auto isAllowedDigit = [this](char ch) {
        if (('1' <= ch) && (ch <= '9'))
            return (int)(ch - '0');
        if ((ch == '0') || (ch == this->ch_anyDigit))
            return 0;
        return -1;
    };

    // backup sudoku
    char bp[82];
    getString(bp);

    // initialize conflicts
    initConflicts();

    // readin
    const char *pch = sudoku_str;
    int digit;
    int count = 0;
    bool bad_sudoku = false;
    for (count = 0; true;) {
        // get digit
        digit = isAllowedDigit(*pch++);
        // if the char is digit then readin
        int i = count;
        if (digit >= 0) {
            if (false == commitDigit(i, digit, true)) {
                bad_sudoku = true;
                DEBUG_readin_bad(i, digit);
                break;
            }
            count++;
        }
        // break when: 81 digits has been readin / sudoku string ended
        if ((count == 81) || (*pch == '\0')) {
            break;
        }
    }

    // readin failed
    if ((count < 81) || (bad_sudoku == true)) {
        // revert sudoku
        readin(bp);
        // return
        return false;
    }
    // readin succeed
    return true;
}
void Sudoku::initConflicts() {
    for (int y = 0; y < 9; y++) {
        for (int x = 0; x < 9; x++) {
            for (int d = 0; d < 9; d++) {
                conflicts[y][x][d][0] = flag_conf_enableDigit;
                conflicts[y][x][d][1] = -1;
                conflicts[y][x][d][2] = -1;
                conflicts[y][x][d][3] = -1;
            }
        }
    }
};
bool Sudoku::checkConflict(int indx, int digt) {
    // return true when the digit is enable (has no conflict)
    // check if the digit has no conflict (by comparing with committed digits of mates)

    //
    limitNumbRange(digt, 0, 9);
    if (digt == 0) {
        return true;
    }
    //
    if (false == checkConflict_row(indx, digt, NULL)) {
        return false;
    }
    if (false == checkConflict_col(indx, digt, NULL)) {
        return false;
    }
    if (false == checkConflict_blo(indx, digt, NULL)) {
        return false;
    }
    return true;
};
bool Sudoku::checkConflict_row(int indx, int digt, int *conflict_i) {
    // return true when the digit is enable (has no conflict)
    // check if the digit has no conflict (by comparing with committed digit of mates) and export index of conflict-cell

    limitNumbRange(digt, 0, 9);
    if (digt == 0) {
        return true;
    }

    // get x y
    int y, x;
    itoxy(y, x, indx);

    // check if any row mate has the same digit
    for (int mate_x = 0; mate_x < 9; mate_x++) {
        // skip self
        if (mate_x == x) {
            continue;
        }
        //
        int mate_y = y;
        int mate_digt = digits[mate_y][mate_x];
        // mate has the same digit
        if (mate_digt == digt) {
            // export mate index
            if (conflict_i != NULL) {
                int mate_i = xytoi(mate_y, mate_x);
                *conflict_i = mate_i;
            }
            // return digit is disable
            return false;
        }
    }
    // return digit is enable
    return true;
};
bool Sudoku::checkConflict_col(int indx, int digt, int *conflict_i) {
    //
    limitNumbRange(digt, 0, 9);
    if (digt == 0) {
        return true;
    }

    // get x y
    int y, x;
    itoxy(y, x, indx);

    // check if any mate has the same digit
    for (int mate_y = 0; mate_y < 9; mate_y++) {
        // skip self
        if (mate_y == y) {
            continue;
        }
        //
        int mate_x = x;
        int mate_digt = digits[mate_y][mate_x];
        // mate has the same digit
        if (mate_digt == digt) {
            // export mate index
            if (conflict_i != NULL) {
                int mate_i = xytoi(mate_y, mate_x);
                *conflict_i = mate_i;
            }
            // return digit is disable
            return false;
        }
    }
    // return digit is enable
    return true;
};
bool Sudoku::checkConflict_blo(int indx, int digt, int *conflict_i) {
    //
    limitNumbRange(digt, 0, 9);
    if (digt == 0) {
        return true;
    }

    // get b bi
    int b, bi;
    itobbi(b, bi, indx);

    // check if any mate has the same digit
    for (int mate_bi = 0; mate_bi < 9; mate_bi++) {
        // skip self
        if (mate_bi == bi) {
            continue;
        }
        //
        int mate_b = b;
        int mate_i;
        mate_i = btoi(mate_b, mate_bi);
        int mate_y, mate_x;
        itoxy(mate_y, mate_x, mate_i);
        int mate_digt = digits[mate_y][mate_x];
        // mate has the same digit
        if (mate_digt == digt) {
            // export mate index
            if (conflict_i != NULL) {
                *conflict_i = mate_i;
            }
            // return digit is disable
            return false;
        }
    }
    // return digit is enable
    return true;
};
bool Sudoku::setConflict(int indx_self, int digt, int indx_conf) {
    if (indx_self == indx_conf) {
        return false;
    }
    // get position
    int self_x, self_y, self_b, self_bi;
    itoxy(self_y, self_x, indx_self);
    itobbi(self_b, self_bi, indx_self);
    int conf_x, conf_y, conf_b, conf_bi;
    itoxy(conf_y, conf_x, indx_conf);
    itobbi(conf_b, conf_bi, indx_conf);

    // mark conflict
    int range;
    bool marked = false;
    auto mark = [&]() {
        marked = true;
        conflicts[self_y][self_x][digt - 1][0] = flag_conf_disableDigit;
        conflicts[self_y][self_x][digt - 1][range] = indx_conf;
    };
    if (self_y == conf_y) {
        range = 1;
        mark();
    }
    if (self_x == conf_x) {
        range = 2;
        mark();
    }
    if (self_b == conf_b) {
        range = 3;
        mark();
    }

    // return true when marked any conflict
    // return false when self-cell and conf-cell has none same range
    return marked;
};
void Sudoku::markMatesConflicts(int indx_self, int digt) {
    if (digt == 0) {
        clrMatesConflicts(indx_self);
        return;
    }
    markMatesConflict_row(indx_self, digt);
    markMatesConflict_col(indx_self, digt);
    markMatesConflict_blo(indx_self, digt);
};
void Sudoku::markMatesConflict_row(int indx_self, int digt) {
    //
    int self_x, self_y, self_i;
    self_i = indx_self;
    itoxy(self_y, self_x, indx_self);

    //
    int mate_y = self_y;
    for (int mate_x = 0; mate_x < 9; mate_x++) {
        // skip cell-self
        if (mate_x == self_x) {
            continue;
        }
        // mark conflicts
        int mate_i = xytoi(mate_y, mate_x);
        setConflict(mate_i, digt, self_i);
    }
};
void Sudoku::markMatesConflict_col(int indx_self, int digt) {
    //
    int self_x, self_y, self_i;
    self_i = indx_self;
    itoxy(self_y, self_x, indx_self);

    //
    int mate_x = self_x;
    for (int mate_y = 0; mate_y < 9; mate_y++) {
        // skip cell-self
        if (mate_y == self_y) {
            continue;
        }
        // mark conflicts
        int mate_i = xytoi(mate_y, mate_x);
        setConflict(mate_i, digt, self_i);
    }
};
void Sudoku::markMatesConflict_blo(int indx_self, int digt) {
    //
    int self_b, self_bi, self_i;
    self_i = indx_self;
    itobbi(self_b, self_bi, indx_self);

    //
    int mate_b = self_b;
    for (int mate_bi = 0; mate_bi < 9; mate_bi++) {
        // skip cell-self
        if (mate_bi == self_bi) {
            continue;
        }
        // mark conflicts
        int mate_i = btoi(mate_b, mate_bi);
        setConflict(mate_i, digt, self_i);
    }
};
void Sudoku::clrMatesConflicts(int indx) {
    // none conflict mates
    int d = getDigit(indx);
    if (d == 0) {
        return;
    }

    // get cell-self position
    int self_i = indx;
    int self_x, self_y, self_b, self_bi;
    itoxy(self_y, self_x, self_i);
    itobbi(self_b, self_bi, self_i);

    // clean
    int r;
    int di = d - 1;
    // row
    r = 1;
    for (int mate_x = 0; mate_x < 9; mate_x++) {
        // skip cell-self
        if (mate_x == self_x) {
            continue;
        }
        // clean mates conflicts
        int mate_y = self_y;
        if (self_i == getRowConflict(mate_y, mate_x, d)) {
            conflicts[mate_y][mate_x][di][r] = -1;
        }
        // amend conflict flag
        if ((getColConflict(mate_y, mate_x, d) == -1) &&
            (getBloConflict(mate_y, mate_x, d) == -1)) {
            conflicts[mate_y][mate_x][di][0] = flag_conf_enableDigit;
        }
    }
    // col
    r = 2;
    for (int mate_y = 0; mate_y < 9; mate_y++) {
        // skip cell-self
        if (mate_y == self_y) {
            continue;
        }
        // clean mates conflicts
        int mate_x = self_x;
        if (self_i == getColConflict(mate_y, mate_x, d)) {
            conflicts[mate_y][mate_x][di][r] = -1;
        }
        // amend conflict flag
        if ((getRowConflict(mate_y, mate_x, d) == -1) &&
            (getBloConflict(mate_y, mate_x, d) == -1)) {
            conflicts[mate_y][mate_x][di][0] = flag_conf_enableDigit;
        }
    }
    // blo
    r = 3;
    for (int mate_bi = 0; mate_bi < 9; mate_bi++) {
        // skip cell-self
        if (mate_bi == self_bi) {
            continue;
        }
        // clean mates conflicts
        int mate_b = self_b;
        int mate_i = btoi(mate_b, mate_bi);
        int mate_y, mate_x;
        itoxy(mate_y, mate_x, mate_i);
        if (self_i == getBloConflict(mate_y, mate_x, d)) {
            conflicts[mate_y][mate_x][di][r] = -1;
        }
        // amend conflict flag
        if ((getColConflict(mate_y, mate_x, d) == -1) &&
            (getRowConflict(mate_y, mate_x, d) == -1)) {
            conflicts[mate_y][mate_x][di][0] = flag_conf_enableDigit;
        }
    }
};
bool Sudoku::checkDigit_byMarks(int indx, int digt) {
    // get position
    int x, y;
    itoxy(y, x, indx);

    // check if this digit marked by any mate
    if (flag_conf_disableDigit == conflicts[y][x][digt - 1][0]) {
        // someone mate has marked this digit
        // so this digit is disable for this cell
        return false;
    } else {
        // nobody marked this digit
        // this digit is enable for this cell
        return true;
    }
};
bool Sudoku::commitDigit(int indx, int digt, bool check) {
    // get position
    int d, i, x, y;
    i = indx;
    d = digt;
    itoxy(y, x, i);

    // check conflict that any mate is this digit
    if (false == checkConflict(indx, d)) {
        return false;
    }

    // commit digit
    digits[y][x] = d;

    // mark mates conflicts
    if (d != 0) {
        markMatesConflicts(i, d);
    }

    //
    return true;
};
int Sudoku::seekDigit_destiny(int indx) {
    // 天选之子-别的格子都填不了这个数字

    // get cell-self position
    int self_x, self_y, self_i;
    self_i = indx;
    itoxy(self_y, self_x, self_i);
    int self_b, self_bi;
    itobbi(self_b, self_bi, self_i);

    // check if the digit is destiny in the row
    auto destinyDigitInRow = [this, &self_x, &self_y](int d) {
        bool im_destiny = true;
        for (int mate_x = 0; mate_x < 9; mate_x++) {
            // skip cell-self
            if (mate_x == self_x) {
                continue;
            }
            // get cell-mate
            int mate_y = self_y;
            int mate_i = xytoi(mate_y, mate_x);
            // if digit is enable for cell-mate then cell-self isnt destiny
            if (true == checkDigit_byMarks(mate_i, d)) {
                im_destiny = false;
                break;
            }
        }
        // return
        if (im_destiny == true) {
            return d;
        } else {
            return -1;
        }
    };
    // check if the digit is destiny in the col
    auto destinyDigitInCol = [this, &self_x, &self_y](int d) {
        bool im_destiny = true;
        for (int mate_y = 0; mate_y < 9; mate_y++) {
            // skip cell-self
            if (mate_y == self_y) {
                continue;
            }
            //
            int mate_x = self_x;
            int mate_i = xytoi(mate_y, mate_x);
            if (true == checkDigit_byMarks(mate_i, d)) {
                im_destiny = false;
                break;
            }
        }
        if (im_destiny == true) {
            return d;
        } else {
            return -1;
        }
    };
    // check if the digit is destiny in the blo
    auto destinyDigitInBlo = [this, &self_b, &self_bi](int d) {
        bool im_destiny = true;
        for (int mate_bi = 0; mate_bi < 9; mate_bi++) {
            // skip cell-self
            if (mate_bi == self_bi) {
                continue;
            }
            //
            int mate_b = self_b;
            int mate_i = btoi(mate_b, mate_bi);
            if (true == checkDigit_byMarks(mate_i, d)) {
                im_destiny = false;
                break;
            }
        }
        if (im_destiny == true) {
            return d;
        } else {
            return -1;
        }
    };
    // seek destiny digit
    for (int d = 1; d <= 9; d++) {
        // skip disable digits
        if (false == isDigitEnable(self_i, d)) {
            continue;
        }

        // seek
        int destiny;
        // seek row digit
        destiny = destinyDigitInRow(d);
        if (destiny != -1) {
            return destiny;
        }
        // seek col digit
        destiny = destinyDigitInCol(d);
        if (destiny != -1) {
            return destiny;
        }
        // seek blo digit
        destiny = destinyDigitInBlo(d);
        if (destiny != -1) {
            return destiny;
        }
    }

    // none sought
    return -1;
};
int Sudoku::seekDigit_fate(int indx) {
    // 逼上梁山-只剩下这个数字可以填

    // get position
    int x, y, i;
    i = indx;
    itoxy(y, x, i);

    // seek fate digit
    int fate = -1;
    for (int d = 1; d <= 9; d++) {
        if (true == isDigitEnable(i, d)) {
            if (fate == -1) {
                // record the first enable digit
                fate = d;
            } else {
                // find the second enable digit
                // means no fate digit
                fate = -1;
                break;
            }
        }
    }

    // return
    return fate;
};
int Sudoku::confirmDigits() {
    // commit all ascertainable digits and save answer if all digits was comitted

    return (confirmDigits(true));
};
int Sudoku::confirmDigits(bool save_answer_if_succeed) {
    // confirm all certain digits of sudoku and commit them all

    // var
    int committed = 0;
    bool new_commit = false;
    bool bad_sudoku = false;

    // crack sudoku
CRACK_AGAIN:
    // printf("\n\nCrack Again\n");
    for (int i = 0; i < 81; i++) {
        // skip committed cells
        int d = getDigit(i);
        if (d != 0) {
            // check if this committed digit is wrong
            // (maybe modified by 3rd, cos this app will not commit wrong digits)
            /* if (false == checkConflict(i, d)) {
                bad_sudoku = true;
                break;
            } */
            // printPanel();
            committed++;
            continue;
        }

        // check if the sudoku is bad
        if (true == isBadCell(i)) {
            bad_sudoku = true;
            break;
        }

        // seek digit
        auto commit = [&]() {
            bool b = commitDigit(i, d, true);
            committed++;
            new_commit = true;
        };

        // fate
        d = seekDigit_fate(i);
        if (d != -1) {
            commit();
            continue;
        }

        // destiny
        d = seekDigit_destiny(i);
        if (d != -1) {
            commit();
            continue;
        }
    }
    // crack again when new digit has been committed and the sudoku is not bad
    if ((new_commit == true) && (bad_sudoku == false)) {
        new_commit = false;
        committed = 0;
        goto CRACK_AGAIN;
    }

    if (bad_sudoku == true) {
        return flag_crack_bad;
    }
    if (committed == 81) { // every cell has been committed
        if (true == save_answer_if_succeed) {
            addAnswer();
        }
        return flag_crack_success;
    } else { // some cells dont commit
        return flag_crack_unsure;
    }
};
int Sudoku::confirmDigits_try() {
    // write nothing to the sudoku
    // just get the return
    // and without saving answer if succeed

    // backup
    char bp[82];
    getString(bp);
    // crack
    int rtn = confirmDigits(false);
    // recover
    readin(bp);
    // return
    return rtn;
};
bool Sudoku::seekDigits_zombie(bool is_initial_call, bool save_answers, bool print_answer_immediately) {
    // 行尸走肉-逐个尝试所有可用的数字

    // used var
    static int LEVEL;                // the index of recursion, `0` is the initial-call
    static int TRY;                  // the index of try
    static unsigned long long COUNT; // the count of answers
    char *bp_initial;                // sudoku before initial-call
    int i = -1;                      // the cell will be tried to commit
    int d = 1;                       // the digit will be tried
    // get the enable digits of the first uncommitted cell one-by-one
    auto getNextEnableDigit = [&]() {
        // initial time to get the first uncommitted cell index
        if (i == -1) {
            for (i = 0; i < 81; i++) {
                // skip committed cells
                d = getDigit(i);
                if (d == 0) {
                    break;
                }
            }
            // initial digit
            d = 0;
        }
        // get the next enable digit
        d++;
        for (; d <= 9; d++) {
            // skip disable digits
            if (true == isDigitEnable(i, d)) {
                return;
            }
        }
        // no more enable digit
        d = -1;
    };
    auto DEBUG_zombie_crack_result = [](int rtn_crack) {
        CHECK_DEBUG;
        char succeed[] = "succeed ^_^";
        char bad[] = "bad -_-";
        char unsure[] = "unsure O_O";
        char *s;
        switch (rtn_crack) {
            case flag_crack_success :
                s = succeed;
                break;
            case flag_crack_bad :
                s = bad;
                break;
            case flag_crack_unsure :
                s = unsure;
                break;
        }
        //[LEVEL:TRY]
        printf("\n"
               "@@ @zombie_crack@ [%d:%d] \n"
               "@@:%s \n"
               "\n",
               LEVEL, TRY, s);
    };
    auto DEBUG_zombie_committing_digit = [this](int i, int d) {
        CHECK_DEBUG;
        int x, y;
        itoxy(y, x, i);
        char dbg[82];
        getString(dbg);
        dbg[i + 1] = 0;
        //[i;y:x;d]
        printf("\n"
               "@@ @zombie_commit@ [%d;%d:%d;%d] \n"
               "@@:",
               i, y, x, d);
        printLine(dbg);
        printf("\n");
    };
    auto DEBUG_zombie_result = [](int c_now, int c_all, int c_rept) {
        CHECK_DEBUG;
        //[now;all:rept]
        printf("\n"
               "@@ @zombie_result@ [%d;%d:%d] \n"
               "\n",
               c_now, c_all, c_rept);
    };
    auto printAnswerImmediately = [this, &print_answer_immediately]() {
        if (true == print_answer_immediately) {
            // copy sudoku
            char str[82];
            this->getString(str);
            Sudoku copy(str);
            // commit all certain digits
            bool b = copy.confirmDigits(false);
            printf("[%llu] \n", COUNT);
            copy.printPanel();
        }
    };
    auto rtn__ = [&](bool r) {
        // final-return
        if (LEVEL == 0) {
            // recover sudoku
            readin(bp_initial);
            // delete repeated answers
            int c_all, c_now, c_repeat;
            c_repeat = cleanAnswers(c_all, c_now);
            DEBUG_zombie_result(c_now, c_all, c_repeat);
            // pre-return
            if (r == true) {
                ;
            } else {
                ;
            }
        }
        // recover recursion level
        LEVEL--;
        // return
        return r;
    };
    auto rtn__true = [&]() {
        return rtn__(true);
    };
    auto rtn__false = [&]() {
        return rtn__(false);
    };

    // initial-call
    if (is_initial_call == true) {
        // set default var
        LEVEL = -1;
        TRY = -1;
        COUNT = 0;
        // clear previous answers
        clrAnswers();
        // backup the entry-time sudoku
        bp_initial = new char[82];
        getString(bp_initial);
    }

    // update recursion level
    LEVEL++;
    // crack
    int bingo = confirmDigits(save_answers);
    DEBUG_zombie_crack_result(bingo);
    // switch crack result
    switch (bingo) {
        case flag_crack_success :
            COUNT++;
            printAnswerImmediately();
            return rtn__true();
            break;
        case flag_crack_bad :
            return rtn__false();
            break;
        case flag_crack_unsure :
            // assume zombie-on failed
            bool zombie_secceed = false;
            // zombie-one, try to commit each enable digit
            while (true) {
                // get one digit
                getNextEnableDigit();
                // no more enable digit
                if (d == -1) {
                    break;
                }
                // backup sudoku before commit
                char bp_PreCommit[82];
                getString(bp_PreCommit);
                // commit this digit
                TRY++;
                commitDigit(i, d, true);
                DEBUG_zombie_committing_digit(i, d);
                // try to crack
                bool b = seekDigits_zombie(false, save_answers, print_answer_immediately);
                // this digit succeed
                if (b == true) {
                    // mark zombie-on succeed
                    zombie_secceed = true;
                }
                // revert sudoku to before the commit
                readin(bp_PreCommit);
            }
            // return
            return rtn__(zombie_secceed);
            break;
    }
    return false;
};
int Sudoku::crack(bool save_answers, bool print_answer_line_immediately) {
    seekDigits_zombie(true, save_answers, print_answer_line_immediately);
    return answer_count;
};
int Sudoku::crack(bool save_answers) {
    return (crack(save_answers, false));
};
void Sudoku::addAnswer() {
    answer_count++;
    // alloc answers table mem
    void *p = realloc(answers, answer_count * sizeof(void *));
    if (p == NULL) {
        //== .
        return;
    }
    answers = (char **)p;

    // save answer
    char *ans = new char[82];
    getString(ans);
    answers[answer_count - 1] = ans;
}
void Sudoku::delAnswer(int i_) {
    // free answer mem
    delete[] answers[i_];
    // reorder answers table
    for (int i = i_ + 1; i < answer_count; i++) {
        answers[i - 1] = answers[i];
    }
    // update count
    answer_count--;
    // free table mem
    void *p = realloc(answers, answer_count * sizeof(void *));
    if (p == NULL) {
        //== -
        return;
    }
    answers = (char **)p;
}
int Sudoku::cleanAnswers() {
    // delete all duplicate answers

    //
    int rept_cnt = 0;
    for (int a = 0; a < (answer_count - 1); a++) {
        for (int b = (a + 1); b < answer_count; b++) {
            if (0 == strcmp(answers[a], answers[b])) {
                delAnswer(b);
                b--;
                rept_cnt++;
            }
        }
    }
    //
    return rept_cnt;
}
int Sudoku::cleanAnswers(int &before, int &after) {
    before = answer_count;
    int rtn = cleanAnswers();
    after = answer_count;
    return rtn;
}
void Sudoku::clrAnswers() {
    //
    for (int i = 0; i < answer_count; i++) {
        delete[] answers[i];
    }
    //
    free(answers);
    //
    answer_count = 0;
}
void Sudoku::getString(char *str) {
    for (int i = 0; i < 81; i++) {
        str[i] = getDigit(i) + '0';
    }
    str[81] = '\0';
}
void Sudoku::printLineTo(char *user_buffer, char *str, char *ch_uncommitted_digit) {
    // print sudoku string in one line to the buffer

    // the buffer where will be printed to
    char output[150] = {0};
    char *o = output;
    auto insertOne = [&o](char ch) {
        *o = ch;
        o++;
    };

    // set the char of uncommitted cell
    char ch_anyDigit = ' ';
    if (ch_uncommitted_digit != NULL) {
        ch_anyDigit = *ch_uncommitted_digit;
    }

    // print the digits
    auto print_ = [&]() {
        char *pch = str;
        auto printThreeCells = [&]() {
            bool is_str_end = false;
            int i;
            // print 3 cells of one row of blo
            for (i = 0; i < 3; i++) {
                // get the char to print
                char ch = *pch;
                if ((ch == '0') || (ch == ch_anyDigit)) {
                    ch = '.';
                }
                // print this char
                insertOne(ch);
                // check if the next char is \0
                pch++;
                if (*pch == '\0') {
                    is_str_end = true;
                    break;
                }
            }
            // print the right blo-space when all 3 cells have been printed
            if (i == 3) {
                insertOne(' ');
            }
            // reutrn
            return is_str_end;
        };
        auto printOneLine = [&]() {
            bool is_str_end = false;
            int i;
            // print 3 blo-rows of one line
            for (i = 0; i < 3; i++) {
                // print one blo-row
                is_str_end = printThreeCells();
                // stop print when the next char is \0
                if (is_str_end == true) {
                    break;
                }
            }
            // print the right border of the line when all 3 blo-rows have been printed
            if (i == 3) {
                insertOne('|');
                insertOne(' ');
            }
            return is_str_end;
        };
        // the str is empty
        if (*pch == '\0') {
            return;
        }
        // print all
        for (; true;) {
            bool is_end = printOneLine();
            if (is_end == true) {
                break;
            }
        }
    };
    print_();

    // print new line and \0
    insertOne('\n');
    insertOne('\0');

    // std-printf or export to user-buffer
    if (user_buffer == NULL) {
        printf("%s", output);
    } else {
        strcpy(user_buffer, output);
    }
}
void Sudoku::printLineTo(char *user_buffer, char *str) {
    printLineTo(user_buffer, str, &ch_anyDigit);
};
void Sudoku::printLineTo(char *user_buffer) {
    char str[82];
    printLineTo(user_buffer, str, &ch_anyDigit);
};

void Sudoku::printLine(char *str) {
    printLineTo(NULL, str);
};
void Sudoku::printLine() {
    printLineTo(NULL);
}

void Sudoku::printPanelTo(char *user_buffer, char *str, int i_cur) {
    // print sudoku string in one panel to the buffer

    /*
      + --------- +-----------+-----------+
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      + --------- +---------- + --------- +
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      + --------- + --------- + --------- +
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      + --------- + --------- + --------- +

      *-----------*-----------*-----------*
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      *-----------*-----------*-----------*
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      *-----------*-----------*-----------*
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      *-----------*-----------*-----------*

      +-----------+-----------+-----------+
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      +-----------+-----------+-----------+
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      +-----------+-----------+-----------+
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      | 1   2   3 | 4   5   6 | 7   8   9 |
      +-----------+-----------+-----------+

      +-----------+-----------+-----------+
      |  1  2  3  |  4  5  6  |  7  8  9  |
      |  1  2  3  |  4  5  6  |  7  8  9  |
      |  1  2  3  |  4  5  6  |  7  8  9  |
      +-----------+-----------+-----------+
      |  1  2  3  |  4  5  6  |  7  8  9  |
      |  1  2  3  |  4  5  6  |  7  8  9  |
      |  1  2  3  |  4  5  6  |  7  8  9  |
      +-----------+-----------+-----------+
      |  1  2  3  |  4  5  6  |  7  8  9  |
      |  1  2  3  |  4  5  6  |  7  8  9  |
      |  1  2  3  |  4  5  6  |  7  8  9  |
      +-----------+-----------+-----------+

    char temp[] = ""
                  "+-----------+-----------+-----------+ \n"
                  "|  1  2  3  |  4  5  6  |  7  8  9  | \n"
                  "|  1  2  3  |  4  5  6  |  7  8  9  | \n"
                  "|  1  2  3  |  4  5  6  |  7  8  9  | \n"
                  "+-----------+-----------+-----------+ \n"
                  "|  1  2  3  |  4  5  6  |  7  8  9  | \n"
                  "|  1  2  3  |  4  5  6  |  7  8  9  | \n"
                  "|  1  2  3  |  4  5  6  |  7  8  9  | \n"
                  "+-----------+-----------+-----------+ \n"
                  "|  1  2  3  |  4  5  6  |  7  8  9  | \n"
                  "|  1  2  3  |  4  5  6  |  7  8  9  | \n"
                  "|  1  2  3  |  4  5  6  |  7  8  9  | \n"
                  "+-----------+-----------+-----------+ \n"
                  ""; // (| :2) ( n :3)*3 ( | :3) ( n :3)*3 ( | :3) ( n :3)*3 ( | :3) (\n:1) :2+3*4*3+1=39
    */
    // char charOrder_assicPrintable[] = " !\" #$ % &'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    // char charOrder_keyboard[] = " `1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?";

    // the buffer where will be printed to
    char output[530] = {0};
    char *o = output;
    auto insertOne = [&o](char ch) {
        *o = ch;
        o++;
    };

    // the chars to highlight the gazed cell
    //           "0123456789"
    int gi = 1;
    char gl[] = "<[(>_*|-=~";
    char gr[] = ">])<_*|-=~";
    char ch_gaze_l = gl[gi];
    char ch_gaze_r = gr[gi];

    // the chars to draw the panel
    char ch_space = ' ';
    char ch_vertical = '|';
    char ch_horizon = '-';
    char ch_corner = '+';
    char ch_unsure_digit = ' ';

    // print the sudoku panel
    int i = 0;
    char ch_1, ch_2, ch_3;
    // print 13 lines
    for (int l13 = 0; l13 < 13; l13++) {
        // confirm if the line is border
        bool border_line = false;
        if (l13 % 4 == 0) {
            border_line = true;
        }

        // print left border of the line
        ch_1 = ch_vertical;
        if (border_line == true) {
            ch_1 = ch_corner;
        }
        insertOne(' ');
        insertOne(ch_1);

        // print 3 blos of one line
        for (int b3 = 0; b3 < 3; b3++) {
            // print left border of one blo
            ch_1 = ch_space;
            if (border_line == true) {
                ch_1 = ch_horizon;
            }
            insertOne(ch_1);

            //  print 3 cells of the blo
            for (int c3 = 0; c3 < 3; c3++) {
                if (border_line == true) {
                    ch_1 = ch_horizon;
                    ch_2 = ch_horizon;
                    ch_3 = ch_horizon;
                } else {
                    ch_1 = ch_space;
                    ch_3 = ch_space;
                    if (i == i_cur) {
                        ch_1 = ch_gaze_l;
                        ch_3 = ch_gaze_r;
                    }
                    ch_2 = str[i];
                    if (ch_2 == '0') {
                        ch_2 = ch_unsure_digit;
                    }
                    i++;
                }
                insertOne(ch_1);
                insertOne(ch_2);
                insertOne(ch_3);
            }
            // print right border of the blo
            ch_1 = ch_space;
            ch_2 = ch_vertical;
            if (border_line == true) {
                ch_1 = ch_horizon;
                ch_2 = ch_corner;
            }
            insertOne(ch_1);
            insertOne(ch_2);
        }
        // print one line finished
        insertOne(' ');
        insertOne('\n');
        insertOne('\0');
    }

    // return
    if (user_buffer == NULL) {
        printf("%s", output);
    } else {
        strcpy(user_buffer, output);
    }
}
void Sudoku::printPanelTo(char *user_buffer, char *str) {
    printPanelTo(user_buffer, str, -1);
};
void Sudoku::printPanelTo(char *user_buffer, int i_cur) {
    char str[82];
    this->getString(str);
    printPanelTo(user_buffer, str, i_cur);
};
void Sudoku::printPanelTo(char *user_buffer) {
    char str[82];
    this->getString(str);
    printPanelTo(user_buffer, str, -1);
};
void Sudoku::printPanel(char *str, int i_cur) {
    printPanelTo(NULL, str, i_cur);
};
void Sudoku::printPanel(int i_cur) {
    //
    char str[82];
    getString(str);
    //
    printPanelTo(NULL, str, i_cur);
};
void Sudoku::printPanel() {
    //
    char str[82];
    getString(str);
    //
    printPanelTo(NULL, str, -1);
};
void Sudoku::printAnswer(int i, bool panel_or_line) {
    //
    if (answer_count == 0) {
        printf("No Answer Data. \n");
        return;
    }
    if (i < 0) {
        i = 0;
    }
    if (i >= answer_count) {
        i = answer_count - 1;
    }
    //
    if (true == panel_or_line) {
        printPanel(answers[i], -1);
    } else {
        printLine(answers[i]);
    }
}
void Sudoku::printAnswer_panel(int i) {
    printAnswer(i, true);
};
void Sudoku::printAnswer_line(int i) {
    printAnswer(i, false);
};
void Sudoku::printAllAnswers(bool panel_or_line) {
    for (int i = 0; i < answer_count; i++) {
        printf("Answers[%d] \n", i);
        printAnswer(i, panel_or_line);
    }
};
void Sudoku::printAllAnswers_panel() {
    printAllAnswers(true);
};
void Sudoku::printAllAnswers_line() {
    printAllAnswers(false);
};
const char *Sudoku::getAnswerString(int i) {
    if (i >= answer_count) {
        return NULL;
    }
    return answers[i];
};
const char **Sudoku::getAnswerStringArray(int &c) {
    if (answer_count == 0) {
        return NULL;
    }
    c = answer_count;
    return ((const char **)answers);
};

Sudoku::Sudoku() {
    // initialize conflicts
    initConflicts();
    //
}
Sudoku::Sudoku(char *str)
    : Sudoku() {
    readin(str);
}

//-- main

void printAllSudoku() {
    //
    char *s_ = new char[82]();
    Sudoku empty_sudoku(s_);

    //
    time_t t1 = time(NULL);
    empty_sudoku.crack(false, true);
    time_t t2 = time(NULL);

    //
    unsigned long t = t2 - t1;
    printf("%lu\n%lu\n%lu\n", t1, t2, t);
}

int main(int argc, char *argv[]) {
    // used to debug this class named sudoku during development

    //
    printAllSudoku();
    return 0;

    //
    char tests[20][256] = {
        "85----2--------48-23--4---63--7------2-9-4-5---9-31----"
        "-7----9-----8---5---2---6-",
        "85----2---------8-23--4---6---7--------9-4-5---9--1----"
        "-7----9-----8---5---2---6-",
        "1-----7-9----3-5-1----9-----287---------62----9-1----"
        "35-7--------2--4----4-3-927-",
        " --------- --------- --------- --------- --------- "
        "--------- --------- --------- --------- ",
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

    //
    Sudoku *sdk = new Sudoku();
    bool b = sdk->readin(tests[3]);

    //
    if (b == false) {
        printf(">> Readin falied! \n");
        return -1;
    }
    printf(">> Readin succeed:");
    sdk->printLine();
    sdk->printPanel();

    //
    printf(">> Cracking.......... \n");
    int i = sdk->crack(false, true);
    sdk->printAllAnswers_line();

    //
    system("clear");
    const char *ans1 = sdk->getAnswerString(0);
    sdk->readin(ans1);
    sdk->printPanel(3);

    //
    return 0;
}
