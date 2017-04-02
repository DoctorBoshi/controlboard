#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/itp.h"
#include "ite/ith.h"
#include <pthread.h>
#include "openrtos/FreeRTOS.h"
#include "openrtos/task.h"

static int clk = 0;
static int cnt = 0;

unsigned int PalGetClock(void)
{
    return xTaskGetTickCount();
}

unsigned long PalGetDuration(unsigned int clock)
{
    return (xTaskGetTickCount() - clock) / portTICK_PERIOD_MS;
}

void timer_isr(void* data)
{
    uint32_t timer = (uint32_t)data;
    
    ithPrintf("\tTimer%d Interrupt occur, clk=%d, IntrState=0x%08X, cnt=%d\n", timer + 1, PalGetDuration(clk), ithTimerGetIntrState(), ++cnt);
    //ithPrintf("T%d\n", timer+1);
    //ithPrintf("\tTimer%d isr, cnt=%d\n", timer + 1, ++cnt);
    //printf("\tTimer%d Interrupt occur, clk=%d, IntrState=0x%08X, cnt=%d\n", timer + 1, PalGetDuration(clk), ithTimerGetIntrState(), ++cnt);
    clk = PalGetClock();
    ithTimerClearIntr(timer);

}


void* Test_IRQ(void* arg)
{
    int status;
    int i = 0;
    int us = 0;    
    for (us=10000; us<=1000000; us*=10)
    {
        // Interrupt test
        printf("\n###### %dms timeout testing ######\n", us/1000);

        for(i=ITH_TIMER1; i<=ITH_TIMER8; i++)
        {
            // timer used in
			// 1: backlight
			// 2: no use
			// 3: print_buffer
			// 4: RTC in alpha
			// 5: no use
			// 6: Operation System
            // 7: no use
            // 8: no use
            if (/*i == ITH_TIMER1 || i == USEDTIMER || */i == ITH_TIMER6)
                continue;
                
            ithTimerReset(i);

            {
                // Initialize Timer IRQ
                ithIntrDisableIrq(ITH_INTR_TIMER1 + i);
                ithIntrClearIrq(ITH_INTR_TIMER1 + i);

                // register Timer Handler to IRQ
                ithIntrRegisterHandlerIrq(ITH_INTR_TIMER1 + i, timer_isr, (void*)i);

                // set Timer IRQ to edge trigger
                ithIntrSetTriggerModeIrq(ITH_INTR_TIMER1 + i, ITH_INTR_EDGE);

                // set Timer IRQ to detect rising edge
                ithIntrSetTriggerLevelIrq(ITH_INTR_TIMER1 + i, ITH_INTR_HIGH_RISING);

                // Enable Timer IRQ
                ithIntrEnableIrq(ITH_INTR_TIMER1 + i);
            }

            ithTimerSetTimeout(i, us);
            clk = PalGetClock();
            ithTimerCtrlEnable(i, ITH_TIMER_EN);

            usleep(us*10); // goal: trigger 10 times

            printf("###### Timer%d leaving\n", i+1);
            ithTimerCtrlDisable(i, ITH_TIMER_EN);
            ithTimerReset(i);

            ithIntrDisableIrq(ITH_INTR_TIMER1 + i);
            ithIntrClearIrq(ITH_INTR_TIMER1 + i);
        }
    }
    printf("end IRQ TEST\n");
    
    return NULL;
}

void* TestFunc_PWM(void* arg)
{//for 9850
    int i = 0;
    int mode;
    int pinlow;
    mode=2;
    pinlow=39;
    for(i=ITH_PWM1;i<=ITH_PWM6;i++){
        int count=0;
        if (i == ITH_TIMER6) continue;
        ithGpioSetMode( pinlow + i, mode);
        ithPwmInit(i, 100000, 10^(i+1));
        ithPwmSetDutyCycle(i,10);
        ithPwmReset(i, pinlow + i, mode);         
        for(count=0;count<=10;count++){
            sleep(1);
            printf("ITH_PWM%d enable\n",i+1);
            ithPwmEnable(i, pinlow + i, mode);
            sleep(1);
            printf("ITH_PWM%d disable\n",i+1);
            ithPwmDisable(i,pinlow + i);     
        }
    }
    return NULL;
}

