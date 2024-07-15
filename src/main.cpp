#include "test.h" //自定义的头文件
#include <DNSServer.h>  //用于DNS服务器功能的库
#include <ESPUI.h>  //用于创建UI界面的库
#include "Freenove_WS2812_Lib_for_ESP32.h"  //用于控制WS2812的库

const byte DNS_PORT = 53;  //定义DNS服务器的端口号为53
IPAddress apIP(192, 168, 4, 1);  //定义AP模式下的IP地址
DNSServer dnsServer;  //创建一个DNS服务器对象"dnsServer"

#if defined(ESP32)  //检查是否定义了ESP32宏，用于区分代码是在ESP32还是ESP8266上运行
#include <WiFi.h>  //如果是ESP32，include WiFi库
#else  //否则，执行针对ESP8266的代码
// esp8266

#ifndef MMU_IRAM_HEAP
#warning Try MMU option '2nd heap shared' in 'tools' IDE menu (cf. https://arduino-esp8266.readthedocs.io/en/latest/mmu.html#option-summary)
#warning use decorators: { HeapSelectIram doAllocationsInIRAM; ESPUI.addControl(...) ... } (cf. https://arduino-esp8266.readthedocs.io/en/latest/mmu.html#how-to-select-heap)
#warning then check http://<ip>/heap
#endif // MMU_IRAM_HEAP
#ifndef DEBUG_ESP_OOM
#error on ESP8266 and ESPUI, you must define OOM debug option when developping
#endif
#endif

const char *ssid = "GuangYu2019";  //定义WiFi网络的SSID（网络名称）
const char *password = "Gyems123";  //定义WiFi网络的密码
const char *hostname = "espressif";  //设备的主机名

// 定义一些全局变量
uint16_t status1=0;
uint16_t status2=0;
uint16_t button1;
uint16_t button2;
uint16_t button3;
uint16_t button4;
//uint16_t button5;
uint16_t millisLabelId;
uint16_t switchOne;
int32_t Speed;
int32_t rotatespeed;

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(1, 4, 0, TYPE_GRB);  //初始化一个Freenove_ESP32_WS2812对象strip，用于控制WS2812 LED灯条
// 参数解释：1：灯条中的LED数量。4：连接到LED灯条的GPIO引脚编号。0：未使用的引脚编号（一般设置为0）。TYPE_GRB：指定LED灯条的颜色类型（GRB格式）

// 以下是一些回调函数，用于处理UI界面中的控件事件
// 回调函数是指在某个事件发生时，由系统或另一个函数调用的函数。
// 在UI编程中，常见的事件包括按钮点击、鼠标移动、键盘按键等。当事件发生时，系统会自动调用相应的回调函数。

void numberCall_1(Control *sender_1, int32_t type_1) //"Speed"数值控件的回调函数
// 参数解释：Control *sender_1：指向触发事件的控件的指针。int32_t type_1：事件类型(表示该回调是由于什么事件触发的)
{
    Serial.println(sender_1->value);  //打印控件的数值
    Speed = sender_1->value.toInt();  //将控件的数值转换为整数，并赋值给Speed变量
}

void numberCall_2(Control *sender_2, int type_2)  //"rotatespeed"数值控件的回调函数
{
    Serial.println(sender_2->value);
    rotatespeed = sender_2->value.toInt();
}

// 以下定义了四个按钮控件的回调函数
void buttonCallback_1(Control *sender, int type)  
{
    switch (type)  //根据事件类型type的值，执行不同的操作
    {
    case B_DOWN:  //如果事件类型是B_DOWN（按钮按下）
        Serial.println("Button_1 DOWN");  //打印“Button_1 DOWN”到串口
        break;

    case B_UP:  //如果事件类型是B_UP（按钮松开）
        Serial.println("Button_1 UP");  //打印“Button_1 UP”到串口
        status1=1;  //将status1的值设为1（该值的初始值为0）
        break;
    }
}

void buttonCallback_2(Control *sender, int type)
{
    switch (type)
    {
    case B_DOWN:
        Serial.println("Button_2 DOWN");
        break;

    case B_UP:
        Serial.println("Button_2 UP");
        status1=2;  //按钮松开后，将status1的值设为2
        break;
    }
}
void buttonCallback_3(Control *sender, int type)
{
    switch (type)
    {
    case B_DOWN:
        Serial.println("Button_2 DOWN");
        break;

    case B_UP:
        Serial.println("Button_2 UP");
        status1=3;  //按钮松开后，将status1的值设为3
        break;
    }
}
void buttonCallback_4(Control *sender, int type)
{
    switch (type)
    {
    case B_DOWN:
        Serial.println("Button_2 DOWN");
        break;

    case B_UP:
        Serial.println("Button_2 UP");
        status1=4;  //按钮松开后，将status1的值设为4
        break;
    }
}

