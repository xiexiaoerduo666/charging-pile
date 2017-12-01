#include "main.h"

uchar j; 
unsigned char status;
uchar Timer0_1sFlag;
uchar Timer0Count;
uchar FirstFlag;
uchar K1Flag;

char xdata dispbuff[32];
#define INBUF_LEN 8   //数据长度
//-------------------------------------------------
typedef struct 
{
 vu8 bRxLength;				  // 已经接收的数据长度
 vu8 bTxLength;				  // 已经发送的数据长度
 vu8 bTxBufferLength;		  // 发送缓冲区中的数据长度
 u8  bRxBuffer[40];	          // 接收数据缓冲区
 //u8 bTxBuffer[38];	      // 发送数据缓冲区
}xdata UART_Buffer;
typedef union  
{ 
	unsigned int word16; 
    unsigned char byte[2]; 
}CRC_C; 

u8 xdata USART_RX_BUF[64];     //接收缓冲,最大64个字节.
u8 xdata USART_TX_BUF[64];     //发送缓冲,最大64个字节.

UART_Buffer USART1_Buf;
u8 USART1_ReFlag=0;
CRC_C crcnow;

u32 Voltage_data;  //电压
u32 Power_data;    //功率
u32 Energy_data1;  //消费
u32 Energy_data;   //电能

unsigned char code data1[16] = {0x12,0x34,0x56,0x78,0xED,0xCB,0xA9,0x87,0x12,0x34,0x56,0x78,0x01,0xFE,0x01,0xFE};
//M1卡的某一块写为如下格式，则该块为钱包，可接收扣款和充值命令
//4字节金额（低字节在前）＋4字节金额取反＋4字节金额＋1字节块地址＋1字节块地址取反＋1字节块地址＋1字节块地址取反 
unsigned char code data2[4]  = {0,0,0,0x01};
unsigned char code DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 

unsigned char g_ucTempbuf[20];                        
void delay1(unsigned int z)
{
	unsigned int x,y;
	for(x=z;x>0;x--)
	for(y=110;y>0;y--);	
} 
	
/*-----------------------------延时函数 1MS/次-------------------------------*/
void DelayM(uint a)       	
{
	uchar i;
	while( --a != 0) { for(i = 0; i < 125; i++); }   				   
}
/*定时器0初始化---------------------------------------------------------------*/
void Timer0Init(void)
{
	TMOD|=0x01;
	TH0=0xB8; //方式1,20ms,12MHz
	TL0=0x00;
	TR0=1;
	ET0=1;
	EA=1;
	Timer0_1sFlag=0;
	Timer0Count=0;
}
/*定时器函数---------------------------------------------------------------*/
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
/*串口初始化---------------------------------------------------------------*/
void init_ser()
{    
	SCON|=0x50;              
	TMOD|=0X20; //定时器1工作在方式2  
	PCON|= 0x80;//SMOD=1       
	TH1=0Xf4;   //波特率4800      
	//TL1=0Xf4;   //12MHZ       
	IE  |= 0x90;       //Enable Serial Interrupt 
	TR1   = 1;          // timer 1 run   
}
/********************************************************************
** //向串口发送一个字符 
********************************************************************/
void send_char_com(unsigned char ch)  
{
	ES     =   0;  //关串口中断
	TI     =   0;  //清零串口发送完成中断请求标志
	SBUF   =   ch;
	while(TI ==0); //等待发送完成
	TI     =   0;  //清零串口发送完成中断请求标志
	ES     =   1;  //允许串口中断
}
/********************************************************************
** //向串口发送一个字符串，strlen为该字符串长度 
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

/*校验和函数---------------------------------------------------------------------*/
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

