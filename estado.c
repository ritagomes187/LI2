#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "estado.h"

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

/* =============================================================================
 * Função que "liberta" a coordenada
 *  Parâmetros recebidos: Coordenada;
 *  -- Faz "free" da coordenada;
 */

void apaga_coordenada(void *coordenadaPtr) {
    free(coordenadaPtr);
}

/* =============================================================================
 * Função que apaga a jogada anteior
 *  Parâmetros recebidos: Jogada;
 *  -- Apaga a jogada anterior;
 * =============================================================================
 */

void apaga_jogada(void *jogada) {
    JOGADA *j = (JOGADA *) (jogada);
    list_free(j->posicoes);
    free(j);
}

COORDENADA MOV_POSX      = {1, 0};
COORDENADA MOV_POSY      = {0, 1};
COORDENADA MOV_NEGX      = {-1, 0};
COORDENADA MOV_NEGY      = {0, -1};
COORDENADA MOV_POSX_POSY = {1, 1};
COORDENADA MOV_POSX_NEGY = {1, -1};
COORDENADA MOV_NEGX_POSY = {-1, 1};
COORDENADA MOV_NEGX_NEGY = {-1, -1};


/* =============================================================================
 * Função que inicializa o jogo
 *  Parâmetros recebidos: Peça, Modo, Nivel;
 *  -- Aloca um novo jogo;
 *  -- Retorna um ponteiro para a estrura ESTADO se conseguio alocar, se não
 *     retorna NULL;
 * =============================================================================
 */
ESTADO* 
inicializa_jogo(char peca, int modo, int nivel) {

    int i, j;

    ESTADO *e = (ESTADO*) malloc (sizeof(ESTADO));

    if (e == NULL) {
        return e;
    }

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            e->grelha[i][j] = VAZIA;
        }
    }

    e->grelha[3][4] = VALOR_X;
    e->grelha[4][3] = VALOR_X;
    e->grelha[3][3] = VALOR_O;
    e->grelha[4][4] = VALOR_O;
    switch (peca) {
        case 'O': 
            e->peca     = VALOR_O;
            e->bot      = VALOR_X;
            break;

        case 'X': 
            e->peca     = VALOR_X;
            e->bot      = VALOR_O;
            break;
    }
    e->modo = modo;
    e->jogadas_anteriores = list_alloc(NULL, apaga_jogada);
    e->fim_do_jogo = 0;
    e->nivel = nivel;

    return e;
}

/* =============================================================================
 * Função que finaliza o jogo
 *  Parâmetros: Estado;
 * -- Desaloca um jogo;
 * =============================================================================
 */

void finaliza_jogo(ESTADO *e) {
    list_free(e->jogadas_anteriores);
    free(e);
}


/* =============================================================================
 * Função que faz print do menu
 *  -- Imprime o menu do jogo;
 * =============================================================================
 */

void print_menu() {
    printf("\n");
    printf("\t\t\t\t******************************** Menu ********************************\n");
    printf("\t\t\t\t** N <peca>         - Novo Jogo                                     **\n");
    printf("\t\t\t\t** L <ficheiro>     - Ler Jogo de ficheiro                          **\n");
    printf("\t\t\t\t** E <ficheiro>     - Guardar Jogo em ficheiro                      **\n");
    printf("\t\t\t\t** J <L> <C>        - Jogar na posicao (L, C)                       **\n");
    printf("\t\t\t\t** S                - Imprime um '.' nas posicoes com jogada válida **\n");
    printf("\t\t\t\t** H                - Imprime um '?' na posicao sugerida            **\n");
    printf("\t\t\t\t** U                - Desfazer ultima jogada                        **\n");
    printf("\t\t\t\t** A <peça> <nível> - Novo jogo contra computador                   **\n");
    printf("\t\t\t\t** Q                - sair                                          **\n");
    printf("\t\t\t\t**********************************************************************\n");
}

/* =============================================================================
 * Função que printa
 *  Parâmetros recebidos: Estado, Ficheiro;
 * -- Imprime o estado do jogo;
 * =============================================================================
 */

void printa(ESTADO e, FILE *stream) {
    printf("\n");
    printf("\t\t\t\t1 2 3 4 5 6 7 8\n");
    int i, j;
    char c = ' ';

    for (i = 0; i < 8; i++) {
        if (stream == stdout) {
            fprintf(stream, "\t\t\t\t");
        }
        for (j = 0; j < 8; j++) {
            switch (e.grelha[i][j]) {
                case VALOR_O:
                    c = 'O';
                    break;
    
                case VALOR_X:
                    c = 'X';
                    break;
            
                case VALOR_P:
                    c = '.';
                    break;

                case VALOR_H:
                    c = '?';
                    break;

                case VAZIA: 
                    c = '-';
                    break;
            }
            fprintf(stream, "%c ", c);
        }
        fprintf(stream, "\n");
    }
    if (stream == stdout) {
        fprintf(stream, "\t\t\t\tPontuação\n");
        fprintf(stream, "\t\t\t\tJogador x: %d\n", conta_numero_de_pecas(&e, VALOR_X));
        fprintf(stream, "\t\t\t\tJogador O: %d\n", conta_numero_de_pecas(&e, VALOR_O));
    }
}

