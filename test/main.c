#include "main.h"

uchar j; 
unsigned char status;
uchar Timer0_1sFlag;
uchar Timer0Count;
uchar FirstFlag;
uchar K1Flag;

char xdata dispbuff[32];
#define INBUF_LEN 8   //���ݳ���
//-------------------------------------------------
typedef struct 
{
 vu8 bRxLength;				  // �Ѿ����յ����ݳ���
 vu8 bTxLength;				  // �Ѿ����͵����ݳ���
 vu8 bTxBufferLength;		  // ���ͻ������е����ݳ���
 u8  bRxBuffer[40];	          // �������ݻ�����
 //u8 bTxBuffer[38];	      // �������ݻ�����
}xdata UART_Buffer;
typedef union  
{ 
	unsigned int word16; 
    unsigned char byte[2]; 
}CRC_C; 

u8 xdata USART_RX_BUF[64];     //���ջ���,���64���ֽ�.
u8 xdata USART_TX_BUF[64];     //���ͻ���,���64���ֽ�.

UART_Buffer USART1_Buf;
u8 USART1_ReFlag=0;
CRC_C crcnow;

u32 Voltage_data;  //��ѹ
u32 Power_data;    //����
u32 Energy_data1;  //����
u32 Energy_data;   //����

unsigned char code data1[16] = {0x12,0x34,0x56,0x78,0xED,0xCB,0xA9,0x87,0x12,0x34,0x56,0x78,0x01,0xFE,0x01,0xFE};
//M1����ĳһ��дΪ���¸�ʽ����ÿ�ΪǮ�����ɽ��տۿ�ͳ�ֵ����
//4�ֽڽ����ֽ���ǰ����4�ֽڽ��ȡ����4�ֽڽ�1�ֽڿ��ַ��1�ֽڿ��ַȡ����1�ֽڿ��ַ��1�ֽڿ��ַȡ�� 
unsigned char code data2[4]  = {0,0,0,0x01};
unsigned char code DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 

unsigned char g_ucTempbuf[20];                        
void delay1(unsigned int z)
{
	unsigned int x,y;
	for(x=z;x>0;x--)
	for(y=110;y>0;y--);	
} 
	
/*-----------------------------��ʱ���� 1MS/��-------------------------------*/
void DelayM(uint a)       	
{
	uchar i;
	while( --a != 0) { for(i = 0; i < 125; i++); }   				   
}
/*��ʱ��0��ʼ��---------------------------------------------------------------*/
void Timer0Init(void)
{
	TMOD|=0x01;
	TH0=0xB8; //��ʽ1,20ms,12MHz
	TL0=0x00;
	TR0=1;
	ET0=1;
	EA=1;
	Timer0_1sFlag=0;
	Timer0Count=0;
}
/*��ʱ������---------------------------------------------------------------*/
void Timer0(void)interrupt 1
{
	TH0=0xB8;
	TL0=0x00;
	if(++Timer0Count>=50) // 1s
	{
		Timer0Count=0;
		Timer0_1sFlag=1;
	}
}
/*���ڳ�ʼ��---------------------------------------------------------------*/
void init_ser()
{    
	SCON|=0x50;              
	TMOD|=0X20; //��ʱ��1�����ڷ�ʽ2  
	PCON|= 0x80;//SMOD=1       
	TH1=0Xf4;   //������4800      
	//TL1=0Xf4;   //12MHZ       
	IE  |= 0x90;       //Enable Serial Interrupt 
	TR1   = 1;          // timer 1 run   
}
/********************************************************************
** //�򴮿ڷ���һ���ַ� 
********************************************************************/
void send_char_com(unsigned char ch)  
{
	ES     =   0;  //�ش����ж�
	TI     =   0;  //���㴮�ڷ�������ж������־
	SBUF   =   ch;
	while(TI ==0); //�ȴ��������
	TI     =   0;  //���㴮�ڷ�������ж������־
	ES     =   1;  //�������ж�
}
/********************************************************************
** //�򴮿ڷ���һ���ַ�����strlenΪ���ַ������� 
********************************************************************/
void send_string_com(unsigned char *str,unsigned int strlen)
{
	unsigned int k=0;
	do 
	{
		send_char_com(*(str + k));
		k++;
	} 
	while(k < strlen);
}
unsigned int calccrc(unsigned char crcbuf,unsigned int crc) 
{ 
	unsigned char i; 
	unsigned char chk; 
	crc=crc ^ crcbuf; 
	for(i=0;i<8;i++) 
	{ 		
		chk=( unsigned char)(crc&1); 
		crc=crc>>1; 
		crc=crc&0x7fff;  
		if (chk==1)
		crc=crc^0xa001;  
		crc=crc&0xffff;  
	}
	return crc; 
}

