#ifndef __CBIGNUMB
#define __CBIGNUMB

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>

class CSudoku;

typedef void (*PFvoid)();

class CBigNumb {

private:
    bool is_unsigned = true;
    unsigned long long low = 0;
    unsigned long long high = 0;

    char **strings = nullptr;
    int string_count = 0;

    PFvoid __whenOutOfMem = NULL;

    static const unsigned long long _max_ = 10'000'000'000'000'000'000ULL;
    // unsned-64b:   +18,446,744,073,709,551,615
    // signed-64b: +/- 9,223,372,036,854,775,807

    static void __whenOutOfMem_() {
        printf("Memory is not enought, please free memory and then try again. \n");
    };
    void uaadd() {
        // big-numb ++

        low++;
        if (low >= _max_) {
            low -= _max_;
            high++;
        }
    };
    void ussub() {
        // big-numb --

        if (low == 0) {
            high--;
            low = _max_;
        }
        low--;
    };
    void aadd() {
        if ((low == 0) && (high = 0)) {
            uaadd();
            is_unsigned = true;
        } else {
            if (is_unsigned == true) {
                uaadd();
            } else {
                ussub();
            }
        }
    };
    void ssub() {
        if ((low == 0) && (high = 0)) {
            uaadd();
            is_unsigned = false;
        } else {
            if (is_unsigned == true) {
                ussub();
            } else {
                uaadd();
            }
        }
    };

public:
    ~CBigNumb() {
        for (int i = 0; i < string_count; i++) {
            delete[] strings[i];
        }
        free(strings);
    };

    CBigNumb &operator++(int) {
        aadd();
        return *this;
    };
    CBigNumb &operator--(int) {
        ssub();
        return *this;
    };

    CBigNumb operator+(long long x) {
        if (x < 0) {
            for (long long i = 0; i < x; i++) {
                this->ssub();
            }
        } else {
            for (long long i = 0; i < x; i++) {
                this->aadd();
            }
        }
        return *this;
    }
    CBigNumb operator-(long long x) {
        x = -1 * x;
        (*this) + x;
        return *this;
    }

    void zero() {
        // big-numb = {0}

        low = 0;
        high = 0;
    };

    unsigned long long getLow() {
        return low;
    }
    unsigned long long getHigh() {
        return high;
    }
    bool isNegative() {
        return (is_unsigned == false);
    }
    bool isPositive() {
        return (is_unsigned == true);
    }

    void setCallback_OutOfMem(PFvoid pfunc) {
        __whenOutOfMem = pfunc;
    };

    char *toString() {
        // get the string of the BigNumb

        // if the bignumb value is zero
        bool ZERO = false;
        if ((low == 0) && (high == 0)) {
            ZERO = true;
        }
        // if the bignumb is negative
        bool SIGNED = false;
        if (is_unsigned == false) {
            SIGNED = true;
        }

        // calc the place count of the bignumb
        unsigned long long copy_low;
        unsigned long long copy_high;
        int place_count = 0;
        if (ZERO == false) {
            copy_low = low;
            while (copy_low != 0) {
                place_count++;
                copy_low /= 10;
            }
            copy_high = high;
            while (copy_high != 0) {
                place_count++;
                copy_high /= 10;
            }
        } else {
            place_count = 1;
        }

        // calc the length of the string
        int str_len;
        if (ZERO == true) {
            str_len = 1;
        } else {
            str_len = place_count;
            if (SIGNED == true) {
                str_len += 1;
            }
        }
        int quote_cnt = (place_count - 1) / 3;
        str_len += quote_cnt;

        // alloc the mem for the string and init
        char *str = new char[str_len + 1];
        if ((SIGNED == true) && (ZERO == false)) {
            str[0] = '-';
        }
        char *c = &str[str_len];
        auto pushC = [&](char ch) {
            *(c--) = ch;
        };
        pushC('\0');

        // get the string of low-BigNumb
        copy_low = low;
        int pushed = 0;
        while (copy_low != 0) {
            // print quote

            if ((pushed != 0) && (pushed % 3 == 0)) {
                pushC('\'');
            }
            // print digit
            int d = copy_low % 10;
            char ch = '0' + d;
            pushC(ch);

            pushed++;
            copy_low /= 10;
        }
        // get the string of high-BigNumb
        copy_high = high;
        pushed = 0;
        while (copy_high != 0) {
            // print quote
            if ((pushed != 0) && (pushed % 3 == 0)) {
                pushC('\'');
            }
            // print digit
            int d = copy_high % 10;
            char ch = '0' + d;
            pushC(ch);

            pushed++;
            copy_high /= 10;
        }

        // record the str addr
        string_count++;
        while (true) {
            void *p = realloc(strings, string_count * sizeof(void *));
            // failed
            if (p == NULL) {
                if (__whenOutOfMem == NULL) {
                    __whenOutOfMem = __whenOutOfMem_;
                }
                __whenOutOfMem_();
                continue;
            }
            // success
            strings = (char **)p;
            break;
        }
        strings[string_count - 1] = str;

        // return the string
        return str;
    };
    void freeString(char *str) {
        for (int i = 0; i < string_count; i++) {
            if (strings[i] == str) {
                delete[] str;
                break;
            }
        }
    };
};

#endif