/******************************************************************************/
/**
 * @file Bsp.c
 * @addtogroup Bsp
 * @author Luis Eduardo
 * @details
 * @version 1
 * @} DOXYGEN GROUP TAG END OF FILE
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "Bsp.h"
#include "main.h"
#include <string.h>
#include <stdbool.h> /* Adicionado para suporte ao tipo bool */
#include <stdint.h>  /* Adicionado para os tipos padroes do C */

/*******************************************************************************
 * DEFINES LOCAIS
 ******************************************************************************/
#define dADC_TIMEOUT_MS     10
#define dUART_TIMEOUT_MS    100
#define dPERCENTAGE_MAX     100
#define dUART_RX_BYTES_QTY  1

// Resolução do nosso PWM por software (20 passos de 5% cada)
#define dPWM_SOFT_STEPS     20

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/
/// Variaveis internas de controle da Bsp
static struct
{
    bool isSamplingReady;
    bool isButtonPressed;
    bool isUartDataReady;
    u8 uartRxBuffer;

    /* === ADICIONE ESTAS LINHAS AQUI === */
    u8 ledDuties[eBSP_NUMBER_OF_LEDS]; // Memoriza o duty cycle (0-100) de cada LED
    u8 pwmCounter;                     // Contador de ciclos do PWM virtual (0-100)
} bsp;

/* Handles gerados pelo STM32CubeMX para a F767ZI */
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim6;   // Timer Amostragem (5 ms)
extern TIM_HandleTypeDef htim7;   // Timer Debounce
extern TIM_HandleTypeDef htim3;   // Timer PWM dos LEDs
extern UART_HandleTypeDef huart3; // USART3 (ST-Link VCP)

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/** @brief Inicializa perifericos. UART e PWM por Polling. Apenas TIM6 usa IT.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_Init(void)
{
    bsp.isSamplingReady = false;
    bsp.isButtonPressed = false;
    bsp.isUartDataReady = false;
    bsp.uartRxBuffer = 0;
    bsp.pwmCounter = 0;

    // Inicializa as memórias de brilho em 0%
    for(int i = 0; i < eBSP_NUMBER_OF_LEDS; i++)
    {
        bsp.ledDuties[i] = 0;
    }

    /* Inicia o Timer de Amostragem e base do PWM virtual (TIM6) */
    HAL_TIM_Base_Start_IT(&htim6);

    /* === NÃO INICIAMOS O TIM3 PWM AQUI POIS SERÁ POR SOFTWARE === */

    /* Ativa a escuta da UART */
    HAL_UART_Receive_IT(&huart3, &bsp.uartRxBuffer, dUART_RX_BYTES_QTY);
}

/******************************************************************************/
/** @brief Aplica o valor de duty cycle ao LED.
 * PWM manual via registro CCR (Polling).
 ******************************************************************************/
void Bsp_SetLedPwm(bspLed_t ledChannel, u8 dutyPercent)
{
	u32 maxArr = __HAL_TIM_GET_AUTORELOAD(&htim3);
	    u32 ccrValue = (maxArr * dutyPercent) / dPERCENTAGE_MAX;

	    switch(ledChannel)
	    {
	        case eBSP_LED_1:
	            // Se o LED1 (Verde) virou o Canal 3 do TIM3 no CubeMX:
	            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, ccrValue);
	            break;
	        case eBSP_LED_2:
	            // Canal correspondente ao LED2 externo ou interno
	            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, ccrValue);
	            break;
	        case eBSP_LED_3:
	            // Se o LED3 (Vermelho) virou o Canal 2 do TIM3 no CubeMX:
	            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, ccrValue);
	            break;
	        default:
	            break;
	    }
}

/******************************************************************************/
/** @brief Retorna o estado atual da flag de amostragem.
 * @retval true se a amostragem estiver pronta, false caso contrário.
 ******************************************************************************/
bool Bsp_GetSamplingFlag(void)
{
    return bsp.isSamplingReady;
}

/******************************************************************************/
/** @brief Limpa a flag de amostragem.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_ClearSamplingFlag(void)
{
    bsp.isSamplingReady = false;
}

/******************************************************************************/
/** @brief Realiza a leitura do canal ADC via polling.
 * @retval Valor digital lido do ADC (12 bits).
 ******************************************************************************/
u16 Bsp_ReadAdc(void)
{
	u16 adcValue = 0;
	HAL_ADC_Start(&hadc1);
	if (HAL_ADC_PollForConversion(&hadc1, dADC_TIMEOUT_MS) == HAL_OK)
	{
		adcValue = (u16)HAL_ADC_GetValue(&hadc1);
	}
	HAL_ADC_Stop(&hadc1);
	return adcValue;
}

