#include "max6921awi.h"

int Blank = 15; // D8    MAX6921  blank
int Load = 13;  // D7    MAX6921  load
int Clk = 12;   // D6    MAX6921  clock
int Din = 14;   // D5    MAX6921  din

//                   a b c d e f g
char zero[7] = {1, 1, 1, 1, 1, 1, 0};     // 0
char one[7] = {0, 1, 1, 0, 0, 0, 0};      // 1
//char one[] = "0110000";      // 1
char two[7] = {1, 1, 0, 1, 1, 0, 1};      // 2
char three[7] = {1, 1, 1, 1, 0, 0, 1};    // 3
char four[7] = {0, 1, 1, 0, 0, 1, 1};     // 4
char five[7] = {1, 0, 1, 1, 0, 1, 1};     // 5
char six[7] = {1, 0, 1, 1, 1, 1, 1};      // 6
char seven[7] = {1, 1, 1, 0, 0, 0, 0};    // 7
char eight[7] = {1, 1, 1, 1, 1, 1, 1};    // 8
char nine[7] = {1, 1, 1, 1, 0, 1, 1};     // 9
char interval[7] = {0, 0, 0, 0, 0, 0, 1}; // -
char E[7] = {1, 0, 0, 1, 1, 1, 1};        // E
char R[7] = {1, 1, 0, 1, 1, 1, 1};        // R
char r[7] = {0, 0, 0, 0, 1, 0, 1};        // r
char S[7] = {1, 0, 1, 1, 0, 1, 1};        // S
char L[7] = {0, 0, 0, 1, 1, 1, 0};        // L
char P[7] = {1, 1, 0, 0, 1, 1, 1};        // P
char O[7] = {1, 1, 1, 1, 1, 1, 0};        // O
char o[7] = {0, 0, 1, 1, 1, 0, 1};        // o

max6921awi vfd;
struct dis_digital d1;
struct dis_letter l1;
struct dis_other o1;

void setup(){
    Serial.begin(9600);
    Serial.println("start");
    
    d1.zero = zero;
    d1.one = one;
    d1.two = two;
    d1.three = three;
    d1.four = four;
    d1.five = five;
    d1.six = six;
    d1.seven = seven;
    d1.eight = eight;
    d1.nine = nine;
    l1.E = E;
    l1.L = L;
    l1.O = O;
    l1.o = o;
    l1.P = P;
    l1.R = R;
    l1.r = r;
    l1.S = S;
    o1.horizontal = interval;

    vfd.setBrightness(10);
    vfd.setCharacter(7);
    vfd.setNumber(13);

    vfd.begin(Blank, Load, Clk, Din);

}

void loop(){
    vfd.show("12-34-567890", d1, l1, o1);
}