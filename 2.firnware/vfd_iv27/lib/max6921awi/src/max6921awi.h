
#include <Arduino.h>
#include <Wire.h>

#define NUM_LEN 8

struct dis_digital {
    char *zero = NULL;
    char *one = NULL;
    char *two = NULL;
    char *three = NULL;
    char *four = NULL;
    char *five = NULL;
    char *six = NULL;
    char *seven = NULL;
    char *eight = NULL;
    char *nine = NULL;
};

// a b c d e f g h i j k l m n o p q r s t u v w x y z
struct dis_letter {
    char *C = NULL;
    char *E = NULL;
    char *H = NULL;
    char *L = NULL;
    char *O = NULL;
    char *o = NULL;
    char *P = NULL;
    char *R = NULL;
    char *r = NULL;
    char *S = NULL;
};

struct dis_other {
    char *horizontal = NULL;
    char *space = NULL;
};

class max6921awi
{
    int displayCenter(char *str);
    void charcopy(char *src, char *dest, int n);

public:
    max6921awi(uint8_t blank_pin = 15, uint8_t load_pin = 13, uint8_t clk_pin = 12, uint8_t din_pin = 14);
    void begin(uint8_t blank_pin, uint8_t load_pin, uint8_t clk_pin, uint8_t din_pin);
    void setBrightness(int n=1);
    void setCharacter(int n = 8);
    void setNumber(int n = 13);
    void showOne(char *character, int digit);
    void show(char *all, struct dis_digital d, struct dis_letter l, struct dis_other o);
    // void showSleep(struct dis_digital d, struct dis_letter l, struct dis_other o);

protected:
    uint8_t blank, load, clk, din;
    int character_val;
    int number_val;
    char content[20];
};