#ifndef __LCD12864_h__ 
#define __LCD12864_h__ 

#include "config.h"



void lcm_w_test(bit i,unsigned char word);//写指令或数据（被调用层）
void write_com(uchar cmdcode);			  //写指令到LCD
void set1616pic(uchar x,uchar y,uchar sign,uchar tt); //显示16X16图形,适用于st7920型液晶
void lcm_w_test(bit i,unsigned char word);//写指令或数据（被调用层）
void write_data(uchar Dispdata);		  //写数据到LCD
void lcm_w_word(uchar *s);				  //向LCM发送一个字符串,长度64字符之内
void chk_busy();						  //检查忙位
void lcm_clr(void);						  //清屏函数
void lcm_clr2(void);//清屏上面3行（用空格填满要清显示的地方，因为液晶屏是静态显示的，所以这办法常用）
void lcm_init();						  //初始化LCD屏
void write1616GDRAM(uchar x,uchar y,uchar sign,uchar *bmp);	 //使用绘图的方法让一个16*16的汉字符反白
void write1632GDRAM(uchar x,uchar y,uchar *bmp)	;			 //显示16X32图形,适用于st7920型液晶
void init_12864_GDRAM()	 ;				  //初始化12864
void Clean_12864_GDRAM(void);			  //清屏函数

#endif