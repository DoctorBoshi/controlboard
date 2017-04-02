/*
    FreeRTOS V8.2.3 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* Dimensions a buffer that can be used by the FreeRTOS+CLI command
interpreter.  Set this value to 1 to save RAM if FreeRTOS+CLI does not supply
the output butter.  See the FreeRTOS+CLI documentation for more information:
http://www.FreeRTOS.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/ */
#define configCOMMAND_INT_MAX_OUTPUT_SIZE           1024

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION            1
#define configUSE_IDLE_HOOK             1
#define configUSE_TICK_HOOK             0

#ifndef __ASSEMBLER__
    #ifdef __SM32__
        unsigned int ithGetCpuClock(void);
        #define configCPU_CLOCK_HZ          ( ithGetCpuClock() )
    #else
        unsigned int ithGetBusClock(void);
        #define configCPU_CLOCK_HZ          ( ithGetBusClock() )
    #endif // __SM32__
#endif // __ASSEMBLER__

#define configENABLE_BACKWARD_COMPATIBILITY 0
#define configTICK_RATE_HZ              ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES            ( 10UL )
#define configMINIMAL_STACK_SIZE        ( ( unsigned short ) 10000 )
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( CFG_OPENRTOS_HEAP_SIZE ) )
#define configMAX_TASK_NAME_LEN         ( 16 )
#define configUSE_16_BIT_TICKS          0
#define configIDLE_SHOULD_YIELD         1
#define configUSE_MUTEXES               1
#define configUSE_RECURSIVE_MUTEXES     1
#define configUSE_COUNTING_SEMAPHORES   1
#define configUSE_APPLICATION_TASK_TAG  1
#define configUSE_NEWLIB_REENTRANT      1
#define configQUEUE_REGISTRY_SIZE		10

#ifndef NDEBUG
    #define configCHECK_FOR_STACK_OVERFLOW  2
#endif

#if !defined(NDEBUG) || defined(CFG_DBG_STATS_TASK_LIST) || defined(CFG_DBG_STATS_TASK_TIME) || defined(CFG_DBG_TRACE_ANALYZER) || defined(CFG_DBG_CLI)
    #define configUSE_TRACE_FACILITY        1
#endif

#if defined(CFG_DBG_STATS_TASK_TIME) || defined(CFG_DBG_CLI)
    #define configGENERATE_RUN_TIME_STATS   1
#endif

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES       0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Software timer definitions. */
#define configUSE_TIMERS                1
#define configTIMER_TASK_PRIORITY       ( 4 )
#define configTIMER_QUEUE_LENGTH        ( 50 )
#define configTIMER_TASK_STACK_DEPTH    ( 10000 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet        1
#define INCLUDE_uxTaskPriorityGet       1
#define INCLUDE_vTaskDelete             1
#define INCLUDE_vTaskSuspend            1
#define INCLUDE_vTaskDelayUntil         1
#define INCLUDE_vTaskDelay              1
#define INCLUDE_xTaskGetCurrentTaskHandle   1
#define INCLUDE_xTaskGetIdleTaskHandle  1
#define INCLUDE_pcTaskGetTaskName       1
#define INCLUDE_xTimerPendFunctionCall  1

/* This demo makes use of one or more example stats formatting functions.  These
format the raw data provided by the uxTaskGetSystemState() function in to human
readable ASCII form.  See the notes in the implementation of vTaskList() within
FreeRTOS/Source/tasks.c for limitations. */
#define configUSE_STATS_FORMATTING_FUNCTIONS    1

/*-----------------------------------------------------------
 * Macros required to setup the timer for the run time stats.
 *-----------------------------------------------------------*/
#ifndef __ASSEMBLER__
    void vConfigureTimerForRunTimeStats( void );
    unsigned long ulGetRunTimeCounterValue( void );
    #define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() vConfigureTimerForRunTimeStats()
    #define portGET_RUN_TIME_COUNTER_VALUE() ulGetRunTimeCounterValue()
#endif

#ifndef __NDS32__
    /* Use the optimised task selection rather than the generic C code version. */
    #define configUSE_PORT_OPTIMISED_TASK_SELECTION 1

    #ifdef CFG_POWER_TICKLESS_IDLE
        #define configUSE_TICKLESS_IDLE 1
    #endif
#endif

//#define configCHECK_CRITICAL_TIME     1

#if defined(CFG_DBG_TRACE_ANALYZER) && defined(CFG_DBG_VCD)

#ifndef __ASSEMBLER__
void portTASK_SWITCHED_IN( void );
void portTASK_SWITCHED_OUT( void );
void portTASK_DELAY( void );
void portTASK_CREATE( void* xTask );
void portTASK_DELETE( void* xTask );
#endif

#define traceTASK_SWITCHED_IN()     portTASK_SWITCHED_IN()
#define traceTASK_SWITCHED_OUT()    portTASK_SWITCHED_OUT()
#define traceTASK_DELAY()           portTASK_DELAY()
#define traceTASK_CREATE(xTask)     portTASK_CREATE(xTask)
#define traceTASK_DELETE(xTask)     portTASK_DELETE(xTask)

#elif configCHECK_CRITICAL_TIME == 3

#ifndef __ASSEMBLER__
void vTaskSwitchedOut( void );
void vTaskSwitchedIn( void );
#endif

#define traceTASK_SWITCHED_OUT()    vTaskSwitchedOut()
#define traceTASK_SWITCHED_IN()     vTaskSwitchedIn()

#endif // CFG_DBG_TRACE_ANALYZER && CFG_DBG_VCD

#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
    #define portTIMER       ITH_TIMER6
    #define portTIMER_INTR  ITH_INTR_TIMER6
#else
    // Default uses ITH_TIMER8 on IT9850. Change to TIMER6 due to there's bug on TIMER7 and TIMER8 on IT9859A0.
    #define portTIMER       ITH_TIMER6
    #define portTIMER_INTR  ITH_INTR_TIMER6
#endif // (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)

#if defined(CFG_DBG_TRACE_ANALYZER) && defined(CFG_DBG_TRACE)
    #include "trcKernelPort.h"
#endif

#endif /* FREERTOS_CONFIG_H */