void* TestFunc_64bitenable(void* arg)
{
    //test 64 bit timer
    int count;
    
    itpInit();
    printf("Hello World\n");
    ithTimerReset(ITH_TIMER3);
    ithTimerReset(ITH_TIMER4);
    ithTimerCtrlEnable(ITH_TIMER4,ITH_TIMER_EN64);
    ithTimerSetLoad(ITH_TIMER3,0x01010202);
    ithWriteRegA(ITH_TIMER_BASE + ITH_TIMER3 * 0x10 + ITH_TIMER1_MATCH1_REG, 0xffffffff);
    ithWriteRegA(ITH_TIMER_BASE + ITH_TIMER3 * 0x10 + ITH_TIMER1_MATCH2_REG, 0x00000000);    
    ithTimerSetLoad(ITH_TIMER4,255);
    printf("interrutp %08x\n",ithTimerGetIntrState());    
    ithTimerCtrlEnable(ITH_TIMER3,ITH_TIMER_EN);
    ithTimerCtrlEnable(ITH_TIMER4,ITH_TIMER_EN);
    
    //count=ithTimerGetCounter(ITH_TIMER3);
    while(1){
        printf("Timer3 %d\n",ithTimerGetCounter(ITH_TIMER3));
        printf("Timer4 %d\n\n",ithTimerGetCounter(ITH_TIMER4));
        printf("interrutp %08x\n",ithTimerGetIntrState());
        usleep(1000000);
        if(ithTimerGetCounter(ITH_TIMER4)==255)
            break;
    }
    ithTimerDisable(ITH_TIMER3);

    return NULL;
}

void* TestFunc_timeout(void* arg)
{
    /* Basic test*/
    int status;
    int i = 0;
    int j = 0;
    int us = 0;
    int clock = 0;

    for (us=10000; us<=1000000; us*=10)
    {
        // Interrupt test
        printf("\n###### %dms timeout testing ######\n", us);

        for(i=ITH_TIMER1; i<=ITH_TIMER8; i++)
        {
            // timer used in
			// 1: backlight
			// 2: no use
			// 3: print_buffer
			// 4: RTC in alpha
			// 5: no use
			// 6: Operation System
            // 7: no use
            // 8: no use
            if (/*i == ITH_TIMER1 || i == USEDTIMER || */i == ITH_TIMER6)
                continue;
                
            ithTimerReset(i);
            ithTimerClearIntr(i);
            ithTimerSetTimeout(i, us);
            clk = PalGetClock();
            ithTimerEnable(i);

            j=0;
            clock = PalGetClock();

            while(j++ < 10)
            {
                status = ithTimerGetIntrState();
                if(status)
                {
                    printf("[TIMER][%d]intr gap time = %d ms status = 0x%08x \n",i+1, PalGetDuration(clock), status);
                    clock = PalGetClock();

                }
                clock = PalGetClock();
                usleep(us); // goal: trigger 10 times
            }

            printf("###### Timer%d leaving\n", i+1);
            ithTimerCtrlDisable(i, ITH_TIMER_EN);;
            ithTimerReset(i);
            ithTimerClearIntr(i);

        }
    }
    printf("**********end***************\n");
}

void* TestFunc_count(void* arg)
{
    /* Basic test*/
    /*
    teset timer count us sleeptime
    up count the counter
    ithTimerGetTime() get time
    count sleeptime=5000 use time2-time1
    test 10 time
    */
    int status;
    int i = 0;
    unsigned int time1=0;
    unsigned int time2=0;
    int sleeptime=5000;
    
    ithTimerReset(ITH_TIMER1);//reset timer
    ithTimerCtrlEnable(ITH_TIMER1, ITH_TIMER_UPCOUNT);//up count the counter
    ithTimerSetCounter(ITH_TIMER1, 0x0);//init counter to 0
    ithTimerSetLoad(ITH_TIMER1,0x0);//set reload


    ithTimerEnable(ITH_TIMER1);//enabel time
    while(1){
        time1=ithTimerGetTime(ITH_TIMER1);
        usleep(sleeptime);
        time2=ithTimerGetTime(ITH_TIMER1);
        printf("time1-time2 = %d us\n",time2-time1);
        if(i==10)
            break;
        else
            i++;
    }
    printf("=======end test======\n");
    return ;
 
}

