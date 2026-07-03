/******************************************************************************/
/**
 * @file Button.c
 * @addtogroup Button
 * @author Luis Eduardo
 * @details
 * @version 2
 / @} DOXYGEN GROUP TAG END OF FILE */

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "Button.h"
#include "Bsp.h"

/*******************************************************************************
 * DEFINES LOCAIS
 ******************************************************************************/

/*******************************************************************************
 * CONSTANTES
 ******************************************************************************/

/*******************************************************************************
 * ESTRUTURAS DE DADOS LOCAIS
 ******************************************************************************/
/// Variaveis internas de estado do botao
static struct
{
    /// Armazena o estado atual do sistema (true = congelado, false = rodando)
    bool isSystemFrozen;
} button;

/*******************************************************************************
 * PROTOTIPOS LOCAIS
 ******************************************************************************/

/*******************************************************************************
 * FUNCOES PUBLICAS
 ******************************************************************************/

/******************************************************************************/
/** @brief Inicializa o estado logico do modulo do botao.
 * @retval Nenhum.
 ******************************************************************************/
void Button_Init(void)
{
    // O sistema sempre inicia em operacao normal (nao congelado)
    button.isSystemFrozen = false;
}

/******************************************************************************/
/** @brief Processa a flag de clique originada pela BSP e inverte o estado.
 * @retval Nenhum.
 * @details Esta funcao nao eh bloqueante e deve ser chamada no laco principal.
 ******************************************************************************/
void Button_Process(void)
{
    // Verifica se a Bsp registrou um clique valido (passando pelo debounce)
    if(Bsp_GetButtonFlag() == true)
    {
        // Limpa a flag na BSP para nao processar o mesmo clique duas vezes
        Bsp_ClearButtonFlag();

        // Alterna o estado de congelamento (se for true vira false, e vice-versa)
        button.isSystemFrozen = !button.isSystemFrozen;
    }
}

/******************************************************************************/
/** @brief Retorna o estado atual de congelamento do sistema.
 * @retval true se a atualizacao dos LEDs deve parar, false caso contrario.
 ******************************************************************************/
bool Button_IsSystemFrozen(void)
{
    return button.isSystemFrozen;
}

/*******************************************************************************
 * FUNCOES LOCAIS
 ******************************************************************************/

/** @} */
