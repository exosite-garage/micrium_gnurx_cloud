;********************************************************************************************************
;                                         BOARD SUPPORT PACKAGE
;
;                            (c) Copyright 2010; Micrium, Inc.; Weston, FL
;                    All rights reserved.  Protected by international copyright laws.
;
;                                              Renesas RX
;
; File    : NET_BSP_A.SRC
;********************************************************************************************************


    .include    "os_cpu_a.inc"          ; Include OS_ISR_ENTER and OS_ISR_EXIT macros

    .global     _AppPWM_ISR
    .global     _AppPWM_ISR_ASM
    .global     _AppADC_ISR
    .global     _AppADC_ISR_ConversionHandler

;/*$PAGE*/
;********************************************************************************************************
;                                             MTU2 TGIB4 ISR
;********************************************************************************************************

    .section    .text

_AppPWM_ISR_ASM:
    OS_CTX_SAVE                         ; Save processor registers on the stack

    MOV.L   #_AppPWM_ISR, R5
    JSR     R5

    OS_CTX_RESTORE                      ; Restore processor registers from stack
    
    RTE

_AppADC_ISR:
    OS_ISR_ENTER                        ; Save context & Notify uC/OS about ISR

    MOV.L   #_AppADC_ISR_ConversionHandler, R5
    JSR     R5

    OS_ISR_EXIT                         ; Restore context

    .END

