#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "estado.h"
#include "funcoes.h"
#include "list.h"


/* =============================================================================
 * Função que pergunta se queremos abandonar o jogo atual;
 * Parâmetros recebidos: Estado.
 * =============================================================================
*/
int abandonar_jogo_atual(ESTADO *e) {

	char linha[TAMANHO_LINHA + 1];

	if (e) {
		do {
			linha[0] = '\0';
			printf("\n\t\t\t\tTem a certeza que quer abandonar o jogo atual?\n");
			printf("\t\t\t\tS ou n : ");
			fgets(linha, TAMANHO_LINHA + 1, stdin);
		} while(strlen(linha) != 2 || (linha[0] != 'S' && linha[0] != 's' && linha[0] != 'N' && linha[0] != 'n'));

		if (linha[0] == 'S' || linha[0] == 's') {
			finaliza_jogo(e);
			return 1;
		}
		else {
			return 0;
		}
	}
	return 1;
}

/* =============================================================================
 * Função que verifica se o jogo terminou;
 * --se não tiver terminado descobre a melhor jogada.
 * Parâmetros recebidos: Estado.
 * =============================================================================
*/

void jogada_do_bot(ESTADO *e) {

	COORDENADA *jogada;
	list_t     *posicoesPtr;

	if (e) {
		if (!fim_do_jogo(e)) {
			jogada = descobre_melhor_jogada(e, e->nivel);

			if (jogada->x != -1 && jogada->y != -1) {
				posicoesPtr = list_alloc(NULL, apaga_coordenada);
				if(jogada_valida(e, jogada->x, jogada->y, posicoesPtr)) {
					colocar_peca(e, jogada->x, jogada->y, posicoesPtr);
					colocar_jogada(e, jogada->x, jogada->y, posicoesPtr);
				}
				printf("\n");
				printa(*e, stdout);
				printf("\n");
			}
			apaga_coordenada(jogada);
			proximo_jogador(e);
		}
		else {
			print_fim_do_jogo(e);
		}
	}
}

/* =============================================================================
* Função que verifica se quer abandonar o jogo;
* Se não quiser faz return, se quiser cria um novo jogo;
* Parâmetros recebidos: Estado, Peça.
* =============================================================================
*/
void executa_N(ESTADO **e, char peca) {


	if (!abandonar_jogo_atual(*e)) {
		return;
	}

	*e = inicializa_jogo(peca, 0, -1);

	if (*e == NULL) {
		printf("\t\t\t\tNão foi possivel carregar o jogo. Tente novamente...\n");
	}
	else {
		printf("\n");
		printa(**e, stdout);
		printf("\n");
	}
}
/* =============================================================================
* Função que verifica se o fim do jogo ainda não foi atingido.
*--Se não tiver sido atingido aloca uma lista de posições(verifica se a jogada é válida e vê as posições que podemos "comer"; coloca a peça).
* Parâmetros recebidos: Estado, linha, coluna.
* =============================================================================
*/

void executa_J(ESTADO *e, int linha, int coluna) {

	list_t* posicoesPtr;

	if (e) {
		if (!fim_do_jogo(e)) {
			posicoesPtr = list_alloc(NULL, apaga_coordenada);
			if (jogada_valida(e, linha, coluna, posicoesPtr)) {
				colocar_peca(e, linha, coluna, posicoesPtr);
				colocar_jogada(e, linha, coluna, posicoesPtr);
				proximo_jogador(e);
				if (e->modo == 1) {
					while (e->peca == e->bot) {
						jogada_do_bot(e);
					}
				}
			}
			else {
				list_free(posicoesPtr);
			}

			printf("\n");
			printa(*e, stdout);
			printf("\n");
		}
		else {
			print_fim_do_jogo(e);
		}
	}
}

/* =============================================================================
* Função que lê o ficheiro.
* Parâmetros recebidos: Estado, ficheiro.
* =============================================================================
*/
void executa_L(ESTADO **e, char nome_ficheiro[]) {

	FILE *fPtr;
	char modo;
	char peca_proximo_jogador;
	char linha[TAMANHO_LINHA + 1];
	char tokens[8];
	int numTokens = 0;
	int l = 0;
	int c = 0;

	if ((fPtr = fopen(nome_ficheiro, "r")) != NULL) {

		if (!abandonar_jogo_atual(*e)) {
			return;
		}

		while (fgets(linha, TAMANHO_LINHA + 1, fPtr)) {
			numTokens = sscanf(linha, "%c %c %c %c %c %c %c %c", &tokens[0], &tokens[1], &tokens[2], &tokens[3], &tokens[4], &tokens[5], &tokens[6], &tokens[7]);
			switch (numTokens) {
				case 2:
				case 3:
					modo = tokens[0];
					peca_proximo_jogador = tokens[1];
					*e = ((numTokens == 2) ? inicializa_jogo(peca_proximo_jogador, modo, -1) : inicializa_jogo(peca_proximo_jogador, modo, tokens[2]));
					if (*e == NULL) {
						printf("\t\t\t\tNão foi possivel carregar o jogo. Tente novamente...\n");
						return;
					}
					break;

				case 8:
					for (c = 0; c < 8; c++) {

						switch (tokens[c]) {
							case 'O':
								(*e)->grelha[l][c] = VALOR_O;
								break;
							case 'X':
								(*e)->grelha[l][c] = VALOR_X;
								break;
							case '.':
								(*e)->grelha[l][c] = VALOR_P;
								break;
							case '-':
								(*e)->grelha[l][c] = VAZIA;
								break;
						}
					}
					l++;
					break;
			}
		}

		fclose(fPtr);
	}
	printf("\n");
	printa(**e, stdout);
	printf("\n");
}

