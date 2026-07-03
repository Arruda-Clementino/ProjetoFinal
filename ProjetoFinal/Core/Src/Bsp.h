/******************************************************************************/
/**
 * @file Bsp.h
 * @addtogroup Bsp
 * @author Luis Eduardo
 * @details
 * @version 1
 / @} DOXYGEN GROUP TAG END OF FILE */
#ifndef BSP_H
#define BSP_H

/*******************************************************************************
 * INCLUDES NECESSARIOS
 ******************************************************************************/

#include "AssertTypes.h"

/*******************************************************************************
 * CONFIGURACOES
 ******************************************************************************/
/// Timeout padrao para leitura em Polling do ADC
#define dBSP_ADC_TIMEOUT_MS 10

/*******************************************************************************
 * DEFINES PUBLICOS
 ******************************************************************************/

/*******************************************************************************
 * TIPOS DE DADOS PUBLICOS
 ******************************************************************************/
/// Identifica qual canal de LED o PWM deve alterar
typedef enum
{
    eBSP_LED_1,
    eBSP_LED_2,
    eBSP_LED_3,
    eBSP_NUMBER_OF_LEDS
} bspLed_t;

/*******************************************************************************
 * PROTOTIPOS PUBLICOS
 ******************************************************************************/
void Bsp_Init(void);

bool Bsp_GetSamplingFlag(void);
void Bsp_ClearSamplingFlag(void);

u16 Bsp_ReadAdc(void);
void Bsp_SetLedPwm(bspLed_t ledChannel, u8 dutyPercent);

bool Bsp_GetButtonFlag(void);
void Bsp_ClearButtonFlag(void);

bool Bsp_GetUartRxFlag(void);
void Bsp_ClearUartRxFlag(void);
u8 Bsp_GetUartRxData(void);
void Bsp_TransmitUartString(const char *str);

#endif /* BSP_H */
/** @} */
