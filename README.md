# NUC980_NonOS_BSP_TestSRAM
 NUC980_NonOS_BSP_TestSRAM

update @ 2021/02/04

1. use NK-RTU980 EVM (power on setting , BOOT from USB) , NUC980_NonOS_BSP , NuWriter use DDR/SRAM setting

2. initial UART for log , TIMER0 with 1 ms interrupt , GPIO (PC3 , PC11 ) for LED indicator

3. Test SRAM flow , with buffer by using memset  > memcpy > memcmp under while loop

4. print log message when test over 1000 times , and up counting

![image](https://github.com/released/NUC980_NonOS_BSP_TestSRAM/blob/main/log.jpg)