/* =============================================================================
* Função que escreve o ficheiro.
* Parâmetros recebidos: Estado, ficheiro.
* =============================================================================
*/
void executa_E(ESTADO *e, char nome_ficheiro[]) {

	FILE *fPtr;
	char modo;
	char peca_proximo_jogador;

	if (e) {
		modo = (e->modo ? 'A' : 'M');
		peca_proximo_jogador = (e->peca == VALOR_X ? 'X' : 'O');

		if ((fPtr = fopen(nome_ficheiro, "w")) != NULL) {
			if (e->modo) {
				fprintf(fPtr, "%c %c %d\n", modo, peca_proximo_jogador, e->modo);
			}
			else {
				fprintf(fPtr, "%c %c\n", modo, peca_proximo_jogador);
			}
			printa(*e, fPtr);

			fclose(fPtr);
		}

		printa(*e, stdout);
		printf("\n");
	}
}

/* =============================================================================
* Função que imprime as posições onde podemos jogar.
* --(estamos a usar uma grelha ailiar para não alterar a grelha atual e,depois é só fazer print da auxiliar)
* Parâmetros recebidos: Estado.
* =============================================================================
*/
void executa_S(ESTADO *e) {

	ESTADO e_aux;

	COORDENADA *posicao_validaPtr;

	list_t* posicoes_jogada_validaPtr;
	list_iter_t it;

	if (e) {

		posicoes_jogada_validaPtr = list_alloc(NULL, apaga_coordenada);
		e_aux = *e;

		descobre_jogadas_validas(&e_aux, posicoes_jogada_validaPtr);

		list_iter_reset(&it, posicoes_jogada_validaPtr);

		while (list_iter_hasNext(&it, posicoes_jogada_validaPtr)) {
			posicao_validaPtr = (COORDENADA *) list_iter_next(&it, posicoes_jogada_validaPtr);
			e_aux.grelha[posicao_validaPtr->x - 1][posicao_validaPtr->y - 1] = VALOR_P;
		}

		printa(e_aux, stdout);
		printf("\n");

		list_free(posicoes_jogada_validaPtr);
	}
}

/* =============================================================================
* Função que imprime a melhor jogada.
* Parâmetros recebidos: Estado.
* =============================================================================
*/
void executa_H(ESTADO *e) {
    int i,j;
	COORDENADA* melhor_jogada;

	if (e) {
		melhor_jogada = descobre_melhor_jogada(e, 2);
		if (melhor_jogada->x != -1 && melhor_jogada->y != -1) {
			e->grelha[melhor_jogada->x - 1][melhor_jogada->y - 1] = VALOR_H;
			printf("\n");
			printa(*e, stdout);
			printf("\n");

			e->grelha[melhor_jogada->x - 1][melhor_jogada->y - 1] = VAZIA;
		}
            e->grelha[i][j] = VAZIA;
        }
		apaga_coordenada(melhor_jogada);
	}

/* =============================================================================
* Função que volta uma jogada atrás, ou seja, vai buscar a última jogada à lista jogadas(list_iter_next).
* Parâmetros recebidos: Estado.
* =============================================================================
*/
void voltar_atras(ESTADO *e) {

	list_iter_t it;
	COORDENADA *posicaoPtr;
	JOGADA *ultima_jogada;

	if (e) {
		if (e->jogadas_anteriores) {
			ultima_jogada = (JOGADA *) list_back(e->jogadas_anteriores);

			if (ultima_jogada) {

				posicaoPtr = (COORDENADA *) list_back(ultima_jogada->posicoes);

				if (posicaoPtr) {
					e->grelha[posicaoPtr->x - 1][posicaoPtr->y - 1] = VAZIA;

					list_pop_back(ultima_jogada->posicoes);
				}

				list_iter_reset(&it, ultima_jogada->posicoes);

				while (list_iter_hasNext(&it, ultima_jogada->posicoes)) {
					posicaoPtr = (COORDENADA *) list_iter_next(&it, ultima_jogada->posicoes);

					if (posicaoPtr) {
						e->grelha[posicaoPtr->x - 1][posicaoPtr->y - 1] = ultima_jogada->peca;
					}
				}
				list_pop_back(e->jogadas_anteriores);
			}
		}
	}
}

/* =============================================================================
* Função que volta atrás, quer seja com o bot ou com a funcionalidade jogador vs. jogador)
* --Se for jogador vs. jogador: apaga só uma jogada;
* --Se for bot vs. jogador: apaga todas as jogadas até ser o jogador a jogar.
* Parâmetros recebidos: Estado.
* =============================================================================
*/
void executa_U(ESTADO *e) {
	if (e) {
		if (e->modo == 0) {
			voltar_atras(e);
			proximo_jogador(e);
		}
		else {
			do {
				voltar_atras(e);
				proximo_jogador(e);
			} while(e->peca != (e->bot % 2 + 1));
		}
		printf("\n");
		printa(*e, stdout);
		printf("\n");
	}
}

/* =============================================================================
* Função que cria um novo jogo contra o bot.
* Parâmetros recebidos: Estado, Peça, Nível.
* =============================================================================
*/
void executa_A(ESTADO **e, char peca, int nivel) {

	if (!abandonar_jogo_atual(*e)) {
		return;
	}

	*e = inicializa_jogo(peca, 1, nivel);
	if (*e == NULL) {
		printf("\t\t\t\tNão foi possivel carregar o jogo. Tente novamente...\n");
	}
	else {
		printf("\n");
		printa(**e, stdout);
		printf("\n");
	}
}

/* =============================================================================
* Função que finaliza o jogo.
* Parâmetros recebidos: Estado.
* =============================================================================
*/

void executa_Q(ESTADO *e) {
	if (e) {
		finaliza_jogo(e);
	}
}