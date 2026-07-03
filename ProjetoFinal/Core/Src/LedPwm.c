/******************************************************************************/
/**
 * @file LedPwm.c
 * @addtogroup LedPwm
 * @author Luis Eduardo
 * @details
 * @version 1
 / @} DOXYGEN GROUP TAG END OF FILE */

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "LedPwm.h"

/*******************************************************************************
 * DEFINES LOCAIS
 ******************************************************************************/
/// Valor percentual padrao para inicializacao dos LEDs (Desligados)
#define dLED_DEFAULT_DUTY_PERCENT 0

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/
/// Variaveis de controle de estado e brilho dos LEDs
static struct
{
    /// Armazena qual LED esta selecionado atualmente para sofrer alteracoes
    bspLed_t activeLed;

    /// Memoriza o ultimo brilho do LED 1
    u8 led1Duty;

    /// Memoriza o ultimo brilho do LED 2
    u8 led2Duty;

    /// Memoriza o ultimo brilho do LED 3
    u8 led3Duty;
} ledPwm;

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/** @brief Inicializa o estado do modulo de controle de LEDs.
 * @retval Nenhum.
 ******************************************************************************/
void LedPwm_Init(void)
{
    // O projeto especifica que comeca com o LED 1 ativo
    ledPwm.activeLed = eBSP_LED_1;

    // Todos os LEDs comecam apagados
    ledPwm.led1Duty = dLED_DEFAULT_DUTY_PERCENT;
    ledPwm.led2Duty = dLED_DEFAULT_DUTY_PERCENT;
    ledPwm.led3Duty = dLED_DEFAULT_DUTY_PERCENT;

    // Forca a atualizacao inicial no hardware
    LedPwm_RefreshAll();
}

/******************************************************************************/
/** @brief Modifica qual eh o LED ativo que respondera ao potenciometro.
 * @param ledChannel: O canal do LED que passara a ser o ativo.
 * @retval Nenhum.
 ******************************************************************************/
void LedPwm_SetActiveLed(bspLed_t ledChannel)
{
    if(ledChannel < eBSP_NUMBER_OF_LEDS)
    {
        ledPwm.activeLed = ledChannel;
    }
}

/******************************************************************************/
/** @brief Retorna qual o LED esta ativo no momento.
 * @retval O enum correspondente ao LED ativo.
 ******************************************************************************/
bspLed_t LedPwm_GetActiveLed(void)
{
    return ledPwm.activeLed;
}

/******************************************************************************/
/** @brief Atualiza o duty cycle apenas do LED que esta selecionado como ativo.
 * @param dutyPercent: Nova intensidade em porcentagem (0 a 100%).
 * @retval Nenhum.
 * @details Os outros LEDs mantem estritamente o seu brilho anterior na memoria.
 ******************************************************************************/
void LedPwm_UpdateActiveLedIntensity(u8 dutyPercent)
{
    // Atualiza apenas o LED que esta ativo na maquina de estados interna
    switch(ledPwm.activeLed)
    {
        case eBSP_LED_1:
            ledPwm.led1Duty = dutyPercent;
            Bsp_SetLedPwm(eBSP_LED_1, ledPwm.led1Duty);
            break;

        case eBSP_LED_2:
            ledPwm.led2Duty = dutyPercent;
            Bsp_SetLedPwm(eBSP_LED_2, ledPwm.led2Duty);
            break;

        case eBSP_LED_3:
            ledPwm.led3Duty = dutyPercent;
            Bsp_SetLedPwm(eBSP_LED_3, ledPwm.led3Duty);
            break;

        default:
            break;
    }
}

/******************************************************************************/
/** @brief Sincroniza os valores em memoria com os registradores fisicos da BSP.
 * @retval Nenhum.
 ******************************************************************************/
void LedPwm_RefreshAll(void)
{
    Bsp_SetLedPwm(eBSP_LED_1, ledPwm.led1Duty);
    Bsp_SetLedPwm(eBSP_LED_2, ledPwm.led2Duty);
    Bsp_SetLedPwm(eBSP_LED_3, ledPwm.led3Duty);
}

/******************************************************************************/
/** @brief Recupera o valor atual de duty cycle de um LED específico.
 * @param ledChannel: Canal do LED desejado.
 * @retval Valor de 0 a 100% do duty cycle em memória.
 ******************************************************************************/
u8 LedPwm_GetLedDuty(bspLed_t ledChannel)
{
    u8 duty = 0;
    switch(ledChannel)
    {
        case eBSP_LED_1: duty = ledPwm.led1Duty; break;
        case eBSP_LED_2: duty = ledPwm.led2Duty; break;
        case eBSP_LED_3: duty = ledPwm.led3Duty; break;
        default: break;
    }
    return duty;
}
/*******************************************************************************
 * FUNCOES LOCAIS
 ******************************************************************************/

/** @} */
