#include <Arduino.h>
#include <Wire.h>

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>

#include <Ticker.h>

#include "max6921awi.h"

#include <WiFiManager.h>

#include "RTClib.h"

// ----- MAX6921AWI 荧光管驱动 ----- /
int Blank = 15; // D8    MAX6921  blank
int Load = 13;  // D7    MAX6921  load
int Clk = 12;   // D6    MAX6921  clock
int Din = 14;   // D5    MAX6921  din
// ----- DS1302 时钟 ----- /
int DS1302_CLK = 3; // RX  SCLK
int DS1302_DAT = 5; // D1  I/O
int DS1302_RST = 4; // D2  CE
// ----- 倒计时按钮 ----- /
// 60秒倒计时。
int SLEEP_BUTTON = 1; // TX
// ----- 显示按钮 ----- /
// 由于荧光管寿命原因，显示10分钟后进入休眠，当按钮背按下时再显示10分钟。
// 将此引脚接地可一直显示不进入休眠状态。
int SHOW_BUTTON = 16; // D0

// ----- NTP Clock ----- /
// NTP校时配置，用于开机时进行时间同步和周期性进行时间校对。
// ntp1.aliyun.com是时间服务器域名。
#define NTP_OFFSET 60 * 60     // In seconds
#define NTP_INTERVAL 60 * 1000 // In miliseconds
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp1.aliyun.com", NTP_OFFSET, NTP_INTERVAL);

// ----- IV27M ----- /
// 荧光管字符设置，初始化所用的字符。
// 需和max6921awi.cpp结合。
//              a  b  c  d  e  f  g
char zero[7] = {1, 1, 1, 1, 1, 1, 0};     // 0
char one[7] = {0, 1, 1, 0, 0, 0, 0};      // 1
char two[7] = {1, 1, 0, 1, 1, 0, 1};      // 2
char three[7] = {1, 1, 1, 1, 0, 0, 1};    // 3
char four[7] = {0, 1, 1, 0, 0, 1, 1};     // 4
char five[7] = {1, 0, 1, 1, 0, 1, 1};     // 5
char six[7] = {1, 0, 1, 1, 1, 1, 1};      // 6
char seven[7] = {1, 1, 1, 0, 0, 0, 0};    // 7
char eight[7] = {1, 1, 1, 1, 1, 1, 1};    // 8
char nine[7] = {1, 1, 1, 1, 0, 1, 1};     // 9
char interval[7] = {0, 0, 0, 0, 0, 0, 1}; // -
char space[7] = {0, 0, 0, 0, 0, 0, 0};    // 空白
char C[7] = {1, 0, 0, 1, 1, 1, 0};        // C
char E[7] = {1, 0, 0, 1, 1, 1, 1};        // E
char H[7] = {0, 1, 1, 0, 1, 1, 1};        // H
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

char old_time[20] = {0}; // 用于存放上一次时间

// 显示配置
int M_TIME_SEC = 3;        // 日期持续时间，单位：秒
int M_TIME_CYCLE = 10;     // 多长周期显示一次，单位：秒
int DAY_COUNTDOWN_SEC = 0; // 天数倒计时持续时间，单位：秒
int M_TIME_TOTAL = M_TIME_SEC + M_TIME_CYCLE + DAY_COUNTDOWN_SEC;

// -------------- DS1302 ----------------//
DS1302 rtc(DS1302_RST, DS1302_CLK, DS1302_DAT);

void m_itoa(int number, char *str, int n);
void get_time();

// ----- Ticker ----- /
// 创建并发任务，24小时校对一次时间
Ticker task1(get_time, 300);