/*读取函数---------------------------------------------------------------------*/
//读取电压，电流，功率，因素和电量值
#if 0
void read_data(void) 
{ 
 //   if((Clock.Second%2)==1)      //2 秒读一次 
    {       
	    USART_TX_BUF[0]=0x01;      //抄读模块的 ID 号   
        USART_TX_BUF[1]=0x03; 
        USART_TX_BUF[2]=0x00; 
        USART_TX_BUF[3]=0x48; 
        USART_TX_BUF[4]=0x00; 
        USART_TX_BUF[5]=0x06;//0x06 
        crcnow.word16=chkcrc(USART_TX_BUF,6); 
        USART_TX_BUF[6]=crcnow.byte[0];       //CRC 效验低字节在前 
        USART_TX_BUF[7]=crcnow.byte[1]; 
      //USART_TX_BUF[6]=0x45;//0x45           //CRC 效验低字节在前 
      //USART_TX_BUF[7]=0xde;//0xde 
        send_string_com(USART_TX_BUF,8);      //发送 8 个数据，

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
	//if(read_enable==1)   // 到时间抄读135模块，最小抄读间隔1秒钟
	{
	  //read_enable=0;
		USART_TX_BUF[0]=0x01;   //135模块的ID号，默认ID为0x01
		USART_TX_BUF[1]=0x03;
		USART_TX_BUF[2]=0x00;
		USART_TX_BUF[3]=0x48;
		USART_TX_BUF[4]=0x00;
		USART_TX_BUF[5]=0x08;
		crcnow.word16=chkcrc(USART_TX_BUF,6);
		USART_TX_BUF[6]=crcnow.byte[0];    //CRC效验低字节在前
		USART_TX_BUF[7]=crcnow.byte[1];
		send_string_com(USART_TX_BUF,8);   //发送8个数据，请根据单片机类型自己编程
	}
}

/*解析函数---------------------------------------------------------------------*/
#if 0
void Analysis_data(void) 
{ 
	unsigned char i; 
    if(USART1_ReFlag==0x01)    //接收完成 
    { 
	USART1_ReFlag=0;
	USART1_Buf.bRxLength=0;
		
        Voltage_data=(((unsigned int)(USART1_Buf.bRxBuffer[3]))<<8)|USART1_Buf.bRxBuffer[4]; //Voltage_data 为 unsigned int 型 
        Current_data=(((unsigned int)(USART1_Buf.bRxBuffer[5]))<<8)|USART1_Buf.bRxBuffer[6]; //Current_data 为 unsigned int 型 
        Power_data=(((unsigned int)(USART1_Buf.bRxBuffer[7]))<<8)|USART1_Buf.bRxBuffer[8];    //Power_data 为 unsigned int 型 
        Energy_data=(((unsigned   long)(USART1_Buf.bRxBuffer[9]))<<24)|(((unsigned   long)(USART1_Buf.bRxBuffer[10]))<<16)|(((unsigned long)(USART1_Buf.bRxBuffer[11]))<<8)|USART1_Buf.bRxBuffer[12];    ////Energy_data 为 unsigned long 型 
        Pf_data=(((unsigned int)(USART1_Buf.bRxBuffer[13]))<<8)|USART1_Buf.bRxBuffer[14];    //Pf_data 为 unsigned int 型 
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
	if(USART1_ReFlag==0x01)  //接收完成
	{
		USART1_ReFlag=0;
		USART1_Buf.bRxLength=0;
		//if(RX_Buffer[0]==Read_ID)  //确认ID正确
		{
			//crcnow.word16=chkcrc(RX_Buffer,reveive_number-2);  //reveive_numbe是接收数据总长度
	  		//if((crcnow.byte[0]==RX_Buffer[reveive_numbe-1])&&(crcnow.byte[1]==RX_Buffer[reveive_numbe-2]))   //确认CRC校验正确
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
//清零电量值
void clear_data(void)
{
	USART_TX_BUF[0]=0x01;      //抄读模块的 ID 号   
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
	USART_TX_BUF[11]=crcnow.byte[0];        //CRC 效验低字节在前 
	USART_TX_BUF[12]=crcnow.byte[1]; 
	send_string_com(USART_TX_BUF,13);      //发送 13 个数据，
} 
#endif
/********************************************************************
** //串口接收中断函数
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
			   USART1_ReFlag=0x01;		  //接受完成
			   USART1_Buf.bRxLength=0;
			}
			
		} 		
		if(USART1_Buf.bRxLength>38)
		{
			USART1_Buf.bRxLength=0;
		}
	}
}
/*显示函数---------------------------------------------------------------------*/
void data_display_1(void)
{
    u32 temp,temp1;
	u32 Power_H,Power_L1,Power_L2;  //功率
	u32 Voltage_H,Voltage_L;//电压
	u32 Energy_H,Energy_L1,Energy_L2,Energy_L3;//电量
	u32 xiaofei_H,xiaofei_L1,xiaofei_L2,xiaofei_L3;//消费

	Voltage_H = Voltage_data/10000;		//电压显示值处理
	Voltage_L = (Voltage_data%10000)/10;
 
	Power_H =  Power_data/10000;	    //功率显示值处理
	Power_L1=Power_data%10000/10;
//	Power_L2=Power_data%10;
 
    temp=Energy_data/32;
	Energy_H = Energy_data/10;	//电量显示值处理
	Energy_L1= Energy_data%10;
//	Energy_L2= Energy_L1/10;
//	Energy_L3= Energy_L1%10;

    temp1=Energy_data1/32;
    xiaofei_H = Energy_data1/10;	//消费显示值处理
	xiaofei_L1= Energy_data1%10;
//	Energy_L2= Energy_L1/10;
//	Energy_L3= Energy_L1%10;

	write_com(0x30);
	write_com(0x0C); 
	write_com(0x06);

	write_com(0x82); lcm_w_word("当前电参数"); 
	write_com(0x90); lcm_w_word("电压：       V");
	write_com(0x88); lcm_w_word("电量：       WH");
    write_com(0x98); lcm_w_word("消费：       Y");
 
    write_com(0x93);
	sprintf(dispbuff,"%d",(uint)Voltage_H);//电压整数部分显示
	lcm_w_word(dispbuff);
	lcm_w_word("."); 				//小数点
	sprintf(dispbuff,"%d",(uint)Voltage_L);//电压整数部分显示
	lcm_w_word(dispbuff);	


//	write_com(0x93);
//	sprintf(dispbuff,"%d",(uint)Power_H);    //功率
//	lcm_w_word(dispbuff);
//	lcm_w_word("."); 	
//	sprintf(dispbuff,"%d",(uint)Power_L1);   //功率
//	lcm_w_word(dispbuff);
//	//sprintf(dispbuff,"%d",(uint)Power_L2); //功率
//	//lcm_w_word(dispbuff);
 
	write_com(0x8B);
	sprintf(dispbuff,"%d",(uint)Energy_H);//电量整数部分显示
	lcm_w_word(dispbuff);
	lcm_w_word("."); 				//小数点
	sprintf(dispbuff,"%d",(uint)Energy_L1); //电量小数部分显示
	lcm_w_word(dispbuff);
//	sprintf(dispbuff,"%d",(uint)Energy_L2); //电量小数部分显示
//	lcm_w_word(dispbuff);
//	sprintf(dispbuff,"%d",(uint)Energy_L3); //电量小数部分显示
//	lcm_w_word(dispbuff);

    write_com(0x9B);
	sprintf(dispbuff,"%d",(uint)xiaofei_H);//电量整数部分显示
	lcm_w_word(dispbuff);
	lcm_w_word("."); 				//小数点
	sprintf(dispbuff,"%d",(uint)xiaofei_L1); //电量小数部分显示
	lcm_w_word(dispbuff);


}


/*显示函数---------------------------------------------------------------------*/
void data_display_2(void)
{
  display_type();
  display_cardnum();

   
}
 
/*初始化函数---------------------------------------------------------------------*/
void InitFunc(void)
{
	K1=1;
	K2=1;
	K3=1;
	K4=1;
	BLK=0;
	BLK=1;
	lcm_init();			//液晶初始化
	lcm_clr();            //清屏
//	init_ser();
//	Timer0Init();
}

void  InitializeSystem()   //定时器和波特率
{
	P0 = 0xFF; P1 = 0xFF; P3 = 0xFF;P2 = 0xFF;
    init_ser();
	Timer0Init();

//
//  TMOD=0x21;		   //设T0为方式1，GATE=1；
//	SCON=0x50;
//	TH1=0xF4;          //波特率为2400bps
//	TL1=0xF4;
//	TH0=0;
//	TL0=0; 
//	TR0=1;  
//	ET0=1;             //允许T0中断
//	TR1=1;			   //开启定时器
//	TI=1;
//	EA=1;			   	//开启总中断
//	
//	ES = 1;
//	RI = 1;

}
	   
 
void main( )
{    
      
     InitFunc();		   //液晶
     InitializeSystem( );  //系统初始化，设置波特率，开定时器中断
     PcdReset();		   //复位RC522，成功返回MI_OK
     PcdAntennaOff(); 	   //关闭天线
     PcdAntennaOn(); 	   //开启天线，每次启动或关闭天线发射之间应至少有1ms的间隔
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
	         Analysis_data();         //解析数据
	         if(Timer0_1sFlag==1)
	     	  {
			  Timer0_1sFlag=0;
			  read_data(); 
			  }
		      DelayM(500);
		      data_display_1();//数据显示
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
               status = PcdRequest(PICC_REQALL, g_ucTempbuf);//寻卡
               if (status != MI_OK)
               {    
                InitializeSystem( );	//初始化
                PcdReset();			    //复位
                PcdAntennaOff(); 		//关天线
                PcdAntennaOn(); 		//开天线
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







 
 
