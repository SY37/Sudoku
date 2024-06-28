

#include <cstddef>
#include <cstring>
#define flag_conf_enableDigit 0
#define flag_conf_disableDigit 1
#define flag_crack_success 1
#define flag_crack_bad -1
#define flag_crack_unsure 0

class Sudoku {
    char ch_anyDigit = '-';

public:
    int digits[9][9] = {0};
    int conflicts[9][9][9][4]; //[y][x][digit][range:0-flag;1-row;2-col;3-blo]
    char string[82] = {0};

    char **answers = NULL;

    unsigned long long answer_count = 0;
    unsigned long long ans_c[2] = {0};

    // inline
    int itoy(int i) {
        return (i / 9);
    };
    int itox(int i) {
        return (i % 9);
    };
    void itoxy(int &y, int &x, int i) {
        y = itoy(i);
        x = itox(i);
    };
    int xytoi(int y, int x) {
        return (y * 9 + x);
    };
    void xytoi(int &i, int y, int x) {
        i = xytoi(y, x);
    };
    int itob(int i) {
        int x = itox(i);
        int y = itoy(i);

        int bx = x / 3;
        int by = y / 3;
        int b = by * 3 + bx;

        return b;
    };
    int itobi(int i) {
        int x = itox(i);
        int y = itoy(i);

        int bx = x / 3;
        int by = y / 3;

        int b_inside_x = x - bx * 3;
        int b_inside_y = y - by * 3;
        int bi = b_inside_y * 3 + b_inside_x;

        return bi;
    };
    void itobbi(int &b, int &bi, int i) {
        int x = itox(i);
        int y = itoy(i);

        int bx = x / 3;
        int by = y / 3;
        b = by * 3 + bx;

        int b_inside_x = x - bx * 3;
        int b_inside_y = y - by * 3;
        bi = b_inside_y * 3 + b_inside_x;
    };
    int btoi(int b, int bi) {

        // get block x,y position in sudoku
        int bx = b % 3;
        int by = b / 3;

        // get cell x,y position in block
        int b_inside_x = bi % 3;
        int b_inside_y = bi / 3;

        // get cell x,y,i position in sudoku
        int x = bx * 3 + b_inside_x;
        int y = by * 3 + b_inside_y;
        int i = xytoi(y, x);

        // return
        return i;
    };
    void btoi(int &i, int b, int bi) {
        i = btoi(b, bi);
    }

    void setDigit(int y, int x, int d) {
        digits[y][x] = d;
    }
    void setDigit(int i, int d) {
        int y, x;
        itoxy(y, x, i);
        setDigit(y, x, d);
    }
    int getDigit(int i) {
        return digits[i / 9][i % 9];
    };
    int getDigit(int y, int x) {
        return digits[y][x];
    };
    bool isDigitEnable(int y, int x, int d) {
        if (flag_conf_enableDigit == conflicts[y][x][d - 1][0]) {
            return true;
        } else {
            return false;
        }
    };
    bool isDigitEnable(int i, int d) {
        return (isDigitEnable(i / 9, i % 9, d));
    };
    bool isDigitConflict(int y, int x, int d) {
        return (!isDigitEnable(y, x, d));
    };
    bool isDigitConflict(int i, int d) {
        return (!isDigitEnable(i, d));
    };
    int getConflict(int y, int x, int d, int range) {
        if (true == isDigitConflict(y, x, d)) {
            int conf_i = conflicts[y][x][d - 1][range];
            return conf_i;
        }
        return -1;
    }
    int getRowConflict(int y, int x, int d) {
        return (getConflict(y, x, d, 1));
    }
    int getRowConflict(int i, int d) {
        return (getConflict(itoy(i), itox(i), d, 1));
    }
    int getColConflict(int y, int x, int d) {
        return (getConflict(y, x, d, 2));
    }
    int getColConflict(int i, int d) {
        return (getConflict(itoy(i), itox(i), d, 2));
    }
    int getBloConflict(int b, int bi, int d) {
        int i = btoi(b, bi);
        int x, y;
        itoxy(y, x, i);
        return (getConflict(y, x, d, 3));
    }
    int getBloConflict(int i, int d) {
        return (getConflict(itoy(i), itox(i), d, 3));
    }
    bool isBadCell(int y, int x) {
        // for: each digit
        for (int d = 1; d <= 9; d++) {
            // if digit is enable
            if (true == isDigitEnable(y, x, d)) {
                return false;
            }
        }
        // each digit is disabled for this cell, means it is a bad cell
        return true;
    }
    bool isBadCell(int i) {
        // check if all the digits of the cell are disabled

        int y, x;
        itoxy(y, x, i);
        return (isBadCell(y, x));
    }