/* =============================================================================
 * Função que ganha posições
 *  Parâmetros recebidos: Estado, Linha, Coluna, Lista de posições, Uma direção;
 * -- Descobre todas as pecas que um jogador pode ganhar em determinada direção
 *    numa dada jogada;
 * =============================================================================
 */

void 
ganha_posicoes(ESTADO *e, int linha, int coluna, list_t* posicoesPtr, COORDENADA direcao) {

    int numero_pecas = 0;

    VALOR peca_oponente = e->peca % 2 + 1;

    COORDENADA coord;

    coord.x = linha  + direcao.x;
    coord.y = coluna + direcao.y;

    if ((coord.x <= 8 && coord.x >= 1) && (coord.y <= 8 && coord.y >= 1)) {

        if (e->grelha[coord.x - 1][coord.y - 1] == e->peca) {
            return;
        }

        while ((coord.x <= 8 && coord.x >= 1) && (coord.y <= 8 && coord.y >= 1) && e->grelha[coord.x - 1][coord.y - 1] == peca_oponente) {
            COORDENADA* posicao_ganha = (COORDENADA*) malloc(sizeof(COORDENADA));
            posicao_ganha->x = coord.x;
            posicao_ganha->y = coord.y;
            coord.x += direcao.x;
            coord.y += direcao.y;
            list_push_back(posicoesPtr ,(void *) posicao_ganha);
            numero_pecas++;
        }

        if ((coord.x <= 0 || coord.x >= 9 || coord.y <= 0 || coord.y >= 9) || e->grelha[coord.x - 1][coord.y - 1] == VAZIA) {
            while (numero_pecas > 0) {
                list_pop_back(posicoesPtr);
                numero_pecas--;
            }
        }
    }
}

/* =============================================================================
 * Função que descobre a jogada válida
 *  Parâmetros recebidos: Estado, Linha, Coluna, Lista de posições;
 * -- Verifica se uma dada jogada é válida e descobre todas as peças que essa
 *    jogada ganha;
 * -- Se a lista de peças ganhas com uma dada jogada for vazia, então a jogada
 *    não é válida;
 * =============================================================================
 */
int jogada_valida(ESTADO *e, int linha, int coluna, list_t* posicoesPtr) {

    if ((linha <= 8 && linha >= 1) && (coluna <= 8 && coluna >= 1) && e->grelha[linha - 1][coluna - 1] == VAZIA) {
    	ganha_posicoes(e, linha, coluna, posicoesPtr, MOV_POSX);
    	ganha_posicoes(e, linha, coluna, posicoesPtr, MOV_POSY);
    	ganha_posicoes(e, linha, coluna, posicoesPtr, MOV_NEGX);
    	ganha_posicoes(e, linha, coluna, posicoesPtr, MOV_NEGY);
    	ganha_posicoes(e, linha, coluna, posicoesPtr, MOV_POSX_POSY);
   		ganha_posicoes(e, linha, coluna, posicoesPtr, MOV_POSX_NEGY);
    	ganha_posicoes(e, linha, coluna, posicoesPtr, MOV_NEGX_POSY);
    	ganha_posicoes(e, linha, coluna, posicoesPtr, MOV_NEGX_NEGY);
    }
    return !list_isEmpty(posicoesPtr);
}


/* =============================================================================
 * Função que coloca uma peça
 *  Parâmetros recebidos: Estado, Linha, Coluna, Lista de posições;
 * -- Preenche o tabuleiro com todas as peças ganhas numa dada jogada;
 * =============================================================================
 */
void colocar_peca(ESTADO *e, int linha, int coluna, list_t* posicoesPtr) {
    
    COORDENADA  posicao;

    list_iter_t it;
    
    e->grelha[linha - 1][coluna - 1] = e->peca;
    list_iter_reset(&it, posicoesPtr);

    while (list_iter_hasNext(&it, posicoesPtr)) {
        posicao = *((COORDENADA *) (list_iter_next(&it, posicoesPtr)));
        e->grelha[posicao.x - 1][posicao.y - 1] = e->peca;
    }
}



/* =============================================================================
 * Função que coloca a jogada
 *  Parâmetros recebidos: Estado, Linha, Coluna, Lista de posições;
 * -- Adiciona as peças ganhas à lista de jogadas efectudas;
 * =============================================================================
 */