void* TestFunc_interrupt(void* arg)
{
    /* Basic test*/
    /*
    test interrupt
    we set timeout =6000 when count down to 0 trigger an interrupt
    */
    int status;
    int i = 0;
    unsigned int time1=0;
    unsigned int time2=0;
    int timeout=6000;
    
    ithTimerReset(ITH_TIMER1);//reset
    ithTimerSetTimeout(ITH_TIMER1, timeout);//set timeout 
    ithTimerSetMatch(ITH_TIMER1,0);//set match
    ithTimerClearIntr(ITH_TIMER1);

    ithTimerEnable(ITH_TIMER1);
    
    status=ithTimerGetIntrState();
    printf("status = 0x%08x\n",status);
    time1=ithTimerGetTime(ITH_TIMER1);
    while(1){
        
        status = ithTimerGetIntrState();
        
        printf("time = %dus status =0x%08x\n",ithTimerGetTime(ITH_TIMER1),status);
        if(status){//timeout break while
            printf("status = 0x%08x\n",status);
            ithTimerClearIntr(ITH_TIMER1);//clear interrupt
            break;
        }
    }
    time2=ithTimerGetTime(ITH_TIMER1);
    printf("time1-time2 = %d us\n",time1-time2);
    printf("time = %dus status =0x%08x\n",ithTimerGetTime(ITH_TIMER1),ithTimerGetIntrState());
    ithTimerCtrlDisable(ITH_TIMER1, ITH_TIMER_EN);
    ithTimerReset(ITH_TIMER1);
    printf("=======end test======\n");
    return ;
 
}

void* TestFunc_count_interrupt(void* arg)
{
    /* Basic test*/
    //count the interval time
    /**/
    int status;
    unsigned int time1=0;
    unsigned int time2=0;
    int timeout=5000;//us
    
    ithTimerReset(ITH_TIMER1);
    ithTimerClearIntr(ITH_TIMER1);
    ithTimerSetTimeout(ITH_TIMER1, timeout);
    
    
    ithTimerReset(ITH_TIMER2);
    ithTimerClearIntr(ITH_TIMER2);
    ithTimerCtrlEnable(ITH_TIMER2, ITH_TIMER_UPCOUNT);
    ithTimerSetCounter(ITH_TIMER2, 0x1);
    ithTimerSetLoad(ITH_TIMER2,0x0);
    
    ithTimerEnable(ITH_TIMER1);
    ithTimerEnable(ITH_TIMER2);
    
    status = ithTimerGetIntrState();
    printf("status = 0x%08x\n",status);
    time1=ithTimerGetTime(ITH_TIMER2);
    while(1){
        status = ithTimerGetIntrState();
        printf("ITH_TIMER1 count = %d   time = %dus status = 0x%08x\n",ithTimerGetCounter(ITH_TIMER1),ithTimerGetTime(ITH_TIMER2),status);
        if(status ==0x0000007){
            printf("ITH_TIMER1 is timeout(%dus) status = 0x%08x\n",timeout,status);
            ithTimerClearIntr(ITH_TIMER1);
            break;
        }
    }
    time2=ithTimerGetTime(ITH_TIMER2);
    printf("time1-time2 = %d us\n",time2-time1);
    
    ithTimerCtrlDisable(ITH_TIMER1, ITH_TIMER_EN);
    ithTimerReset(ITH_TIMER1);
    ithTimerClearIntr(ITH_TIMER1);
    
    ithTimerCtrlDisable(ITH_TIMER2, ITH_TIMER_EN);
    ithTimerReset(ITH_TIMER2);
    
    printf("=======end test======\n");
    return ;
 
}