#ifndef PROJ_ESTADO_H
#define PROJ_ESTADO_H

#include "list.h"

typedef struct coordenada {
	int x;
	int y;
} COORDENADA;

void apaga_coordenada(void *coordenadaPtr);
void apaga_jogada(void *jogada);

typedef enum {
	VAZIA,
	VALOR_X,
	VALOR_O,
	VALOR_P,
	VALOR_H
} VALOR;

typedef struct jogada {
	VALOR   peca;               /* peca que foi comida */
	list_t *posicoes;           /* lista de posicoes que foram comidas */
}JOGADA;

typedef struct estado {
	VALOR peca;                 /* peça do jogador que vai jogar! */
	VALOR bot;                  /* peca do bot */
	VALOR grelha[8][8];         /* grelha */
	int fim_do_jogo;            /* flag que indica se o jogo terminou */
	int nivel;                  /* nivel do jogo contra o bot */
	char modo;                  /* modo em que se está a jogar! 0-> manual, 1-> contra computador */
	list_t *jogadas_anteriores; /* todas as jogadas já efectuadas */
} ESTADO;

ESTADO*
inicializa_jogo(char peca, int modo, int nivel);

void finaliza_jogo(ESTADO *e);

void print_menu();

void printa(ESTADO e, FILE *stream);

void ganha_posicoes(ESTADO *e, int linha, int coluna, list_t* posicoesPtr, COORDENADA direcao);


int jogada_valida(ESTADO *e, int linha, int coluna, list_t* posicoesPtr);

void colocar_peca(ESTADO *e, int linha, int coluna, list_t* posicoesPtr);

void colocar_jogada(ESTADO *e, int linha, int coluna, list_t* posicoesPtr);

void descobre_jogadas_validas(ESTADO *e, list_t* posicoes_jogada_validaPtr);

COORDENADA*
descobre_melhor_jogada(ESTADO *e, int nivel);

int fim_do_jogo(ESTADO *e);

void print_fim_do_jogo(ESTADO *e);

void proximo_jogador(ESTADO *e);

int conta_numero_de_pecas(ESTADO *e, VALOR peca);

#endif