void 
colocar_jogada(ESTADO *e, int linha, int coluna, list_t* posicoesPtr) {

    COORDENADA *posicaoPtr;
    JOGADA     *jogada = (JOGADA *) malloc (sizeof(JOGADA));

    if (jogada != NULL) {
        posicaoPtr = (COORDENADA *) malloc (sizeof(COORDENADA));
        posicaoPtr->x = linha;
        posicaoPtr->y = coluna;
    
        list_push_back(posicoesPtr, (void *) posicaoPtr);
        jogada->peca = e->peca % 2 + 1;
        jogada->posicoes = posicoesPtr; 
        list_push_back(e->jogadas_anteriores, (void *) jogada);
    }
}


/* =============================================================================
 * Função que descobre as jogadas válidas
 *  Parâmetros recebidos: Estado, Lista de posições válidas;
 * -- Descobre todas as jogadas válidas que um jogador pode efectuar;
 * =============================================================================
 */

void descobre_jogadas_validas(ESTADO *e, list_t* posicoes_jogada_validaPtr) {

	int i, j;

	COORDENADA *posicao_validaPtr;
	
	for (i = 1; i <= 8; i++) {
		for (j = 1; j <= 8; j++) {
			list_t* posicoesPtr = list_alloc(NULL, apaga_coordenada);
			if (jogada_valida(e, i, j, posicoesPtr)) {
				posicao_validaPtr = (COORDENADA *) malloc (sizeof(COORDENADA));
				posicao_validaPtr->x = i;
				posicao_validaPtr->y = j;
				list_push_back(posicoes_jogada_validaPtr, (void *) posicao_validaPtr);
                list_free(posicoesPtr);
			}
			else {
				list_free(posicoesPtr);
			}
		}
	}
}


int minmax(ESTADO *e, int nivel, int peca_jogador) {
    
    int value;
    int peca_aux;

    ESTADO estado_aux;

    COORDENADA *posicao;

    list_t* posicoesPtr;
    list_iter_t it;

    list_t *posicoes_jogada_validaPtr = list_alloc(NULL, apaga_coordenada);

    if (nivel == 0 || fim_do_jogo(e)) {
        list_free(posicoes_jogada_validaPtr);
        if(peca_jogador) {
            return conta_numero_de_pecas(e, e->peca) - conta_numero_de_pecas(e, e->peca % 2 + 1);
        }
        else {
            return - (conta_numero_de_pecas(e, e->peca) - conta_numero_de_pecas(e, e->peca % 2 + 1));
        }
    }

    if (peca_jogador) {
        value = INT_MIN;

        descobre_jogadas_validas(e, posicoes_jogada_validaPtr);
        list_iter_reset(&it, posicoes_jogada_validaPtr);

        while (list_iter_hasNext(&it, posicoes_jogada_validaPtr)) {
            posicao = (COORDENADA *)(list_iter_next(&it, posicoes_jogada_validaPtr));
            posicoesPtr = list_alloc(NULL, apaga_coordenada);
            estado_aux = *e;
            peca_aux = estado_aux.peca;
            jogada_valida(&estado_aux, posicao->x, posicao->y, posicoesPtr);
            colocar_peca(&estado_aux, posicao->x, posicao->y, posicoesPtr);
            proximo_jogador(&estado_aux);
            list_free(posicoesPtr);
            if (peca_aux == estado_aux.peca) {
                value = MAX(value, minmax(&estado_aux, nivel - 1, 1));
            }
            else {
                value = MAX(value, minmax(&estado_aux, nivel - 1, 0));
            }
        }
        list_free(posicoes_jogada_validaPtr);
        return value;
    }    

    else {
        value = INT_MAX;
        
        descobre_jogadas_validas(e, posicoes_jogada_validaPtr);

        list_iter_reset(&it, posicoes_jogada_validaPtr);

        while (list_iter_hasNext(&it, posicoes_jogada_validaPtr)) {

            posicao = (COORDENADA *)(list_iter_next(&it, posicoes_jogada_validaPtr));
            posicoesPtr = list_alloc(NULL, apaga_coordenada);
            estado_aux = *e;
            peca_aux = estado_aux.peca;
            jogada_valida(&estado_aux, posicao->x, posicao->y, posicoesPtr);
            colocar_peca(&estado_aux, posicao->x, posicao->y, posicoesPtr);
            proximo_jogador(&estado_aux);
            list_free(posicoesPtr);
             if (peca_aux == estado_aux.peca) {
                value = MIN(value, minmax(&estado_aux, nivel - 1, 0));
            }
            else {
                value = MIN(value, minmax(&estado_aux, nivel - 1, 1));
            }
        }
        list_free(posicoes_jogada_validaPtr);
        return value;
    }
}

/* =============================================================================
 * Função que descobre a melhor jogada
 *  Parâmetros recebidos: Estado, Nível;
 * -- Descobre a melhor jogada válida que um jogador pode efetuar;
 * =============================================================================
 */
