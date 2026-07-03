/******************************************************************************/
/**
 * @file Bsp.c
 * @addtogroup Bsp
 * @author Luis Eduardo
 * @details
 * @version 1
 / @} DOXYGEN GROUP TAG END OF FILE */

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "Bsp.h"
#include "main.h"
#include <string.h>

/*******************************************************************************
 * DEFINES LOCAIS
 ******************************************************************************/
/// Timeout para leitura do ADC em milissegundos
#define dADC_TIMEOUT_MS  10
/// Timeout para envio da UART em milissegundos
#define dUART_TIMEOUT_MS 100
/// Porcentagem maxima
#define dPERCENTAGE_MAX 100
/// Quantidade de bytes para UART
#define dUART_RX_BYTES_QTY 1

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/
/// Variaveis internas de controle da Bsp
static struct
{
    /// Flag para amostragem do ADC (5ms)
    bool isSamplingReady;

    /// Flag para detecao final do botao apos debounce
    bool isButtonPressed;

    /// Flag de recepcao da UART
    bool isUartDataReady;

    /// Buffer de recepcao de 1 byte da USART3
    u8 uartRxBuffer;
} bsp;

/* Handles gerados pelo STM32CubeMX para a F767ZI */
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim6;   // Timer Amostragem (5 ms)
extern TIM_HandleTypeDef htim7;   // Timer Debounce
extern TIM_HandleTypeDef htim2;   // Timer PWM dos LEDs
extern UART_HandleTypeDef huart3; // USART3 (ST-Link VCP)

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/** @brief Inicializa perifericos dependentes de interrupcao (Timers e UART).
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_Init(void)
{
    bsp.isSamplingReady = false;
    bsp.isButtonPressed = false;
    bsp.isUartDataReady = false;
    bsp.uartRxBuffer = 0;

    /* Inicia o Timer de Amostragem do ADC */
    HAL_TIM_Base_Start_IT(&htim6);

    /* Inicia os 3 canais de PWM  */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);

    /* Inicia a recepcao da USART3 via Interrupcao (recebendo 1 caractere) */
    HAL_UART_Receive_IT(&huart3, &bsp.uartRxBuffer, dUART_RX_BYTES_QTY);
}

/******************************************************************************/
/** @brief Retorna o estado da flag de amostragem.
 * @retval true se o timer estourou, false caso contrario.
 ******************************************************************************/
bool Bsp_GetSamplingFlag(void)
{
    return bsp.isSamplingReady;
}

/******************************************************************************/
/** @brief Limpa a flag de amostragem do ADC.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_ClearSamplingFlag(void)
{
    bsp.isSamplingReady = false;
}

/******************************************************************************/
/** @brief Executa o polling do ADC de forma manual.
 * @retval Valor de conversao do ADC.
 ******************************************************************************/
u16 Bsp_ReadAdc(void)
{
    u16 val = 0;

    HAL_ADC_Start(&hadc1);
    if(HAL_ADC_PollForConversion(&hadc1, dADC_TIMEOUT_MS) == HAL_OK)
    {
        val = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);

    return val;
}

/******************************************************************************/
/** @brief Aplica o valor de duty cycle ao LED selecionado.
 * @param ledChannel: Enum correspondente ao LED ativo.
 * @param dutyPercent: Valor percentual (0 a 100%).
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_SetLedPwm(bspLed_t ledChannel, u8 dutyPercent)
{
    /* Pegamos o valor maximo de ARR (periodo) diretamente do handle do Timer
       para evitar numeros magicos e garantir portabilidade. */
    u32 maxArr = __HAL_TIM_GET_AUTORELOAD(&htim2);
    u32 ccrValue = (maxArr * dutyPercent) / dPERCENTAGE_MAX;

    switch(ledChannel)
    {
        case eBSP_LED_1:
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ccrValue);
            break;
        case eBSP_LED_2:
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, ccrValue);
            break;
        case eBSP_LED_3:
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, ccrValue);
            break;
        default:
            break;
    }
}

/******************************************************************************/
/** @brief Retorna a flag do botao (debounce concluido).
 * @retval true se pressionado, false caso contrario.
 ******************************************************************************/
bool Bsp_GetButtonFlag(void)
{
    return bsp.isButtonPressed;
}

/******************************************************************************/
/** @brief Limpa a flag do botao.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_ClearButtonFlag(void)
{
    bsp.isButtonPressed = false;
}

/******************************************************************************/
/** @brief Retorna a flag de recebimento de dado na UART.
 * @retval true se chegou, false caso contrario.
 ******************************************************************************/
bool Bsp_GetUartRxFlag(void)
{
    return bsp.isUartDataReady;
}

/******************************************************************************/
/** @brief Limpa a flag da UART.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_ClearUartRxFlag(void)
{
    bsp.isUartDataReady = false;
}

/******************************************************************************/
/** @brief Resgata o ultimo byte lido via USART3.
 * @retval O caractere ASCII lido.
 ******************************************************************************/
u8 Bsp_GetUartRxData(void)
{
    return bsp.uartRxBuffer;
}

/******************************************************************************/
/** @brief Transmite uma string inteira pela USART3 usando Polling.
 * @param str: Ponteiro para a string formatada.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_TransmitUartString(const char *str)
{
    u16 len = strlen(str);
    HAL_UART_Transmit(&huart3, (u8*)str, len, dUART_TIMEOUT_MS);
}

/*******************************************************************************
 * FUNCOES LOCAIS (CALLBACKS DA HAL)
 ******************************************************************************/

/******************************************************************************/
/** @brief Callback de interrupcao de pinos externos (EXTI).
 * @param GPIO_Pin: Pino que gerou a interrupcao.
 * @retval Nenhum.
 ******************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    /* PC13 eh o botao azul de usuario (USER_Btn) da Nucleo-F767ZI */
    if(GPIO_Pin == USER_Btn_Pin)
    {
        /* Inicia o timer de debounce para nao usar Delay (nao bloqueante) */
        HAL_TIM_Base_Start_IT(&htim7);
    }
}

/******************************************************************************/
/** @brief Callback padrao da HAL para estouro de Timers.
 * @param htim: ponteiro para o timer que gerou a interrupcao.
 * @retval Nenhum.
 ******************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // Verifica se foi o timer de amostragem (TIM6)
    if(htim->Instance == TIM6)
    {
        bsp.isSamplingReady = true;
    }
    // Verifica se foi o timer de debounce (TIM7)
    else if(htim->Instance == TIM7)
    {
        // Para o timer para ele nao ficar estourando de novo
        HAL_TIM_Base_Stop_IT(&htim7);

        // Confirma se o botao realmente ainda esta pressionado (filtro de ruido)
        if(HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin) == GPIO_PIN_SET)
        {
            bsp.isButtonPressed = true;
        }
    }
}

/******************************************************************************/
/** @brief Callback padrao da HAL para recepcao da UART.
 * @param huart: ponteiro para a uart que gerou a interrupcao.
 * @retval Nenhum.
 ******************************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART3)
    {
        bsp.isUartDataReady = true;

        // Reativa a interrupcao imediatamente para receber o proximo caractere
        HAL_UART_Receive_IT(&huart3, &bsp.uartRxBuffer, dUART_RX_BYTES_QTY );
    }
}
/** @} */
