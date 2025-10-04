#include <stdio.h>
#include <stdbool.h>

/*
* Batalha Naval - Tabuleiro Completo e Navios Diagonais (nível intermediário)
*
* Regras desta etapa:
* - Tabuleiro fixo 10X10 preenchido com 0 (água).
* - Quatro navios de tamanho fixo 3, valor 3 nas cécluas ocupadas.
* - Dois vanios ortogonais (horizontal/vertical) e dois navios diagonais:
*   - Diaogonal "descendo para a direita" (dr=+1, dc=+1)
*   - Diagonal "subindo para a direita" (dr=-1, dc=+1)
* - Coordenadas definidas no código.
* - Validação de limites e de sobreposição (inclui diagonais).
* Impressão organizada do tabuleiro
*/

#define BOARD_SIZE 10
#define WATER 0
#define SHIP_VALUE 3
#define SHIP_SIZE 3

typedef enum {
    HORIZONTAL      = 0,    // dr=0,  dc=+1
    VERTICAL        = 1,    // dr=+1, dc=0
    DIAG_DOWN_RIGHT = 2,    // dr=+1, dc=+1   (tipo tabuleiro[i][i])
    DIAG_UP_RIGHT   = 3     // dr=-1, dc=+1   (tipo tabuleiro[i][9-i])
} Orientation;

/*
* Converte a orientação em deslocamentos (delta) por passo
*/
static inline void delta_from_orientation(Orientation o, int *dr, int *dc) {
    switch (o) {
        case HORIZONTAL:
            *dr = 0;
            *dc = 1;
            break;
        
        case VERTICAL:
            *dr = 1;
            *dc = 0;
            break;

        case DIAG_DOWN_RIGHT:
            *dr = 1;
            *dc = 1;
            break;

        case DIAG_UP_RIGHT:
            *dr = -1;
            *dc = 1;
            break;

        default:
            *dr = 0;
            *dc = 0;
            break;
    }
}

/*
* Inicializa todo o tabuleiro com água
*/
void init_board(int board[BOARD_SIZE][BOARD_SIZE])
{
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            board[r][c] = WATER;
        }
    }
}

/*
* Verifica:
* - Se todas as posições do navio (a partir de start_row/start_col) ficam dentro do tabuleiro
* - Se não há sobreposições com células já ocupadas (diferentes de WATER).
*/ 
bool can_place_ship(
    const int board[BOARD_SIZE][BOARD_SIZE],
    int start_row,
    int start_col,
    int ship_len,
    Orientation orient
)
{
    int dr, dc;
    delta_from_orientation(orient, &dr, &dc);

    for(int i = 0; i < ship_len; i++) {
        int r = start_row + dr * i;
        int c = start_col + dc * i;

        // Limites
        if (r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE) {
            return false;
        }

        // Sobreposição
        if (board[r][c] != WATER) {
            return false;
        }
    }

    return true;
}

/*
* Posiciona o navio copiando os valores do vetor 'ship[]' (todos valem 3)
* para o tabuleiro, seguindo a orientação. Retorna false se nõ couber.
*/
bool place_ship_from_array(
    int board[BOARD_SIZE][BOARD_SIZE],
    int start_row,
    int start_col,
    const int ship[],
    int ship_len,
    Orientation orient
)
{
    if (!can_place_ship(board, start_row, start_col, ship_len, orient)) {
        return false;
    }

    int dr, dc;
    delta_from_orientation(orient, &dr, &dc);

    for (int i = 0; i < ship_len; i++) {
        int r = start_row + dr * i;
        int c = start_col + dc * i;
        board[r][c] = ship[i];
    }

    return true;
}

/*
* Imprime o tabuleiro de forma organizada (0 = água, 3 = navio).
*/
void print_board(const int board[BOARD_SIZE][BOARD_SIZE]) {
    printf("Tabuleiro (0 = água, 3 = navio)\n\n  ");
    
    // Cabeçalho de colunas
    for (int c = 0; c < BOARD_SIZE; c++) {
        printf("%2d ", c);
    }

    printf("\n");

    for (int r = 0; r < BOARD_SIZE; r++) {
        printf("%2d", r); // Índice da linha
        for(int c = 0; c < BOARD_SIZE; c++) {
            printf("%2d ", board[r][c]);
        }

        printf("\n");
    }
}


int main() {
    // Tabuleiro 10X10
    int board[BOARD_SIZE][BOARD_SIZE];
    init_board(board);

    /* Todos os navios têm tamanho 3, e cada posição vale 3 */
    const int ship[SHIP_SIZE] = { SHIP_VALUE, SHIP_VALUE, SHIP_VALUE };

    /* ============ Coordenadas de exemplo (sem sobreposição) ============ 
     * - Diagonal para baixo/direita começando em (0,0): (0,0) (1,1) (2,2)
     * - Horizontal na linha 2 a partir da coluna 5:     (2,5) (2,6) (2,7)
     * - Vertical na coluna 3 a partir da linha 5:       (5,3) (6,3) (7,3)
     * - Diagonal para cima/direita começando em (9,6):  (9,6) (8,7) (7,8)
     */

    struct {
        int row, col;
        Orientation orient;
        const char *name;
    } placements[4] = {
        { 0, 0, DIAG_DOWN_RIGHT, "Diag Down-Right" },
        { 2, 5, HORIZONTAL,      "Horizontal"      },
        { 5, 3, VERTICAL,        "Vertical"        },
        { 9, 6, DIAG_UP_RIGHT,   "Diag Up-Right"   }
    };

    /* Posiciona os quatro navios com validação */
    for (int k = 0; k < 4; k++) {
        if (!place_ship_from_array(board, placements[k].row, placements[k].col,
                                   ship, SHIP_SIZE, placements[k].orient)) {
            printf("ERRO: Nao foi possivel posicionar o navio %s em (%d,%d).\n",
                   placements[k].name, placements[k].row, placements[k].col);
            return 1;
        }
    }

    /* Exibe o tabuleiro final */
    print_board(board);
    return 0;
}