/******************************************************************************/
/** @brief Transmite uma string via UART usando polling.
 * @param str: Ponteiro para a string a ser transmitida.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_TransmitUartString(const char *str)
{
    HAL_UART_Transmit(&huart3, (uint8_t*)str, strlen(str), dUART_TIMEOUT_MS);
}

/******************************************************************************/
/** @brief Retorna o estado da flag do botão.
 * @retval true se o botão foi pressionado, false caso contrário.
 ******************************************************************************/
bool Bsp_GetButtonFlag(void)
{
    return bsp.isButtonPressed;
}

/******************************************************************************/
/** @brief Limpa a flag de detecção do botão.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_ClearButtonFlag(void)
{
    bsp.isButtonPressed = false;
}

/******************************************************************************/
/** @brief Verifica se há dados novos na UART.
 * @retval true se houver dados, false caso contrário.
 ******************************************************************************/
bool Bsp_GetUartRxFlag(void)
{
    return bsp.isUartDataReady;
}

/******************************************************************************/
/** @brief Limpa a flag de recepção da UART.
 * @retval Nenhum.
 ******************************************************************************/
void Bsp_ClearUartRxFlag(void)
{
    bsp.isUartDataReady = false;
}

/******************************************************************************/
/** @brief Retorna o dado recebido pela UART.
 * @retval Byte recebido.
 ******************************************************************************/
u8 Bsp_GetUartRxData(void)
{
    return bsp.uartRxBuffer;
}

/**
 * @brief Controla os pinos físicos dos LEDs internos soldados na placa Nucleo
 */
void Bsp_LedInternal_Write(bspLed_t ledChannel, bool state)
{
    GPIO_PinState pinState = (state == true) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    switch(ledChannel)
    {
        case eBSP_LED_1:
            HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, pinState); // LED Verde da placa
            break;
        case eBSP_LED_2:
            // Caso sua placa use a macro do LED azul padrão (geralmente LD2_Pin ou substituído)
            // Se o CubeMX gerou como LD2_Pin, use: HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, pinState);
            break;
        case eBSP_LED_3:
            HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, pinState); // LED Vermelho da placa
            break;
        default:
            break;
    }
}

/**
 * @brief Executa o ciclo do PWM virtual. Deve ser chamada a cada estouro de amostragem.
 */
void Bsp_PwmSoftware_Process(void)
{
    // Incrementa o contador do PWM de 0 a 100
    bsp.pwmCounter++;
    if(bsp.pwmCounter >= 100)
    {
        bsp.pwmCounter = 0;
    }

    // Compara o contador atual com o Duty Cycle definido para cada LED
    for(int i = 0; i < eBSP_NUMBER_OF_LEDS; i++)
    {
        if(bsp.pwmCounter < bsp.ledDuties[i])
        {
            Bsp_LedInternal_Write(i, true);  // Liga se estiver dentro do tempo do Duty Cycle
        }
        else
        {
            Bsp_LedInternal_Write(i, false); // Desliga no resto do período
        }
    }
}

/*******************************************************************************
 * FUNCOES LOCAIS (CALLBACKS DA HAL)
 ******************************************************************************/

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == USER_Btn_Pin)
    {
        HAL_TIM_Base_Start_IT(&htim7);
    }
}

/**
 * @brief Chamado a cada 5 ms pelo estouro do TIM6
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM6)
    {
        // Sinaliza para a main.c coletar o ADC
        bsp.isSamplingReady = true;

        // ---- Lógica do PWM por Software (Virtual) ----
        bsp.pwmCounter++;
        if(bsp.pwmCounter >= dPWM_SOFT_STEPS)
        {
            bsp.pwmCounter = 0;
        }

        // Converte a contagem atual (0 a 20) para uma escala de porcentagem (0 a 100)
        u8 currentProgressPercent = bsp.pwmCounter * (100 / dPWM_SOFT_STEPS);

        // LED 1 (Verde interno da placa - LD1_Pin)
        if (currentProgressPercent < bsp.ledDuties[eBSP_LED_1]) {
            HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
        }

        // LED 3 (Vermelho interno da placa - LD3_Pin)
        if (currentProgressPercent < bsp.ledDuties[eBSP_LED_3]) {
            HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
        }
    }
    else if(htim->Instance == TIM7)
    {
        HAL_TIM_Base_Stop_IT(&htim7);
        if(HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin) == GPIO_PIN_SET)
        {
            bsp.isButtonPressed = true;
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART3)
    {
        bsp.isUartDataReady = true;
        HAL_UART_Receive_IT(&huart3, &bsp.uartRxBuffer, dUART_RX_BYTES_QTY);
    }
}

/** @} */
