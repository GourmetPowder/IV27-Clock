#include "max6921awi.h"

max6921awi::max6921awi(uint8_t blank_pin, uint8_t load_pin, uint8_t clk_pin, uint8_t din_pin)
{
    blank = blank_pin;
    load = load_pin;
    clk = clk_pin;
    din = din_pin;
}

void max6921awi::begin(uint8_t blank_pin, uint8_t load_pin, uint8_t clk_pin, uint8_t din_pin)
{
    blank = blank_pin;
    load = load_pin;
    clk = clk_pin;
    din = din_pin;
    pinMode(blank, OUTPUT);
    pinMode(load, OUTPUT);
    pinMode(clk, OUTPUT);
    pinMode(din, OUTPUT);
}

void max6921awi::setBrightness(int n)
{
    // 1 ~ 255  数值越小越亮
    analogWrite(blank, n);
    Serial.printf("setBrightness :");
    Serial.println(n);
}

void max6921awi::setNumber(int n)
{
    number_val = n;
    Serial.printf("setNumber :");
    Serial.println(n);
}

void max6921awi::setCharacter(int n)
{
    character_val = n;
    Serial.printf("setCharacter :");
    Serial.println(n);
}

void max6921awi::charcopy(char *src, char *dest, int n)
{
    int i;
    // Serial.printf("character_val : ");
    // Serial.println(character_val);
    for (i = 0; i < character_val; i++)
        *(dest + i) = *(src + i);
    // 翻转，从右到左1~13
    *(dest + character_val + (number_val - n)) = 1;
}

void max6921awi::showOne(char *character, int digit)
{
    int i;
    memset(content, '\0', sizeof(content));
    charcopy(character, content, digit);

    digitalWrite(load, LOW);
    for (i = 19; i >= 0; i--)
    {
        digitalWrite(din, content[i]);
        digitalWrite(clk, HIGH);
        digitalWrite(clk, LOW);
    }
    digitalWrite(load, HIGH);
    digitalWrite(load, LOW);
}

int max6921awi::displayCenter(char *str)
{
    return ((number_val / 2) - (strlen(str) / 2));
}

void max6921awi::show(char *all, struct dis_digital d, struct dis_letter l, struct dis_other o)
{
    int i;
    // int cen = displayCenter(all);
    int lenght = strlen(all);
    int cen1, cen2;
    cen1 = number_val / 2 - lenght / 2;
    cen2 = cen1 + lenght;

    for (i = 0; i < lenght; i++)
    {
        // Serial.println(*(all + i));
        switch (*(all + i))
        {
        case '0':
            showOne(d.zero, cen2 - i);
            break;
        case '1':
            showOne(d.one, cen2 - i);
            break;
        case '2':
            showOne(d.two, cen2 - i);
            break;
        case '3':
            showOne(d.three, cen2 - i);
            break;
        case '4':
            showOne(d.four, cen2 - i);
            break;
        case '5':
            showOne(d.five, cen2 - i);
            break;
        case '6':
            showOne(d.six, cen2 - i);
            break;
        case '7':
            showOne(d.seven, cen2 - i);
            break;
        case '8':
            showOne(d.eight, cen2 - i);
            break;
        case '9':
            showOne(d.nine, cen2 - i);
            break;
        case 'C':
            showOne(l.C, cen2 - i);
            break;
        case 'E':
            showOne(l.E, cen2 - i);
            break;
        case 'H':
            showOne(l.H, cen2 - i);
            break;
        case 'L':
            showOne(l.L, cen2 - i);
            break;
        case 'O':
            showOne(l.O, cen2 - i);
            break;
        case 'o':
            showOne(l.o, cen2 - i);
            break;
        case 'P':
            showOne(l.P, cen2 - i);
            break;
        case 'R':
            showOne(l.R, cen2 - i);
            break;
        case 'r':
            showOne(l.r, cen2 - i);
            break;
        case 'S':
            showOne(l.S, cen2 - i);
            break;
        case '-':
            showOne(o.horizontal, cen2 - i);
            break;
        case ' ':
            showOne(o.space, cen2 - i);
            break;

        default:
            showOne(o.horizontal, cen2 - i);
            break;
        }
    delayMicroseconds((16-lenght)*100);
    }
    if(lenght > 1)
        showOne(o.space, 13);
}

/*
void max6921awi::showSleep(struct dis_digital d, struct dis_letter l, struct dis_other o)
{
#ifdef ESP8266
    ESP.wdtDisable();
#endif
    show("SLEEP", d, l, o);
#ifdef ESP8266
    ESP.wdtEnable(1000);
#endif
}
*/