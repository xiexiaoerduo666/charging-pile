
#define uchar unsigned char
#define uint  unsigned int
#define delayNOP(); {_nop_();_nop_();_nop_();_nop_();};
//uchar IRDIS[2];
//uchar IRCOM[4];
//void delay0(uchar x);  //x*0.14MS
//void beep();
void dataconv();
void lcd_pos(uchar X,uchar Y);  //确定显示位置
bit  lcd_busy();			    //检查LCD忙状态
void LCD_Data(unsigned char LCDbyte ); //LCD数据
void lcd_wcmd(uchar cmd);		//写指令数据到LCD
void lcd_wdat(uchar dat);		//写显示数据到LCD
void lcd_init();				//LCD初始化设定
void display_welcome2();		//显示欢迎
void display_cardnum();			//显示卡号
void display_carddat();         //显示余额
void display_xiaofeicarddat();  //显示消费
void display_type();            //寻到卡后显示出该卡的类型