/*У��ͺ���---------------------------------------------------------------------*/
unsigned int chkcrc(unsigned char *buf,unsigned char len)
{ 
	unsigned char hi,lo; 
	unsigned int i;  
	unsigned int crc;  
	crc=0xFFFF; 
	for(i=0;i<len;i++) 
	{ 
	 	crc=calccrc(*buf,crc); 
		buf++; 
	} 
	hi=( unsigned char)(crc%256); 
	lo=( unsigned char)(crc/256);
	crc=(((unsigned int)(hi))<<8)|lo; 
	return crc;  
}

/*��ȡ����---------------------------------------------------------------------*/
//��ȡ��ѹ�����������ʣ����غ͵���ֵ
#if 0
void read_data(void) 
{ 
 //   if((Clock.Second%2)==1)      //2 ���һ�� 
    {       
	    USART_TX_BUF[0]=0x01;      //����ģ��� ID ��   
        USART_TX_BUF[1]=0x03; 
        USART_TX_BUF[2]=0x00; 
        USART_TX_BUF[3]=0x48; 
        USART_TX_BUF[4]=0x00; 
        USART_TX_BUF[5]=0x06;//0x06 
        crcnow.word16=chkcrc(USART_TX_BUF,6); 
        USART_TX_BUF[6]=crcnow.byte[0];       //CRC Ч����ֽ���ǰ 
        USART_TX_BUF[7]=crcnow.byte[1]; 
      //USART_TX_BUF[6]=0x45;//0x45           //CRC Ч����ֽ���ǰ 
      //USART_TX_BUF[7]=0xde;//0xde 
        send_string_com(USART_TX_BUF,8);      //���� 8 �����ݣ�

    } 
}
#endif
void read_data(void)
{
	//union	crcdata
 	//{
	//	unsigned int word16;
	//	unsigned char  byte[2];
	//}crcnow;
	//if(read_enable==1)   // ��ʱ�䳭��135ģ�飬��С�������1����
	{
	  //read_enable=0;
		USART_TX_BUF[0]=0x01;   //135ģ���ID�ţ�Ĭ��IDΪ0x01
		USART_TX_BUF[1]=0x03;
		USART_TX_BUF[2]=0x00;
		USART_TX_BUF[3]=0x48;
		USART_TX_BUF[4]=0x00;
		USART_TX_BUF[5]=0x08;
		crcnow.word16=chkcrc(USART_TX_BUF,6);
		USART_TX_BUF[6]=crcnow.byte[0];    //CRCЧ����ֽ���ǰ
		USART_TX_BUF[7]=crcnow.byte[1];
		send_string_com(USART_TX_BUF,8);   //����8�����ݣ�����ݵ�Ƭ�������Լ����
	}
}

