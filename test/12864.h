
#define uchar unsigned char
#define uint  unsigned int
#define delayNOP(); {_nop_();_nop_();_nop_();_nop_();};
//uchar IRDIS[2];
//uchar IRCOM[4];
//void delay0(uchar x);  //x*0.14MS
//void beep();
void dataconv();
void lcd_pos(uchar X,uchar Y);  //ȷ����ʾλ��
bit  lcd_busy();			    //���LCDæ״̬
void LCD_Data(unsigned char LCDbyte ); //LCD����
void lcd_wcmd(uchar cmd);		//дָ�����ݵ�LCD
void lcd_wdat(uchar dat);		//д��ʾ���ݵ�LCD
void lcd_init();				//LCD��ʼ���趨
void display_welcome2();		//��ʾ��ӭ
void display_cardnum();			//��ʾ����
void display_carddat();         //��ʾ���
void display_xiaofeicarddat();  //��ʾ����
void display_type();            //Ѱ��������ʾ���ÿ�������