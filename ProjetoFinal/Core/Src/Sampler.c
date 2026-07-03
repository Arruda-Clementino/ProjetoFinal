/******************************************************************************/
/**
 * @file Sampler.c
 * @addtogroup Sampler
 * @author Luis Eduardo
 * @details
 * @version 1
 / @} DOXYGEN GROUP TAG END OF FILE */

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "Sampler.h"

/*******************************************************************************
 * DEFINES LOCAIS
 ******************************************************************************/
/// Quantidade de amostras para o calculo da media (Janela de 500 ms a 5 ms/amostra)
#define dNUMBER_OF_SAMPLES    100

/// Resolucao maxima do ADC do microcontrolador (12 bits)
#define dADC_MAX_VALUE        4095

/// Valor maximo para a escala percentual
#define dMAX_PERCENTAGE       100

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/
/// Variaveis internas de controle do calculo de media
static struct
{
    /// Somatorio das amostras brutas coletadas
    u32 accumulator;

    /// Contador de amostras validas no ciclo atual
    u8 sampleCount;

    /// Ultima media convertida para a escala de 0 a 100%
    u8 filteredPercentage;

    /// Sinaliza que um novo valor de media foi fechado
    bool isNewAverageReady;
} sampler;

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/** @brief Inicializa as variaveis do modulo de amostragem.
 * @retval Nenhum.
 ******************************************************************************/
void Sampler_Init(void)
{
    sampler.accumulator = 0;
    sampler.sampleCount = 0;
    sampler.filteredPercentage = 0;
    sampler.isNewAverageReady = false;
}

/******************************************************************************/
/** @brief Acumula uma nova leitura do ADC e calcula a media ao atingir o limite.
 * @param rawAdcValue: Valor cru lido direto do registrador do conversor AD.
 * @retval Nenhum.
 ******************************************************************************/
void Sampler_ProcessNewSample(u16 rawAdcValue)
{
    // Soma o novo valor ao acumulador
    sampler.accumulator += rawAdcValue;
    sampler.sampleCount++;

    // Verifica se a janela de amostras (100 amostras) encheu
    if(sampler.sampleCount >= dNUMBER_OF_SAMPLES)
    {
        // 1. Calcula a media aritmetica crua
        u32 rawAverage = sampler.accumulator / dNUMBER_OF_SAMPLES;

        // 2. Converte o resultado bruto para a escala percentual (0 a 100%)
        // Multiplica-se por 100 ANTES da divisao para evitar truncamento em numeros inteiros
        sampler.filteredPercentage = (u8)((rawAverage * dMAX_PERCENTAGE) / dADC_MAX_VALUE);

        // 3. Levanta a flag de que temos uma nova media pronta
        sampler.isNewAverageReady = true;

        // 4. Zera os acumuladores para a proxima janela de amostragem
        sampler.accumulator = 0;
        sampler.sampleCount = 0;
    }
}

/******************************************************************************/
/** @brief Retorna o valor atualizado do potenciometro em porcentagem.
 * @retval Valor de 0 a 100 representando a media da intensidade.
 ******************************************************************************/
u8 Sampler_GetFilteredPercentage(void)
{
    return sampler.filteredPercentage;
}

/******************************************************************************/
/** @brief Verifica se uma nova media foi calculada com sucesso.
 * @retval true se ha uma nova media, false caso contrario.
 ******************************************************************************/
bool Sampler_IsNewAverageReady(void)
{
    return sampler.isNewAverageReady;
}

/******************************************************************************/
/** @brief Limpa a flag de nova media calculada.
 * @retval Nenhum.
 ******************************************************************************/
void Sampler_ClearAverageFlag(void)
{
    sampler.isNewAverageReady = false;
}

/*******************************************************************************
 * FUNCOES LOCAIS
 ******************************************************************************/

/** @} */
