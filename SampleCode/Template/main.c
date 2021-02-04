/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include "nuc980.h"
#include "sys.h"

#include "gpio.h"
#include "uart.h"
#include "etimer.h"

#include	"project_config.h"


/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ D E F I N I T I O N S ______________________________________________*/

#define TEST_BUFFER_SIZE   						(50000)
#define NON_CACHE_MEMORY_MASK   	 			(0x80000000)



/*_____ M A C R O S ________________________________________________________*/

volatile uint32_t BitFlag = 0;

uint32_t counter_tick = 0;

/*_____ F U N C T I O N S __________________________________________________*/

void tick_counter(void)
{
	counter_tick++;
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}


void compare_buffer(uint8_t *src, uint8_t *des, int nBytes)
{
    uint16_t i = 0;	
	
    for (i = 0; i < nBytes; i++)
    {
        if (src[i] != des[i])
        {
            printf("error idx : %4d : 0x%2X , 0x%2X\r\n", i , src[i],des[i]);
			set_flag(flag_error , ENABLE);
        }
    }

	if (!is_flag_set(flag_error))
	{
    	printf("%s finish \r\n" , __FUNCTION__);	
		set_flag(flag_error , DISABLE);
	}

}

void reset_buffer(void *dest, unsigned int val, unsigned int size)
{
    uint8_t *pu8Dest;
//    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;

	#if 1
	while (size-- > 0)
		*pu8Dest++ = val;
	#else
	memset(pu8Dest, val, size * (sizeof(pu8Dest[0]) ));
	#endif
	
}

void copy_buffer(void *dest, void *src, unsigned int size)
{
    uint8_t *pu8Src, *pu8Dest;
    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;
    pu8Src  = (uint8_t *)src;


	#if 0
	  while (size--)
	    *pu8Dest++ = *pu8Src++;
	#else
    for (i = 0; i < size; i++)
        pu8Dest[i] = pu8Src[i];
	#endif
}

void dump_buffer(uint8_t *pucBuff, int nBytes)
{
    uint16_t i = 0;
    
    printf("dump_buffer : %2d\r\n" , nBytes);    
    for (i = 0 ; i < nBytes ; i++)
    {
        printf("0x%2X," , pucBuff[i]);
        if ((i+1)%8 ==0)
        {
            printf("\r\n");
        }            
    }
    printf("\r\n\r\n");
}

void  dump_buffer_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0)
    {
        printf("0x%04X  ", nIdx);
        for (i = 0; i < 16; i++)
            printf("%02X ", pucBuff[nIdx + i]);
        printf("  ");
        for (i = 0; i < 16; i++)
        {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                printf("%c", pucBuff[nIdx + i]);
            else
                printf(".");
            nBytes--;
        }
        nIdx += 16;
        printf("\n");
    }
    printf("\r\n");
}


void CacheTest(void)
{

   uint8_t bTestBuffer[TEST_BUFFER_SIZE];
   uint32_t bNonCacheTestBuffer = NON_CACHE_MEMORY_MASK | (uint32_t)bTestBuffer;
   uint8_t bTestPattern[TEST_BUFFER_SIZE];
   int i;
   static uint32_t u32Count;
   static uint32_t u32Times;

   
   for (i = 0; i < TEST_BUFFER_SIZE; i++)
   {
      bTestPattern[i] = (uint8_t)i;
   }

   u32Count = 0;
   while (TRUE)
   {
      bTestBuffer[0] = 0x55;
      bTestBuffer[1] = 0xAA;
      bTestBuffer[2] = 0xAB;
      bTestBuffer[3] = 0xEF;

      memset((uint8_t*)bNonCacheTestBuffer, 0, TEST_BUFFER_SIZE);
      memcpy((uint8_t*)bNonCacheTestBuffer, bTestPattern, TEST_BUFFER_SIZE);
      if (memcmp((uint8_t*)bNonCacheTestBuffer, bTestPattern, TEST_BUFFER_SIZE) != 0)
      {
         printf("\nData compare fail!\r\n");
         printf("\nTEST pattern\r\n");		 
         dump_buffer_hex(bTestPattern, TEST_BUFFER_SIZE);
         printf("\nTEST non cache buffer\r\n");			 
         dump_buffer_hex((uint8_t*)bNonCacheTestBuffer, TEST_BUFFER_SIZE);
         printf("\n*** END! ***\r\n");
         while (TRUE);
      }
      else
      {
//         printf("Testing...\r\n");
//         switch (u32Count % 1000)
//         {
//            case 0:
//				printf("Testing...0\r\n");
//               break;
//            case 250:
//				printf("Testing...250\r\n");
//               break;
//            case 500:
//				printf("Testing...500\r\n");
//               break;
//            case 750:
//				printf("Testing...750\r\n");
//               break;
//         }

		 if ((u32Count % 1000) == 0)
		 {
			printf("Times...%d\r\n" , u32Times++);
		 }
		u32Count++;
		PC11 ^= 1;
      }
      //delay10ms(1);
      ETIMER_Delay(4,1000);
   }
}



void ETMR0_IRQHandler(void)
{
	static uint32_t LOG = 0;
	
    if(ETIMER_GetIntFlag(0) == 1)
    {
   	 	// clear timer interrupt flag		
        ETIMER_ClearIntFlag(0);
		tick_counter();

		if ((get_tick() % 1000) == 0)
		{
//        	printf("%s : %4d\r\n",__FUNCTION__,LOG++);
			PC3 ^= 1;
		}

		if ((get_tick() % 50) == 0)
		{

		}	
    }
}

void ETIMER0_Init(void)
{
    // Set timer frequency to 1HZ
    ETIMER_Open(0, ETIMER_PERIODIC_MODE, 1000);

    // Enable timer interrupt
    ETIMER_EnableInt(0);
    sysInstallISR(IRQ_LEVEL_1, IRQ_TIMER0, (PVOID)ETMR0_IRQHandler);
    sysSetLocalInterrupt(ENABLE_IRQ);
    sysEnableInterrupt(IRQ_TIMER0);

    // Start Timer 0
    ETIMER_Start(0);
}

void GPIO_Init (void)
{

    GPIO_SetMode(PC, BIT3, GPIO_MODE_OUTPUT);

    GPIO_SetMode(PC, BIT11, GPIO_MODE_OUTPUT);	
}


void UART_Init(void)
{

    /* GPF11, GPF12 */
    outpw(REG_SYS_GPF_MFPH, (inpw(REG_SYS_GPF_MFPH) & 0xfff00fff) | 0x11000);   // UART0 multi-function

    UART_Open(UART0, 115200);	
}


void SYS_Init(void)
{

    sysDisableCache();
    sysFlushCache(I_D_CACHE);
    sysEnableCache(CACHE_WRITE_BACK);


    /* enable UART0 clock */
    outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | 0x10000);

    // Enable ETIMER0 engine clock
    outpw(REG_CLK_PCLKEN0, inpw(REG_CLK_PCLKEN0) | (1 << 8));

    outpw(REG_CLK_HCLKEN, inpw(REG_CLK_HCLKEN)|(1<<11)); //Enable GPIO engine

}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
    SYS_Init();

	GPIO_Init();
    UART_Init();

    ETIMER0_Init();
	

	CacheTest();

    while(1)
    {

    }

}
