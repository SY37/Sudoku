
#include <cstddef>

class CSudoku;

typedef void (*PFvoid_pSudoku)(CSudoku *);

class CSudoku {
private:
    enum EnumDigitConflictStatus {
        DIGIT_ENABLED = 0,
        DIGIT_DISABLED = 1,
    };
    enum EnumCellRange {
        RANGE_ROW = 1,
        RANGE_COL = 2,
        RANGE_BLO = 3,
    };
    enum EnumHowResult {
        CRACK_RESULT_SAVE = 1,
        CRACK_RESULT_PRINT = 0,
        CRACK_RESULT_COUNT = -1,
    };
    //-- -
    int digits[9][9] = {0};
    int getDigit(int i) {
        return digits[i / 9][i % 9];
    };
    void setDigit(int i, int d) {
        digits[i / 9][i % 9] = d;
    };
    int seekDigit(int cell_index);

    static inline int i2y(int i) {
        return (i / 9);
    };
    static inline int i2x(int i) {
        return (i % 9);
    };

    static inline int i2B(int i) {
        int x = i2x(i);
        int y = i2y(i);

        int bx = x / 3;
        int by = y / 3;
        int b = by * 3 + bx;

        return b;
    };
    static inline int i2bi(int i) {
        int x = i2x(i);
        int y = i2y(i);

        int bx = x / 3;
        int by = y / 3;

        int b_inside_x = x - bx * 3;
        int b_inside_y = y - by * 3;
        int bi = b_inside_y * 3 + b_inside_x;

        return bi;
    };
    static inline void i2bbi(int &b, int &bi, int i) {
        int x = i2x(i);
        int y = i2y(i);

        int bx = x / 3;
        int by = y / 3;
        b = by * 3 + bx;

        int b_inside_x = x - bx * 3;
        int b_inside_y = y - by * 3;
        bi = b_inside_y * 3 + b_inside_x;
    };
    static inline int b2i(int b, int bi) {

        // get block x,y position in sudoku
        int bx = b % 3;
        int by = b / 3;

        // get cell x,y position in block
        int b_inside_x = bi % 3;
        int b_inside_y = bi / 3;

        // get cell x,y,i position in sudoku
        int x = bx * 3 + b_inside_x;
        int y = by * 3 + b_inside_y;
        int i = xy2i(y, x);

        // return
        return i;
    };
    void limitXY(int &y, int &x) {
        if (y < 0) {
            y = 0;
        }
        if (x < 0) {
            x = 0;
        }

        if (y > 8) {
            y = 8;
        }
        if (x > 8) {
            x = 8;
        }
    }
    //-- -
    int conflicts[9][9][9][4]; //[y][x][digit][range:0-flag;1-row;2-col;3-blo]
    void initConflicts();
    void setStatus(int i, int d, int r, int v) {
        int y, x;
        i2xy(y, x, i);
        conflicts[y][x][d - 1][r] = v;
    };
    int getStatus(int i, int d, int r) {
        int y, x;
        i2xy(y, x, i);
        return (conflicts[y][x][d - 1][r]);
    }
    bool isDigitEnabled(int indx, int digt) {
        // check if this digit is enabled for this cell

        return (DIGIT_ENABLED == getStatus(indx, digt, 0));
    };
    void commitCell(int cell_index, int digit_to_commit);

    int rangeFirst(int i, int range) {
        switch (range) {
            case RANGE_ROW :
                // the index of the 1st cell in the row
                return (i / 9 * 9);
                break;
            case RANGE_COL :
                // the index of the 1st cell in the col
                return i % 9;
                break;
            case RANGE_BLO :
                // the index of the 1st cell in the blo
                return (((i / 9) / 3 * 3) * 9 + ((i % 9) / 3 * 3));
                break;
        }
        return -1;
    };
    int inRange(int first_i, int n, int range) {
        switch (range) {
            case RANGE_ROW :
                // the index of the nth cell in the row
                return (first_i + n);
                break;
            case RANGE_COL :
                // the index of the nth cell in the col
                return (first_i + (9 * n));
                break;
            case RANGE_BLO :
                // the index of the nth cell in the blo
                return (first_i + ((n / 3) * 9 + (n % 3)));
                break;
        }
        return -1;
    };
    void clrOrMarkMates(int indx, bool clrOrMark, int prev_digit);
    void markMatesStatus(int indx) {
        clrOrMarkMates(indx, false, 123);
    };
    void clrMatesStatus(int indx, int prev_digit) {
        clrOrMarkMates(indx, true, prev_digit);
    };
    //-- -
    PFvoid_pSudoku __whenCrackBingo = NULL;
    static void __whenCrackBingo_(CSudoku *this_) {
        ;
    }
    bool crack(bool init_recursion);

    void toPrettyPanel(char *buffer, int gazed_index, int gazed_style);

public:
    CSudoku();
    CSudoku(char *str);
    static inline void i2xy(int &y, int &x, int i) {
        y = i2y(i);
        x = i2x(i);
    };
    static inline int xy2i(int y, int x) {
        return (y * 9 + x);
    };
    bool readin(const char *sudoku_str);
    int getDigit(int y, int x) {
        limitXY(y, x);
        return digits[y][x];
    };
    bool isDigitOK(int y, int x, int d) {
        // check if this digit is enabled for this cell

        limitXY(y, x);
        int i = xy2i(y, x);
        return isDigitEnabled(i, d);
    };
    int certainDigit(int y, int x) {
        // get the certain digit of one cell

        limitXY(y, x);
        int i = xy2i(y, x);
        return seekDigit(i);
    }
    void commitDigit(int y, int x, int d) {
        limitXY(y, x);
        int i = xy2i(y, x);
        commitCell(i, d);
    };
    enum EnumSudokuStatus {
        SUDOKU_BAD = -1,
        SUDOKU_UNSURE = 0,
        SUDOKU_BINGO = 1,
    };
    EnumSudokuStatus commitAllCertains();
    bool crack() {
        return crack(true);
    }
    void setCall_CrackBingo(PFvoid_pSudoku pfunc) {
        __whenCrackBingo = pfunc;
    }

    static const int SIZE_STRING = 82;
    void toString(char *str);
    static const int SIZE_LINE = (4 * 3 + 2) * 9;
    void toPrettyLine(char *buffer);
    static const int SIZE_PANEL = (3 * (4 * 3 + 1) + 1) * 13 + 1;
    void toPrettyPanel(char *buffer, int gazed_y, int gazed_x, int gazed_style) {

        int y = gazed_y;
        int x = gazed_x;

        int i;
        if ((y < 0) || (x < 0)) {
            i = -1;
        } else {
            limitXY(y, x);
            i = xy2i(gazed_y, gazed_x);
        }

        toPrettyPanel(buffer, i, gazed_style);
    };
};