COORDENADA* descobre_melhor_jogada(ESTADO *e, int nivel) {
    
    list_t *posicoes_jogada_validaPtr = list_alloc(NULL, apaga_coordenada);
    list_iter_t it;
    list_t* posicoesPtr;
    ESTADO estado_aux;

    int value     = INT_MIN;
    int value_aux = INT_MIN;
    int peca_aux;

    COORDENADA* posicao;
    COORDENADA* melhor_jogada = (COORDENADA*) malloc (sizeof(COORDENADA));
    melhor_jogada->x = -1;
    melhor_jogada->y = -1;

    descobre_jogadas_validas(e, posicoes_jogada_validaPtr);

    list_iter_reset(&it, posicoes_jogada_validaPtr);
    
    while (list_iter_hasNext(&it, posicoes_jogada_validaPtr)) {
            posicao = (COORDENADA *)(list_iter_next(&it, posicoes_jogada_validaPtr));
            posicoesPtr = list_alloc(NULL, apaga_coordenada);
            estado_aux = *e;
            peca_aux = estado_aux.peca;
            jogada_valida(&estado_aux, posicao->x, posicao->y, posicoesPtr);
            colocar_peca(&estado_aux, posicao->x, posicao->y, posicoesPtr);
            proximo_jogador(&estado_aux);
            list_free(posicoesPtr);
           	if (peca_aux == estado_aux.peca) {
           		value_aux = minmax(&estado_aux, nivel - 1, 1);
           	}
            else {
                value_aux = minmax(&estado_aux, nivel - 1, 0);
            }
            if (value_aux >= value) {
                melhor_jogada->x = posicao->x;
                melhor_jogada->y = posicao->y;
                value = value_aux;
            }
    }
    list_free(posicoes_jogada_validaPtr);

    return melhor_jogada;
}


/* =============================================================================
 * Função que para para o próximo jogador
 *  Parâmetros recebidos: Estado;
 * -- Calcula o próximo jogador a jogar;
 * -- Calcula também se o jogo acabou;
 * =============================================================================
 */

void proximo_jogador(ESTADO *e) {

	list_t *posicoes_jogada_validaPtr = list_alloc(NULL, apaga_coordenada); 

	e->peca = (e->peca % 2) + 1;

	descobre_jogadas_validas(e, posicoes_jogada_validaPtr);
	if (list_isEmpty(posicoes_jogada_validaPtr)) {
		
		e->peca = (e->peca % 2) + 1;

		descobre_jogadas_validas(e, posicoes_jogada_validaPtr);
        if (list_isEmpty(posicoes_jogada_validaPtr)) {
            list_free(posicoes_jogada_validaPtr);
            e->fim_do_jogo = 1;
        }
        else {
            list_free(posicoes_jogada_validaPtr);
            e->fim_do_jogo = 0;
        }
	}
    else {
        list_free(posicoes_jogada_validaPtr);
        e->fim_do_jogo = 0;
    }
}


/* =============================================================================
 * Função que verifica se o jogo terminou
 *  Parâmetros recebidos: Estado;
 * -- Verifica se o jogo chegou ao fim;
 * -- Devolve 1 caso o jogo tenha chegado ao fim e 0 em caso contrário;
 * =============================================================================
 */
int fim_do_jogo(ESTADO *e) {
    return e->fim_do_jogo;
}


/* =============================================================================
 * Função que faz o print do fim do jogo
 *  Parâmetros recebidos: Estado;
 * -- Imprime a mensagem de fim de jogo;
 * =============================================================================
 */

void
print_fim_do_jogo(ESTADO *e) {
    int pecas_jogador_x = conta_numero_de_pecas(e, VALOR_X);
    int pecas_jogador_o = conta_numero_de_pecas(e, VALOR_O);

    if (pecas_jogador_x > pecas_jogador_o) {
        printf("\n\t\t\t\tO jogo terminou com a vitória do jogador X com %d pontos.\n", pecas_jogador_x);
    }
    else if (pecas_jogador_x < pecas_jogador_o) {
        printf("\n\t\t\t\tO jogo terminou com a vitória do jogador O com %d pontos.\n", pecas_jogador_o);
    }
    else {
        printf("\n\t\t\t\tO jogo terminou com um empate\n");
    }
}


/* =============================================================================
 * Função que conta o número de peças
 *  Parâmetros recebidos: Estado, Peça;
 * -- Devolve o número de peças de uma dada peça;
 * =============================================================================
 */

int 
conta_numero_de_pecas(ESTADO *e, VALOR peca) {
	
    int i, j;
	int numero_pecas = 0;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if (e->grelha[i][j] == peca) {
				numero_pecas++;
			}
		}
	}

	return numero_pecas;
}