#ifndef __main_h__
#define __main_h__

#include	<stdio.h>
#include    <string.h>
#include    <intrins.h>
#include    <reg52.h>
#include    "config.h"
#include    "LCD12864.h"
#include    "mfrc522.h"
#include    "12864.h"

//////////////////////////////////
//端口定义
sbit K1  = P2^7; //K1
sbit K2  = P2^6; //K2
sbit K3  = P2^5; //K3
sbit K4  = P2^4; //K4
//sbit K5  = P2^9; //液晶背光控制按键，按一下亮，再按一下灭
sbit BLK  = P1^7; //液晶背光控制输出，低电平有效，PNP三极管控制。
sbit BEEP = P3^7;
sbit LCM_RS  = P1^5;   //模式位，为0输入指令，为1输入数据 
sbit LCM_RW  = P1^6;   //读写位，为0读，为1写 
sbit LCM_EN  = P1^4;   //使能位，高脉冲  
sbit LCM_PSB = P3^4;
sbit LCM_RST = P3^3; 
#define Lcd_Bus P0     //LCM12864数据总线，P0.0--P0.7对应连接DB0--DB7
//sbit     MF522_SDA  = P2^0;	      //SDA
//sbit     MF522_SCK  = P2^1;         //SCK
//sbit     MF522_MOSI = P2^2;         //MOSI
//sbit     MF522_MISO = P2^3;         //MISO
//sbit     MF522_RST  = P1^0;         //RST
sbit     MF522_SDA  = P1^0;	    	//SDA
sbit     MF522_SCK  = P1^1;         //SCK
sbit     MF522_MOSI = P1^2;         //MOSI
sbit     MF522_MISO = P1^3;         //MISO
sbit     MF522_RST  = P1^4;         //RST
sbit     LED_GREEN  = P0^0;  
sbit     Speaker    = P3^6;

/*---------------------函数声明------------------------------*/
void DelayM(uint);       	
void ds_w(void);
void Conver_week(bit c,uchar year,uchar month,uchar day);

void InitializeSystem();                                   
                            



#endif