/******************************************************************************/
/**
 * @file SerialCmd.c
 * @addtogroup SerialCmd
 * @author Luis Eduardo
 * @details
 * @version 1
 / @} DOXYGEN GROUP TAG END OF FILE */

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "SerialCmd.h"
#include "Bsp.h"
#include "LedPwm.h"
#include <string.h>

/*******************************************************************************
 * DEFINES LOCAIS
 ******************************************************************************/
/// Tamanho maximo suportado para um comando de texto recebido
#define dMAX_CMD_LENGTH 10

/// Textos esperados para selecao de cada LED
#define dCMD_LED_1      "LED1"
#define dCMD_LED_2      "LED2"
#define dCMD_LED_3      "LED3"

/// Tamanho em bytes da string de comando (4 caracteres)
#define dCMD_LENGTH     4
/// CMD vazio
#define dEMPTY_INDEX 	0
/// Deslocamento para o caractere nulo \0
#define dNULL_TERMINATOR_OFFSET 1

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/
/// Variaveis internas de controle do buffer da UART
static struct
{
    /// Buffer para acumular os caracteres recebidos
    char buffer[dMAX_CMD_LENGTH];

    /// Posicao atual de escrita no buffer
    u8 index;
} serialCmd;

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/
static void SerialCmd_EvaluateBuffer(void);
static void SerialCmd_ClearBuffer(void);

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/** @brief Inicializa o buffer de recepcao de comandos seriais.
 * @retval Nenhum.
 ******************************************************************************/
void SerialCmd_Init(void)
{
    SerialCmd_ClearBuffer();
}

/******************************************************************************/
/** @brief Processa os caracteres que chegaram pela Bsp e monta os comandos.
 * @retval Nenhum.
 * @details Esta funcao deve ser chamada frequentemente no laco principal para
 * evitar perda de dados. Nao eh bloqueante.
 ******************************************************************************/
void SerialCmd_Process(void)
{
    // Verifica se a Bsp sinalizou a chegada de um novo caractere
    if(Bsp_GetUartRxFlag() == true)
    {
        // Resgata o byte recebido
        u8 receivedChar = Bsp_GetUartRxData();

        // Limpa a flag para permitir que a Bsp sinalize o proximo caractere
        Bsp_ClearUartRxFlag();

        // Verifica se eh o caractere de terminacao (Enter - \r ou \n)
        if((receivedChar == '\r') || (receivedChar == '\n'))
        {
            // Avalia o comando contido no buffer, caso haja algo escrito
            if(serialCmd.index > dEMPTY_INDEX)
            {
                SerialCmd_EvaluateBuffer();
            }
        }
        else
        {
            // Protege o buffer contra estouro de memoria (overflow)
            if(serialCmd.index < (dMAX_CMD_LENGTH - dNULL_TERMINATOR_OFFSET))
            {
                // Salva o caractere e incrementa a posicao do cursor
                serialCmd.buffer[serialCmd.index] = (char)receivedChar;
                serialCmd.index++;
            }
            else
            {
                // Se estourou o limite, limpa o buffer para reiniciar a recepcao
                SerialCmd_ClearBuffer();
            }
        }
    }
}

/*******************************************************************************
 * FUNCOES LOCAIS
 ******************************************************************************/

/******************************************************************************/
/** @brief Avalia o conteudo atual do buffer contra os comandos validos.
 * @retval Nenhum.
 ******************************************************************************/
static void SerialCmd_EvaluateBuffer(void)
{
    // Compara o inicio do buffer com os comandos conhecidos
    // Utilizado strncmp para maior seguranca de memoria

    if(strncmp(serialCmd.buffer, dCMD_LED_1, dCMD_LENGTH) == 0)
    {
        LedPwm_SetActiveLed(eBSP_LED_1);
    }
    else if(strncmp(serialCmd.buffer, dCMD_LED_2, dCMD_LENGTH) == 0)
    {
        LedPwm_SetActiveLed(eBSP_LED_2);
    }
    else if(strncmp(serialCmd.buffer, dCMD_LED_3, dCMD_LENGTH) == 0)
    {
        LedPwm_SetActiveLed(eBSP_LED_3);
    }

    // Independentemente de ser um comando valido ou nao, limpa o buffer apos uso
    SerialCmd_ClearBuffer();
}

/******************************************************************************/
/** @brief Limpa o buffer de recepcao para receber um novo comando do zero.
 * @retval Nenhum.
 ******************************************************************************/
static void SerialCmd_ClearBuffer(void)
{
    // Limpa o conteudo com preenchimento em 0 (NULL) e zera o indice
    memset(serialCmd.buffer, 0, dMAX_CMD_LENGTH);
    serialCmd.index = dEMPTY_INDEX;
}

/** @} */
