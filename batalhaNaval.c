#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/*
* Batalha Naval - Habilidades especiais e áreas de efeito (nível avançado)
*
* Regras desta etapa:
* - Tabuleiro fixo 10X10 com água (0), navios (3) e áreas de habilidade (5).
* - Quatro navios de tamanho 3: dois ortogonais + dois diagonais (do nível anterior).
* - Três habilidaeds com máscaras 5x5 (0/1) geradas dinamicamente:
*   - Cone: ápice no topo da máscara, expandindo para baixo.
*   - Cruz: linhas que cruzam no centro.
*   - Octaedro (losango): distância Manhattan <= raio
* - Cada habilidade é aplicada sobre o tabuleiro em um ponto de origem (linha, coluna),
*   respeitando limites e sem sobrescrever navios.
*/

#define BOARD_SIZE 10
#define WATER 0
#define SHIP_VALUE 3
#define SKILL_VALUE 5

#define SHIP_SIZE 3
#define MASK_N 5

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
 * Gera máscara CONE (MASK_N x MASK_N) com ápice no TOPO (linha 0, coluna centro).
 * Lógica: para cada linha r, ativa colunas dentro do intervalo [centro - r, centro + r].
 * Exemplo com N=5 (1 = afetado):
 *   0 0 1 0 0
 *   0 1 1 1 0
 *   1 1 1 1 1
 *   1 1 1 1 1  (para N>3 continua alargando até o limite da máscara)
 *   1 1 1 1 1
 */
void build_cone_mask(int mask[MASK_N][MASK_N]) {
    int center = MASK_N / 2;
    for (int r = 0; r < MASK_N; r++) {
        for (int c = 0; c < MASK_N; c++) {
            // Ativa se |c - center| <= r (abre o cone conforme desce)
            if (abs(c - center) <= r) {
                mask[r][c] = 1;
            } else {
                mask[r][c] = 0;
            }
        }
    }
}

/*
 * Gera máscara CRUZ (MASK_N x MASK_N) com origem no CENTRO.
 * Lógica: ativa toda a linha central e toda a coluna central.
 */
void build_cross_mask(int mask[MASK_N][MASK_N]) {
    int center = MASK_N / 2;
    for (int r = 0; r < MASK_N; r++) {
        for (int c = 0; c < MASK_N; c++) {
            mask[r][c] = (r == center || c == center) ? 1 : 0;
        }
    }
}

/*
 * Gera máscara OCTAEDRO (losango) com origem no CENTRO.
 * Lógica: ativa posições cuja distância Manhattan ao centro <= raio (center).
 * d = |r - center| + |c - center| <= center
 */
void build_octa_mask(int mask[MASK_N][MASK_N]) {
    int center = MASK_N / 2;
    for (int r = 0; r < MASK_N; r++) {
        for (int c = 0; c < MASK_N; c++) {
            int d = abs(r - center) + abs(c - center);
            mask[r][c] = (d <= center) ? 1 : 0;
        }
    }
}

/*
 * Aplica uma máscara de habilidade ao tabuleiro.
 *
 * Parâmetros:
 *  - origin_row, origin_col: ponto de origem no TABULEIRO (coordenadas destino).
 *  - anchor_row, anchor_col: onde está o "ponto de origem" dentro da MÁSCARA.
 *      * Para CRUZ e OCTAEDRO: usar o centro (MASK_N/2, MASK_N/2).
 *      * Para CONE: usar o ÁPICE no topo (0, MASK_N/2).
 *  - Só marca com SKILL_VALUE se a célula no tabuleiro estiver com WATER (não sobrescreve navios).
 *  - Ignora partes da máscara que caírem fora do tabuleiro (bordas).
 */
void apply_skill_mask(int board[BOARD_SIZE][BOARD_SIZE],
                      int origin_row, int origin_col,
                      int mask[MASK_N][MASK_N],
                      int anchor_row, int anchor_col)
{
    for (int mr = 0; mr < MASK_N; mr++) {
        for (int mc = 0; mc < MASK_N; mc++) {
            if (mask[mr][mc] != 1) continue; // só aplica onde a máscara ativa

            int br = origin_row + (mr - anchor_row);
            int bc = origin_col + (mc - anchor_col);

            if (br < 0 || br >= BOARD_SIZE || bc < 0 || bc >= BOARD_SIZE) {
                continue; // fora do tabuleiro
            }

            // Não sobrescrever navio
            if (board[br][bc] == WATER) {
                board[br][bc] = SKILL_VALUE;
            }
        }
    }
}

/* Imprime o tabuleiro com legenda:
 * 0 = água, 3 = navio, 5 = área de habilidade
 */
void print_board(const int board[BOARD_SIZE][BOARD_SIZE]) {
    printf("Tabuleiro 10x10  (0=agua, 3=navio, 5=habilidade)\n\n   ");
    for (int c = 0; c < BOARD_SIZE; c++) printf("%2d ", c);
    printf("\n");
    for (int r = 0; r < BOARD_SIZE; r++) {
        printf("%2d ", r);
        for (int c = 0; c < BOARD_SIZE; c++) {
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

    // Cria máscaras de habilidades (0/1)
    int cone[MASK_N][MASK_N];
    int cross[MASK_N][MASK_N];
    int octa[MASK_N][MASK_N];

    build_cone_mask(cone);
    build_cross_mask(cross);
    build_octa_mask(octa);

    // Aplica habilidades no tabuleiro
    // Âncoras (ponto de origem dentro da máscara)
    const int center = MASK_N / 2;
    const int cone_anchor_row = 0;         // ápice no topo
    const int cone_anchor_col = center;
    const int center_anchor_row = center;  // centro para cruz e octaedro
    const int center_anchor_col = center;

    // Pontos de origem no TABULEIRO (escolha livre; mantidos no código)
    // Cuidados: cone cresce para baixo; escolha uma linha que caiba (<= 5 com N=5).
    int origin_cone_row  = 1, origin_cone_col  = 4; // alinha ápice do cone em (1,4)
    int origin_cross_row = 6, origin_cross_col = 6; // centro da cruz
    int origin_octa_row  = 3, origin_octa_col  = 3; // centro do losango

    apply_skill_mask(board, origin_cone_row,  origin_cone_col,  cone,
                     cone_anchor_row, cone_anchor_col);

    apply_skill_mask(board, origin_cross_row, origin_cross_col, cross,
                     center_anchor_row, center_anchor_col);

    apply_skill_mask(board, origin_octa_row,  origin_octa_col,  octa,
                     center_anchor_row, center_anchor_col);

    /* Exibe o tabuleiro final */
    print_board(board);
    return 0;
}