/*��������---------------------------------------------------------------------*/
#if 0
void Analysis_data(void) 
{ 
	unsigned char i; 
    if(USART1_ReFlag==0x01)    //������� 
    { 
	USART1_ReFlag=0;
	USART1_Buf.bRxLength=0;
		
        Voltage_data=(((unsigned int)(USART1_Buf.bRxBuffer[3]))<<8)|USART1_Buf.bRxBuffer[4]; //Voltage_data Ϊ unsigned int �� 
        Current_data=(((unsigned int)(USART1_Buf.bRxBuffer[5]))<<8)|USART1_Buf.bRxBuffer[6]; //Current_data Ϊ unsigned int �� 
        Power_data=(((unsigned int)(USART1_Buf.bRxBuffer[7]))<<8)|USART1_Buf.bRxBuffer[8];    //Power_data Ϊ unsigned int �� 
        Energy_data=(((unsigned   long)(USART1_Buf.bRxBuffer[9]))<<24)|(((unsigned   long)(USART1_Buf.bRxBuffer[10]))<<16)|(((unsigned long)(USART1_Buf.bRxBuffer[11]))<<8)|USART1_Buf.bRxBuffer[12];    ////Energy_data Ϊ unsigned long �� 
        Pf_data=(((unsigned int)(USART1_Buf.bRxBuffer[13]))<<8)|USART1_Buf.bRxBuffer[14];    //Pf_data Ϊ unsigned int �� 
        for(i=0;i++;i<18) 
	{
	   USART1_Buf.bRxBuffer[i]=0;
	}  
    }	  
} 
#endif
void Analysis_data(void)
{
	unsigned char i;
	//union	crcdata
 	//{
	//	unsigned int word16;
	//	unsigned char  byte[2];
	//}crcnow;
	if(USART1_ReFlag==0x01)  //�������
	{
		USART1_ReFlag=0;
		USART1_Buf.bRxLength=0;
		//if(RX_Buffer[0]==Read_ID)  //ȷ��ID��ȷ
		{
			//crcnow.word16=chkcrc(RX_Buffer,reveive_number-2);  //reveive_numbe�ǽ��������ܳ���
	  		//if((crcnow.byte[0]==RX_Buffer[reveive_numbe-1])&&(crcnow.byte[1]==RX_Buffer[reveive_numbe-2]))   //ȷ��CRCУ����ȷ
	  		{  	
				Voltage_data=(((unsigned long)(USART1_Buf.bRxBuffer[3]))<<24)|(((unsigned long)(USART1_Buf.bRxBuffer[4]))<<16)|(((unsigned 	long)(USART1_Buf.bRxBuffer[5]))<<8)|USART1_Buf.bRxBuffer[6];
				//Current_data=(((unsigned long)(USART1_Buf.bRxBuffer[7]))<<24)|(((unsigned long)(USART1_Buf.bRxBuffer[8]))<<16)|(((unsigned 	long)(USART1_Buf.bRxBuffer[9]))<<8)|USART1_Buf.bRxBuffer[10]; 
				Power_data=(((unsigned long)(USART1_Buf.bRxBuffer[11]))<<24)|(((unsigned long)(USART1_Buf.bRxBuffer[12]))<<16)|(((unsigned 	long)(USART1_Buf.bRxBuffer[13]))<<8)|USART1_Buf.bRxBuffer[14]; 
				Energy_data=(((unsigned long)(USART1_Buf.bRxBuffer[15]))<<24)|(((unsigned long)(USART1_Buf.bRxBuffer[16]))<<16)|(((unsigned long)(USART1_Buf.bRxBuffer[17]))<<8)|USART1_Buf.bRxBuffer[18]; 
				Energy_data1= 10*Energy_data;
				//Pf_data=(((unsigned long)(USART1_Buf.bRxBuffer[19]))<<24)|(((unsigned long)(USART1_Buf.bRxBuffer[20]))<<16)|(((unsigned long)(USART1_Buf.bRxBuffer[21]))<<8)|USART1_Buf.bRxBuffer[22]; 
				//CO2_data=(((unsigned long)(USART1_Buf[23]))<<24)|(((unsigned long)(RX_Buffer[24]))<<16)|(((unsigned long)(RX_Buffer[25]))<<8)|RX_Buffer[26]; 
	  		}
  		}
		for(i=0;i++;i<38) 
		{
		   USART1_Buf.bRxBuffer[i]=0;
		}  
	}	
}
#if 0
//�������ֵ
void clear_data(void)
{
	USART_TX_BUF[0]=0x01;      //����ģ��� ID ��   
	USART_TX_BUF[1]=0x10; 
	USART_TX_BUF[2]=0x00; 
	USART_TX_BUF[3]=0x0C; 
	USART_TX_BUF[4]=0x00; 
	USART_TX_BUF[5]=0x02; 
	USART_TX_BUF[6]=0x04;
	USART_TX_BUF[7]=0x00;
	USART_TX_BUF[8]=0x00;
	USART_TX_BUF[9]=0x00;
	USART_TX_BUF[10]=0x00;
	crcnow.word16=chkcrc(USART_TX_BUF,11); 
	USART_TX_BUF[11]=crcnow.byte[0];        //CRC Ч����ֽ���ǰ 
	USART_TX_BUF[12]=crcnow.byte[1]; 
	send_string_com(USART_TX_BUF,13);      //���� 13 �����ݣ�
} 
#endif
/********************************************************************
** //���ڽ����жϺ���
********************************************************************/
void serial (void) interrupt 4
{
	if(RI)
	{
		RI=0;
		USART1_Buf.bRxBuffer[USART1_Buf.bRxLength++]=SBUF;	
		if(USART1_Buf.bRxLength==1)
		{
		   if(USART1_Buf.bRxBuffer[0]!=0x01)
		   USART1_Buf.bRxLength=0;
		}
		if(USART1_Buf.bRxLength==2)
		{
		   if(USART1_Buf.bRxBuffer[1]!=0x03)
		   USART1_Buf.bRxLength=0;
		}
		//USART1_ReFlag=0x01;	
		if(USART1_Buf.bRxLength==38)
		{
			crcnow.word16=chkcrc(USART1_Buf.bRxBuffer,35);
		 	if(crcnow.byte[0]==USART1_Buf.bRxBuffer[35]&&crcnow.byte[1]==USART1_Buf.bRxBuffer[36])
			{
			   USART1_ReFlag=0x01;		  //�������
			   USART1_Buf.bRxLength=0;
			}
			
		} 		
		if(USART1_Buf.bRxLength>38)
		{
			USART1_Buf.bRxLength=0;
		}
	}
}
/*��ʾ����---------------------------------------------------------------------*/
void data_display_1(void)
{
    u32 temp,temp1;
	u32 Power_H,Power_L1,Power_L2;  //����
	u32 Voltage_H,Voltage_L;//��ѹ
	u32 Energy_H,Energy_L1,Energy_L2,Energy_L3;//����
	u32 xiaofei_H,xiaofei_L1,xiaofei_L2,xiaofei_L3;//����

	Voltage_H = Voltage_data/10000;		//��ѹ��ʾֵ����
	Voltage_L = (Voltage_data%10000)/10;
 
	Power_H =  Power_data/10000;	    //������ʾֵ����
	Power_L1=Power_data%10000/10;
//	Power_L2=Power_data%10;
 
    temp=Energy_data/32;
	Energy_H = Energy_data/10;	//������ʾֵ����
	Energy_L1= Energy_data%10;
//	Energy_L2= Energy_L1/10;
//	Energy_L3= Energy_L1%10;

    temp1=Energy_data1/32;
    xiaofei_H = Energy_data1/10;	//������ʾֵ����
	xiaofei_L1= Energy_data1%10;
//	Energy_L2= Energy_L1/10;
//	Energy_L3= Energy_L1%10;

	write_com(0x30);
	write_com(0x0C); 
	write_com(0x06);

	write_com(0x82); lcm_w_word("��ǰ�����"); 
	write_com(0x90); lcm_w_word("��ѹ��       V");
	write_com(0x88); lcm_w_word("������       WH");
    write_com(0x98); lcm_w_word("���ѣ�       Y");
 
    write_com(0x93);
	sprintf(dispbuff,"%d",(uint)Voltage_H);//��ѹ����������ʾ
	lcm_w_word(dispbuff);
	lcm_w_word("."); 				//С����
	sprintf(dispbuff,"%d",(uint)Voltage_L);//��ѹ����������ʾ
	lcm_w_word(dispbuff);	


//	write_com(0x93);
//	sprintf(dispbuff,"%d",(uint)Power_H);    //����
//	lcm_w_word(dispbuff);
//	lcm_w_word("."); 	
//	sprintf(dispbuff,"%d",(uint)Power_L1);   //����
//	lcm_w_word(dispbuff);
//	//sprintf(dispbuff,"%d",(uint)Power_L2); //����
//	//lcm_w_word(dispbuff);
 
	write_com(0x8B);
	sprintf(dispbuff,"%d",(uint)Energy_H);//��������������ʾ
	lcm_w_word(dispbuff);
	lcm_w_word("."); 				//С����
	sprintf(dispbuff,"%d",(uint)Energy_L1); //����С��������ʾ
	lcm_w_word(dispbuff);
//	sprintf(dispbuff,"%d",(uint)Energy_L2); //����С��������ʾ
//	lcm_w_word(dispbuff);
//	sprintf(dispbuff,"%d",(uint)Energy_L3); //����С��������ʾ
//	lcm_w_word(dispbuff);

    write_com(0x9B);
	sprintf(dispbuff,"%d",(uint)xiaofei_H);//��������������ʾ
	lcm_w_word(dispbuff);
	lcm_w_word("."); 				//С����
	sprintf(dispbuff,"%d",(uint)xiaofei_L1); //����С��������ʾ
	lcm_w_word(dispbuff);


}