//以下是备用的按钮控件的回调函数，模板与之前相同

/*void buttonCallback_5(Control *sender, int type)
{
    switch (type)
    {
    case B_DOWN:
        Serial.println("Button_2 DOWN");
        break;

    case B_UP:
        Serial.println("Button_2 UP");
        status1=5;
        break;
    }
}*/

CAN_device_t CAN_cfg;             // CAN Config (CAN配置)
unsigned long previousMillis = 0; // will store last time a CAN Message was send (将存储最后一次发送的CAN消息)
const int interval = 10;          // interval at which send CAN Messages (milliseconds) (发送CAN消息的时间间隔(毫秒))
const int rx_queue_size = 10;     // Receive Queue size (接收队列大小)

void setup()
{
    //定义各个引脚的输出模式以及初始电平状态
    pinMode(PIN_5V_EN, OUTPUT);
    digitalWrite(PIN_5V_EN, HIGH);

    pinMode(CAN_SE_PIN, OUTPUT);
    digitalWrite(CAN_SE_PIN, LOW);

    pinMode(RS485_EN_PIN, OUTPUT);
    digitalWrite(RS485_EN_PIN, HIGH);

    pinMode(RS485_SE_PIN, OUTPUT);
    digitalWrite(RS485_SE_PIN, HIGH);

    pinMode(WS2812_PIN, OUTPUT);
    digitalWrite(WS2812_PIN, LOW);

    ESPUI.setVerbosity(Verbosity::VerboseJSON);  //设置ESPUI库的日志输出级别为VerboseJSON，表示以详细JSON格式输出日志信息
    Serial.begin(115200);  //初始化串口，波特率为115200
    CAN_cfg.speed = CAN_SPEED_1000KBPS;  //设置CAN总线速度为1000 Kbps
    CAN_cfg.tx_pin_id = GPIO_NUM_27;  //设置CAN总线的发送引脚为GPIO 27
    CAN_cfg.rx_pin_id = GPIO_NUM_26;  //设置CAN总线的接收引脚为GPIO 26
    CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
    // 创建一个接收队列，队列大小为rx_queue_size，每个队列项的大小为CAN_frame_t结构体的大小，用于存储接收到的CAN帧
    // Init CAN Module
    ESP32Can.CANInit();

#if defined(ESP32)  //检查当前的硬件平台是ESP32还是ESP8266，并根据平台选择适当的方法设置主机名称
    WiFi.setHostname(hostname);
#else
    WiFi.hostname(hostname);
#endif

    // try to connect to existing network
    WiFi.begin(ssid, password);  //使用预先定义的SSID和密码尝试连接到现有的WiFi网络
    Serial.print("\n\nTry to connect to existing network");

    {
        uint8_t timeout = 10;  

        // Wait for connection, 5s timeout
        // 连接到现有的WiFi网络，每500ms检查一次连接状态，最多等待5s
        do
        {
            delay(500);
            Serial.print(".");
            timeout--;
        } while (timeout && WiFi.status() != WL_CONNECTED);

        // not connected -> create hotspot
        // 如果连接失败，则切换到热点模式并创建一个WiFi热点
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.print("\n\nCreating hotspot");

            WiFi.mode(WIFI_AP);
            delay(100);
            WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
        #if defined(ESP32)
        // 如果是ESP32，则使用ESP.getEfuseMac()获取MAC地址，然后通过位移和位操作提取其中的部分数据生成芯片ID
                    uint32_t chipid = 0;
                    for (int i = 0; i < 17; i = i + 8)
                    {
                        chipid |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
                    }
        #else
        // 如果是ESP8266，则使用ESP.getChipId()获取芯片ID
                    uint32_t chipid = ESP.getChipId();
        #endif
            char ap_ssid[25];  //定义一个字符数组ap_ssid用于存储热点的SSID
            snprintf(ap_ssid, 26, "ESPUI-%08X", chipid);  //将芯片ID格式化为字符串并存储到ap_ssid中
            WiFi.softAP(ap_ssid);  //使用生成的SSID创建WiFi热点

            // 等待热点创建完成，每500ms检查一次，最多等待5s
            timeout = 5;

            do
            {
                delay(500);
                Serial.print(".");
                timeout--;
            } while (timeout);
        }
    }

    dnsServer.start(DNS_PORT, "*", apIP);  // 启动DNS服务器，设置监听端口、域名和IP地址
    // DNS_PORT：DNS服务器监听的端口号(53), "*"：域名通配符，表示对所有域名的请求都响应，apIP：热点模式下使用的IP地址

    // 通过串口监视器输出当前WiFi连接的参数信息，包括模式和IP地址
    Serial.println("\n\nWiFi parameters:");
    Serial.print("Mode: ");
    Serial.println(WiFi.getMode() == WIFI_AP ? "Station" : "Client");
    Serial.print("IP address: ");
    Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