//
char COUNTDOWN_DAY[] = "26-12-2022"; // 天数倒计时截至日期，格式有要求
long COUNTDOWN_DAY_SEC = 0;          // COUNTDOWN_DAY距1970-01-01时间
long REAL_SEC = 0;                   // 当前距1970-01-01时间
int COUNTDOWN_SEC = 60;              // 秒倒计时起始秒数
int COUNTDOWN_SEC_FLAG = 0;          // 倒计时标识，1表示显示，0表示不显示
int SHOW_SEC = 10 * 60;               // 显示持续时间，单位秒
int SHOW_FLAG = 1;                   // 显示标识，1表示显示，0表示不显示
int COUNTDOWN_VAL = 0;               // 秒倒计时临时变量
int sec_sum = 0;                     // 秒计数器，理论上每秒钟增长1
int show_sum = 0;                    //显示计数器

// int转string
void m_itoa(int number, char *str, int n)
{
    int i;
    for (i = n - 1; i >= 0; i--)
    {
        *(str + i) = number % 10 + '0';
        number /= 10;
    }
}

// 通过NTP获取时间
char real_time[12] = {0}; // 2022-04-01
char real_date[12] = {0}; //   22-57-33

void get_time()
{
    char buff[5];
    memset(real_time, '\0', sizeof(real_time));
    memset(real_date, '\0', sizeof(real_date));
    timeClient.update();

    memset(buff, '\0', sizeof(buff));
    m_itoa(timeClient.getHours(), buff, 2);
    strncpy(real_date, buff, 2);
    strncat(real_date, "-", 1);
    memset(buff, '\0', sizeof(buff));
    m_itoa(timeClient.getMinutes(), buff, 2);
    strncat(real_date, buff, 2);
    strncat(real_date, "-", 1);
    memset(buff, '\0', sizeof(buff));
    m_itoa(timeClient.getSeconds(), buff, 2);
    strncat(real_date, buff, 2);
    // Serial.println(real_date);
    // sprintf(real_date, "%s-%s-%s", );

    unsigned long epochTime = timeClient.getEpochTime();
    time_t t = epochTime;
    struct tm *ptm = gmtime(&t);
    // Serial.println(epochTime);

    memset(buff, '\0', sizeof(buff));
    m_itoa(ptm->tm_year + 1900, buff, 4);
    strncpy(real_time, buff, 4);
    strncat(real_time, "-", 1);
    memset(buff, '\0', sizeof(buff));
    m_itoa(ptm->tm_mon + 1, buff, 2);
    strncat(real_time, buff, 2);
    strncat(real_time, "-", 1);
    memset(buff, '\0', sizeof(buff));
    m_itoa(ptm->tm_mday, buff, 2);
    strncat(real_time, buff, 2);
    // Serial.println(real_time);
}

// 通过DS1302时钟获取时间
char local_date[12] = {0};
char local_time[12] = {0};

void get_time_local(void)
{
    char buf[20] = {0};
    DateTime now = rtc.now();
    REAL_SEC = now.unixtime();
    now.tostr(buf);
    if (!strcmp(old_time, buf))
        return;

    memset(local_date, '\0', sizeof(local_date));
    memset(local_time, '\0', sizeof(local_time));
    strncpy(local_time, buf, 10);
    strncpy(local_date, buf + 11, 8);
    local_date[2] = local_date[5] = local_time[4] = local_time[7] = '-';
    // Serial.println(local_date);
    // Serial.println(local_time);
    Serial.println(now.second());
    Serial.println(now.minute());

    strcpy(old_time, buf);
    sec_sum++;
    show_sum++;
}

// 获取倒计时按钮状态
int sleep_button_sum = 0;
int get_sleep_button_status(int butten)
{
    int val = digitalRead(butten);
    if (val == LOW)
        sleep_button_sum++;
    else
        sleep_button_sum = 0;

    // 按钮按下持续一段时间后进入倒计时模式
    if (sleep_button_sum > 100)
        return 1;
    return 0;
}

// 获取显示按钮状态
int show_button_sum = 0;
int get_show_button_status(int button)
{
    int val = digitalRead(button);
    if (val == LOW)
        show_button_sum++;
    else
        show_button_sum = 0;

    if (show_button_sum > 100)
        return 1;
    return 0;
}

