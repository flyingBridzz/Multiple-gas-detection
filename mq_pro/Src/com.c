#include "com.h"

uint8_t sendbyte(uint8_t ch)
{
	if((USART3->SR&0x0080) == 0x0080)
	{
		USART3->DR = ch;
		return 0;//数据发送完成
	}else{
		return 1;//数据未发送
	}
}

void sendfloat(float num)
{
	uint8_t i;
	uint8_t f[2]={0};
	uint32_t tmp;
	tmp = num*100;
	f[0] = tmp/100;
	f[1] = tmp%100;
	for(i=0;i<2;i++)
	{
		while(sendbyte(f[i]));
	}
}

void sendframe(float a,float b,float c)
{
	sendbyte(0xff);
	sendfloat(a);
	sendfloat(b);
	sendfloat(c);
}
