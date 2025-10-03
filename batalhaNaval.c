#include <stdio.h>
#include <stdbool.h>

/*
* Batalha Naval (nível novato)
* - Tabuleiro 10X10 preenchido com 0 *água)
* - Dois navios (tamanho fixo - 3), representados por vetores 1D
* - Um navio horizontal, outro vertical
* - Valor 3 representa a parte do navio no tabuleiro
* - Coordenadas dos navios definidas diretamente no código
* - Validação: limites do tabuleiro e ausência de sobreposiçào
*/

#define BOARD_SIZE 10
#define WATER 0
#define SHIP 3
#define SHIP_SIZE 3

typedef enum {
    HORIZONTAL = 0,
    VERTICAL = 1
} Orientation;

/*
* Inicializa todo i tabuleiro com água
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
* Verifica se um navio cabe nos limites e não sobrepões outro navio já posicionado
*/ 
bool can_place_ship(
    const int board[BOARD_SIZE][BOARD_SIZE],
    int start_row,
    int start_col,
    int ship_len,
    Orientation orient
)
{
    // Validação de limites
    if (orient == HORIZONTAL) {
        // Última coluna ocupada será start_col + ship_len - 1
        if (start_row < 0 || start_row >= BOARD_SIZE)
        {
            return false;
        }

        if (start_col < 0 || (start_col + ship_len -1) >= BOARD_SIZE) {
            return false;
        }
    } else { // VERTICAL
        // Última linha ocupada será start_rol + ship_len - 1
        if (start_col < 0 || start_col >= BOARD_SIZE) {
            return false;
        } 

        if (start_row < 0 || (start_row + ship_len - 1) >= BOARD_SIZE) {
            return false;
        }
    }

    // Verificação de sobreposição (todas as casas devem estar com WATER)
    for (int i = 0; i < ship_len; i++) {
        int r = start_row + (orient == VERTICAL ? i : 0);
        int c = start_col + (orient == HORIZONTAL ? i : 0);

        if (board[r][c] != WATER) {
            return false; // Encotrou célula já ocupada
        }
    }

    return true;
}

/*
* Copia os valores do veotr 'ship[]' para o tabuleiro, de acordo com a orientação.
* Retorna true em caso de sucesso, false se não puder posicionar (fora de limite/sobreposição).
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

    for (int i = 0; i < ship_len; i++) {
        int r = start_row + (orient == VERTICAL ? i : 0);
        int c = start_col + (orient == HORIZONTAL ? i : 0);
        board[r][c] = ship[i];
    }

    return true;
}

/*
* Imprime o tabuleiro de forma organizada (0 = água, 3 = navio).
*/
void print_board(const int board[BOARD_SIZE][BOARD_SIZE]) {
    printf("Tabuleiro (0 = água, 3 = navio\n\n)");
    
    // Cabeçalho de colunas
    for (int c = 0; c < BOARD_SIZE; c++) {
        printf("%2d", c);
    }

    printf("\n");

    for (int r = 0; r < BOARD_SIZE; r++) {
        printf("%2d", r); // Índice da linha
        for(int c = 0; c < BOARD_SIZE; c++) {
            printf("%2d", board[r][c]);
        }

        printf("\n");
    }
}


int main() {
    // Tabuleiro 10X10
    int board[BOARD_SIZE][BOARD_SIZE];
    init_board(board);

    // Dois vavios (vetores 1D) com tamanho fixo = 3
    // cada posição do navio contém o valor 3 (que será copiado para a matriz)
    int ship_horizontal[SHIP_SIZE]  = {SHIP, SHIP, SHIP};
    int ship_vertical[SHIP_SIZE]    = {SHIP, SHIP, SHIP};

    // Coordenadas definidas no código
    //  - Navio horizontal começando em (linha=2, coluna=4)
    //  - Navio vertical começando em (linha=5, coluna=1)
    // Essas escolhas já garantem que não haja sobreposição.
    int h_row = 2, h_col = 4;
    int v_row = 5, v_col = 1;

    // Posicionamento com validação
    if (!place_ship_from_array(board, h_row, h_col, ship_horizontal, SHIP_SIZE, HORIZONTAL)) {
        printf("ERRO: Nao foi possivel posicionar o navio horizontal em (%d,%d).\n", h_row, h_col);
        return 1;
    }

    if (!place_ship_from_array(board, v_row, v_col, ship_vertical, SHIP_SIZE, VERTICAL)) {
        printf("ERRO: Nao foi possivel posicionar o navio vertical em (%d,%d).\n", v_row, v_col);
        return 1;
    }

    // Exibe tabuleiro final
    print_board(board);

    return 0;
}