// 配置AP模式
// 第一次开机时或无法连接WiFi时用于动态修改wifi
WiFiManager wifiManager;
bool USBConnection = false; // true = usb...
bool WIFIConnection = false;
// flag for saving data
bool shouldSaveConfig = false;

void saveConfigCallback()
{
    if (!USBConnection)
    {
        Serial.println("Should save config");
    }
    shouldSaveConfig = true;
}

void configModeCallback(WiFiManager *myWiFiManager)
{

    if (!USBConnection)
    {
        Serial.println("Entered config mode");
        Serial.println(WiFi.softAPIP());
        Serial.println(myWiFiManager->getConfigPortalSSID());
    }
    Serial.println("1111");
    vfd.show("S", d1, l1, o1);
}

void setup()
{
    Serial.begin(9600);
    Serial.println("start");

    pinMode(SLEEP_BUTTON, INPUT_PULLUP); // 设置引脚上拉模式
    pinMode(SHOW_BUTTON, INPUT_PULLUP);  // 设置引脚上拉模式

    // 初始化荧光管
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
    l1.C = C;
    l1.E = E;
    l1.H = H;
    l1.L = L;
    l1.O = O;
    l1.o = o;
    l1.P = P;
    l1.R = R;
    l1.r = r;
    l1.S = S;
    o1.horizontal = interval;
    o1.space = space;

    vfd.setBrightness(10);
    vfd.setCharacter(7);
    vfd.setNumber(13);
    vfd.begin(Blank, Load, Clk, Din);

    // 初始化AP
    wifiManager.setAPStaticIPConfig(IPAddress(172, 217, 28, 1), IPAddress(172, 217, 28, 1), IPAddress(255, 255, 255, 0));
    WiFiManagerParameter custom_time_sec("sec", "日期持续时间（0不显示日期）", "3", 16);
    WiFiManagerParameter custom_time_cycle("cycle", "日期显示周期", "10", 6);
    WiFiManagerParameter custom_time_countdown("countdown", "日期倒计时", "0", 1);
    // Just a quick hint
    WiFiManagerParameter host_hint("<small>AWTRIX Host IP (without Port)<br></small><br><br>");
    WiFiManagerParameter port_hint("<small>Communication Port (default: 7001)<br></small><br><br>");
    WiFiManagerParameter matrix_hint("<small>0: Columns; 1: Tiles; 2: Rows <br></small><br><br>");
    WiFiManagerParameter p_lineBreak_notext("<p></p>");

    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.setAPCallback(configModeCallback);
    // 可在AP模式下增加其他配置的输入，例如倒计时，显示时间等
    // wifiManager.addParameter(&p_lineBreak_notext);
    // wifiManager.addParameter(&host_hint);
    // wifiManager.addParameter(&custom_time_sec);
    // wifiManager.addParameter(&port_hint);
    // wifiManager.addParameter(&custom_time_cycle);
    // wifiManager.addParameter(&matrix_hint);
    // wifiManager.addParameter(&custom_time_countdown);
    // wifiManager.addParameter(&p_lineBreak_notext);

    if (!wifiManager.autoConnect("VFD Clock", "vfdclock"))
    {
        // reset and try again, or maybe put it to deep sleep
        Serial.println("-- VFD AP --");
        ESP.reset();
        delay(5000);
    }

    // NTP
    timeClient.begin();
    // Set offset time in seconds to adjust for your timezone, for example:
    // GMT +8 = 28800
    timeClient.setTimeOffset(28800);

    vfd.show("-", d1, l1, o1);
    do
    {
        get_time();
        delay(500);
    } while (!strcmp(real_time, "1970-01-01")); // NTP获取时间成功

    Serial.println(real_time);
    Serial.println(real_date);

    rtc.begin();
    if (!rtc.isrunning())
    {
        Serial.println("RTC is NOT running!");
        // following line sets the RTC to the date & time this sketch was compiled
        // rtc.adjust(DateTime("10-04-2022", "18:53:00"));
    }

    if (strlen(COUNTDOWN_DAY) != 10)
        DAY_COUNTDOWN_SEC = 0; // 0表示不显示天数倒计时
    rtc.adjust(DateTime(COUNTDOWN_DAY, "00:00:00"));
    COUNTDOWN_DAY_SEC = rtc.now().unixtime(); // 计算1970-01-01距离截至日期的时间，单位秒
    Serial.println(COUNTDOWN_DAY_SEC);

    char flip_time[11] = {0};
    // 格式转换
    sprintf(flip_time, "%c%c-%c%c-%c%c%c%c", real_time[8], real_time[9],
            real_time[5], real_time[6],
            real_time[0], real_time[1], real_time[2], real_time[3]);
    Serial.printf("flip time :");
    Serial.println(flip_time);
    delay(1);
    if (strlen(real_date))
        rtc.adjust(DateTime(flip_time, real_date)); // 初始化时间

    task1.start(); // 开启校时
}

