#ifndef __LCD12864_h__ 
#define __LCD12864_h__ 

#include "config.h"



void lcm_w_test(bit i,unsigned char word);//дָ������ݣ������ò㣩
void write_com(uchar cmdcode);			  //дָ�LCD
void set1616pic(uchar x,uchar y,uchar sign,uchar tt); //��ʾ16X16ͼ��,������st7920��Һ��
void lcm_w_test(bit i,unsigned char word);//дָ������ݣ������ò㣩
void write_data(uchar Dispdata);		  //д���ݵ�LCD
void lcm_w_word(uchar *s);				  //��LCM����һ���ַ���,����64�ַ�֮��
void chk_busy();						  //���æλ
void lcm_clr(void);						  //��������
void lcm_clr2(void);//��������3�У��ÿո�����Ҫ����ʾ�ĵط�����ΪҺ�����Ǿ�̬��ʾ�ģ�������취���ã�
void lcm_init();						  //��ʼ��LCD��
void write1616GDRAM(uchar x,uchar y,uchar sign,uchar *bmp);	 //ʹ�û�ͼ�ķ�����һ��16*16�ĺ��ַ�����
void write1632GDRAM(uchar x,uchar y,uchar *bmp)	;			 //��ʾ16X32ͼ��,������st7920��Һ��
void init_12864_GDRAM()	 ;				  //��ʼ��12864
void Clean_12864_GDRAM(void);			  //��������

#endif