    // api
    bool readin(const char *sudoku_str);
    void initConflicts();
    bool checkConflict(int indx, int digt);
    bool checkConflict_row(int indx, int digt, int *conflict_i);
    bool checkConflict_col(int indx, int digt, int *conflict_i);
    bool checkConflict_blo(int indx, int digt, int *conflict_i);
    bool commitDigit(int indx, int digt, bool check);
    bool setConflict(int indx_self, int digt, int indx_conf);
    void markMatesConflict_row(int indx_self, int digt);
    void markMatesConflict_col(int indx_self, int digt);
    void markMatesConflict_blo(int indx_self, int digt);
    void markMatesConflicts(int indx_self, int digt);
    void clrMatesConflicts(int indx);
    bool checkDigit_byMarks(int indx, int digt);
    int seekDigit_destiny(int indx);
    int seekDigit_fate(int indx);
    int confirmDigits(bool save_answer_if_succeed);
    int confirmDigits();
    int confirmDigits_try();
    bool seekDigits_zombie(bool must_be_true, bool record_results, bool print_answer_line_immediately);
    int crack(bool save_answers, bool print_answer_line_immediately);
    int crack(bool save_answers);
    void getString(char *str);
    static void printLineTo(char *user_buffer, char *str, char *ch_uncommitted_digit);
    void printLineTo(char *user_buffer, char *str);
    void printLineTo(char *user_buffer);
    void printLine(char *str);
    void printLine();
    void printSudokuCells(); // by sudoku cells
    static void printPanelTo(char *user_buffer, char *str, int i_cur);
    static void printPanelTo(char *user_buffer, char *str);
    void printPanelTo(char *user_buffer, int i_cur);
    void printPanelTo(char *user_buffer);
    void printPanel(char *str, int i_cur);
    void printPanel(int i_cur);
    void printPanel();
    void printAnswer(int i, bool panel_or_line);
    void printAnswer_panel(int i);
    void printAnswer_line(int i);
    const char *getAnswerString(int i);
    const char **getAnswerStringArray(int &c);
    void printAllAnswers(bool panel_or_line);
    void printAllAnswers_panel();
    void printAllAnswers_line();

    //
    Sudoku();
    Sudoku(char *str);

private:
    //== wait to optimize
    void addAnswer();
    void delAnswer(int i);
    class BigNumb {
    public:
        static const unsigned long long _max_ = 10'000'000'000'000'000'000ull;
        static void ppBigNumb(unsigned long long *big_numb) {
            // big-numb ++

            //
            unsigned long long &low = big_numb[0];
            unsigned long long &high = big_numb[1];
            //
            low++;
            if (low >= _max_) {
                low -= _max_;
                high++;
            }
        };
        static void mmBigNumb(unsigned long long *big_numb) {
            // big-numb --

            //
            unsigned long long &low = big_numb[0];
            unsigned long long &high = big_numb[1];
            //
            if (low == 0) {
                high--;
                low = _max_;
            }
            low--;
        };
        static void zeroBigNumb(unsigned long long *big_numb) {
            // big-numb = {0}

            //
            unsigned long long &low = big_numb[0];
            unsigned long long &high = big_numb[1];
            //
            low = 0;
            high = 0;
        };
        static char *toString(char *e__str, unsigned long long *big_numb) {
            // convert big-numb to string

            // something will be used for this func
            auto ulltos = [](char *e__s, unsigned long long ull) {
                // convert unsigned-long-long to string

                // array where to save numb string
                char str[21] = {0};
                char *c = &str[20];
                *c = '\0';
                // add one digit char to the array
                auto insertC = [&c](int d) {
                    c--;
                    *c = d + '0';
                };

                // ull to string
                if (ull == 0) {
                    // set str to '0'
                    insertC(0);
                } else {
                    // get numb string
                    for (int i = 0; true; i++) {
                        // numb ending
                        if (ull == 0) {
                            break;
                        }
                        // get the digit at the least place
                        int d = ull % 10;
                        insertC(d);
                        // shift right by one place
                        ull /= 10;
                    }
                }

                // export numb string
                strcpy(e__s, c);
            };
            unsigned long long &low = big_numb[0];
            unsigned long long &high = big_numb[1];
            char *str = e__str;

            // new mem to save string
            if (e__str == NULL) {
                str = new char[20 + 20 + 1]();
            };
            // high-ull to string
            if (high != 0) {
                ulltos(str, high);
            }
            // low-ull to string
            int high_len = strlen(str);
            char *low_str = str + high_len;
            ulltos(low_str, low);

            // return string
            if (e__str != NULL) {
                e__str = str;
            }
            return str;
        };
    };

    void clrAnswers();
    int cleanAnswers();
    int cleanAnswers(int &before, int &after);
};