#ifdef ESP8266  //如果是ESP8266平台, 则需要在IRAM中分配内存
    {
        HeapSelectIram doAllocationsInIRAM;
#endif
        //向UI界面中添加两个数值控件，分别用于速度模式和旋转模式，初始值为"0"
        ESPUI.addControl(ControlType::Number, "速度模式数值（正值前进-负值后退）", "0", ControlColor::Sunflower, Control::noParent, &numberCall_1);
        ESPUI.addControl(ControlType::Number, "旋转模式数值（正值左转-负值右转）", "0", ControlColor::Dark, Control::noParent, &numberCall_2);
        // ControlType::Number：控件类型为数值输入; "速度模式数值（正值前进-负值后退）"：控件标签; "0"：控件初始值; ControlColor：控件颜色
        // Control::noParent：控件没有父控件; &numberCall_1和&numberCall_2：控件值改变时的回调函数

        //向UI界面中添加四个按钮控件，分别用于速度模式启动、旋转模式启动、电机停止和电机失能
        button1 = ESPUI.addControl(
            ControlType::Button, " ", "速度模式启动", ControlColor::Peterriver, Control::noParent, &buttonCallback_1);
        button2 = ESPUI.addControl(
            ControlType::Button, " ", "旋转模式启动", ControlColor::Peterriver, Control::noParent, &buttonCallback_2);
        button3 = ESPUI.addControl(
            ControlType::Button, " ", "电机停止", ControlColor::Peterriver, Control::noParent, &buttonCallback_3);
        button4 = ESPUI.addControl(
            ControlType::Button, " ", "电机失能", ControlColor::Peterriver, Control::noParent, &buttonCallback_4);
        /*button5 = ESPUI.addControl(
            ControlType::Button, " ", "电机失能", ControlColor::Peterriver, Control::noParent, &buttonCallback_5);*/
        // ControlType::Button：控件类型为按钮; "速度模式启动"、"..."：控件标签; ControlColor：控件颜色
        // Control::noParent：控件没有父控件; &buttonCallback_1、&buttonCallback_2、&buttonCallback_3、&buttonCallback_4：按钮按下和松开时的回调函数

        ESPUI.begin("Myactuator Demo", "myactuator", "123456");  //启动ESPUI，并设置WiFi热点的SSID为"Myactuator Demo"，用户名为"myactuator"，密码为"123456"

#ifdef ESP8266  //如果是ESP8266平台, 则需要结束ESP8266专用内存分配
    } // HeapSelectIram
#endif

    
    strip.begin();  //初始化WS2812 LED灯条
    strip.setBrightness(10);  //设置LED灯条的亮度为10
	strip.setLedColorData(0, 0, 0, 255);  //设置LED灯条的第一个LED的颜色为蓝色
	strip.show();  //将设置的颜色显示出来
    
}

void loop()
{
    // put your setup code here, to loop run: (将您的设置代码放在此处，循环运行:)
    dnsServer.processNextRequest();  //处理DNS服务器的下一个请求的函数(确保DNS服务器能够随时响应请求)
    if (status2 == 0)
    {
        switch (status1)
        {
        case 1: //速度模式
            int32_t speed1;
            speed1 = -(Speed * 100);  //计算速度值speed1，并调用speedrun(speed1)函数控制电机运行。
            speedrun(speed1); // 速度运行
            status1 = 0;
            status2 = 1;
            break;
        case 2:  //旋转模式
            int32_t speed2;
            speed2 = rotatespeed * 100;  //计算旋转速度值speed2，并调用rotaterun(speed2)函数控制电机旋转。
            rotaterun(speed2);
            status1 = 0;
            status2 = 2;
            break;
            }
    }
    if(status1==3)
    {
        speedrun(0);  //停止电机，并将status1和status2重置为0
        status1=0;
        status2=0;
    }
    if(status1==4)
    {
        STOP();  //失能电机，并将status1和status2重置为0
        status1=0;
        status2=0;
    }
    if (status1==1&status2==1)
    {
        int32_t speed1;
        speed1=-(Speed*100);  //计算速度值speed1，并调用speedrun(speed1)函数控制电机运行。
        speedrun(speed1); // 速度运行
        status1=0;
    }    
    if(status1==2&status2==2)
    {
        int32_t speed2;
        speed2=rotatespeed*100;  //计算旋转速度值speed2，并调用rotaterun(speed2)函数控制电机旋转。
        rotaterun(speed2);
        status1=0;
    }
    
}
