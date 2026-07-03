/******************************************************************************/
/**
 * @file LedPwm.h
 * @addtogroup LedPwm
 * @author Luis Eduardo
 * @details
 * @version 1
 / @} DOXYGEN GROUP TAG END OF FILE */

#ifndef LED_PWM_H
#define LED_PWM_H

/*******************************************************************************
 * INCLUDES NECESSARIOS
 ******************************************************************************/
#include "AssertTypes.h"
#include "Bsp.h" // Necessario para reutilizar o enum bspLed_t

/*******************************************************************************
 * CONFIGURACOES
 ******************************************************************************/

/*******************************************************************************
 * DEFINES PUBLICOS
 ******************************************************************************/

/*******************************************************************************
 * TIPOS DE DADOS PUBLICOS
 ******************************************************************************/

/*******************************************************************************
 * PROTOTIPOS PUBLICOS
 ******************************************************************************/
void LedPwm_Init(void);
void LedPwm_SetActiveLed(bspLed_t ledChannel);
bspLed_t LedPwm_GetActiveLed(void);
void LedPwm_UpdateActiveLedIntensity(u8 dutyPercent);
void LedPwm_RefreshAll(void);
u8 LedPwm_GetLedDuty(bspLed_t ledChannel);
#endif /* LED_PWM_H */
/** @} */