void loop()
{
    // Serial.println("------");
    char flip_time[11] = {0};
    char countdown_day[13] = {0};
    char countdown_sec[6] = {0};
    int val = COUNTDOWN_SEC;
    int sleep_b, show_b;

    get_time_local();
    sleep_b = get_sleep_button_status(SLEEP_BUTTON);
    show_b = get_show_button_status(SHOW_BUTTON);

    if (sleep_b | COUNTDOWN_SEC_FLAG)
    {
        COUNTDOWN_SEC_FLAG = 1;
        if (!COUNTDOWN_VAL)
            COUNTDOWN_VAL = sec_sum + 1;
        if (sec_sum >= COUNTDOWN_VAL)
            val = COUNTDOWN_SEC - (sec_sum - COUNTDOWN_VAL);

        sprintf(countdown_sec, "- - %d - -", val);
        vfd.show(countdown_sec, d1, l1, o1);
        if (!val)
        {
            COUNTDOWN_SEC_FLAG = 0;
            SHOW_FLAG = 1;
            COUNTDOWN_VAL = 0; // 复位
        }
    }
    else if (show_b | SHOW_FLAG)
    {
        SHOW_FLAG = 1;
        if (show_b)
            show_sum = 0;
        if ((sec_sum % M_TIME_TOTAL) < DAY_COUNTDOWN_SEC) // 60秒内显示DAY_COUNTDOWN_SEC秒
        {
            sprintf(countdown_day, "---%ld---", (COUNTDOWN_DAY_SEC - REAL_SEC) / 86400);
            vfd.show(countdown_day, d1, l1, o1);
        }
        else
        {
            if ((sec_sum % M_TIME_TOTAL) < (M_TIME_SEC + DAY_COUNTDOWN_SEC))
            {
                vfd.show(local_time, d1, l1, o1);
            }
            else
                vfd.show(local_date, d1, l1, o1);
        }
    }
    else
        vfd.show("             ", d1, l1, o1);

    // 通过NTP校时
    if (!strcmp("00-00-00", local_date))
    {
        task1.update();
        if (strcmp(real_date, local_date))
        {
            sprintf(flip_time, "%c%c-%c%c-%c%c%c%c", real_time[8], real_time[9],
                    real_time[5], real_time[6],
                    real_time[0], real_time[1], real_time[2], real_time[3]);
            rtc.adjust(DateTime(flip_time, real_date));
            // Serial.println("NTP school hours");
        }
    }

    // 休眠模式
    if (show_sum >= SHOW_SEC)
        SHOW_FLAG = 0;
    if (sec_sum > (M_TIME_TOTAL * 10000))
    {
        sec_sum = 0;
        show_sum = 0;
    }
}