/*��ʾ����---------------------------------------------------------------------*/
void data_display_2(void)
{
  display_type();
  display_cardnum();

   
}
 
/*��ʼ������---------------------------------------------------------------------*/
void InitFunc(void)
{
	K1=1;
	K2=1;
	K3=1;
	K4=1;
	BLK=0;
	BLK=1;
	lcm_init();			//Һ����ʼ��
	lcm_clr();            //����
//	init_ser();
//	Timer0Init();
}

void  InitializeSystem()   //��ʱ���Ͳ�����
{
	P0 = 0xFF; P1 = 0xFF; P3 = 0xFF;P2 = 0xFF;
    init_ser();
	Timer0Init();

//
//  TMOD=0x21;		   //��T0Ϊ��ʽ1��GATE=1��
//	SCON=0x50;
//	TH1=0xF4;          //������Ϊ2400bps
//	TL1=0xF4;
//	TH0=0;
//	TL0=0; 
//	TR0=1;  
//	ET0=1;             //����T0�ж�
//	TR1=1;			   //������ʱ��
//	TI=1;
//	EA=1;			   	//�������ж�
//	
//	ES = 1;
//	RI = 1;

}
	   
 
void main( )
{    
      
     InitFunc();		   //Һ��
     InitializeSystem( );  //ϵͳ��ʼ�������ò����ʣ�����ʱ���ж�
     PcdReset();		   //��λRC522���ɹ�����MI_OK
     PcdAntennaOff(); 	   //�ر�����
     PcdAntennaOn(); 	   //�������ߣ�ÿ��������ر����߷���֮��Ӧ������1ms�ļ��
	 display_welcome2();

 	 while(1)
	 {
	  if(K2==0)
       {
	    DelayM(20);	
	    if(K2==0)
	     {
		  
	  	  lcm_clr();
	       while(1)
	       {
	         Analysis_data();         //��������
	         if(Timer0_1sFlag==1)
	     	  {
			  Timer0_1sFlag=0;
			  read_data(); 
			  }
		      DelayM(500);
		      data_display_1();//������ʾ
			  if(K1==0)  
			  {
			  		K1=0;
					K2=1;	
				 	break;
			  }
		   }
	    } 	 
	  }

	  else if(K1==0)
	    {
		 DelayM(20);
		 if(K1==0)
		   {
		     
		    while (1)
             { 
               status = PcdRequest(PICC_REQALL, g_ucTempbuf);//Ѱ��
               if (status != MI_OK)
               {    
                InitializeSystem( );	//��ʼ��
                PcdReset();			    //��λ
                PcdAntennaOff(); 		//������
                PcdAntennaOn(); 		//������
			    continue;
                }
	    	     lcm_clr();
	             data_display_2();
				 if(K2==0)
				 { 
				 	K2=0;
					K1=1;
					//K2=0;	
				 	break;
				 }
				 	
			    
	        }	
		  }
	    }	
	 }
}







 
 
