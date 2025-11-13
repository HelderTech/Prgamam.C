/*
  Batalha Naval - Projeto para disciplina
  Autor: Hélder (estudante)
  Linguagem: C (conceitos de vetores, matrizes, loops aninhados e condicionais)
  Niveis: Novato, Aventureiro, Mestre
  Compilar: gcc -o batalha main.c
  Executar: ./batalha
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX 8            // tamanho do tabuleiro (8x8)
#define MAX_SHIPS 5      // numero de navios por jogador
#define SAVEFILE "score.txt"

typedef enum { false, true } bool;

/* Representacao:
   '.' agua
   'S' navio (visivel apenas no proprio tabuleiro)
   'X' acerto
   'O' erro (agua atingida)
*/

// funcoes utilitarias
void clear_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) ;
}

void pause() {
    printf("Pressione Enter para continuar...");
    clear_stdin();
}

void init_board(char board[MAX][MAX], char fill) {
    for (int i = 0; i < MAX; i++)
        for (int j = 0; j < MAX; j++)
            board[i][j] = fill;
}

void print_board_hidden(char board[MAX][MAX]) {
    // exibe tabuleiro para o jogador adversario (oculta 'S')
    printf("  ");
    for (int c = 0; c < MAX; c++) printf("%d ", c);
    printf("\n");
    for (int i = 0; i < MAX; i++) {
        printf("%d ", i);
        for (int j = 0; j < MAX; j++) {
            char ch = board[i][j];
            if (ch == 'S') ch = '.';
            printf("%c ", ch);
        }
        printf("\n");
    }
}

void print_board_reveal(char board[MAX][MAX]) {
    // exibe proprio tabuleiro com navios visiveis
    printf("  ");
    for (int c = 0; c < MAX; c++) printf("%d ", c);
    printf("\n");
    for (int i = 0; i < MAX; i++) {
        printf("%d ", i);
        for (int j = 0; j < MAX; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
}

bool inside(int r, int c) {
    return r >= 0 && r < MAX && c >= 0 && c < MAX;
}

/* =========================
   Módulo Novato
   - usa um vetor 1D para armazenar posicoes de navios simples 1x1
   - facil de entender para estudante do 1o ano
   - navios posicionados aleatoriamente
   ========================= */
void module_novato(char player_board[MAX][MAX], char cpu_board[MAX][MAX]) {
    printf("Modo Novato\n");
    // Representacao 1D do tabuleiro: index = r*MAX + c
    int cells = MAX * MAX;
    int ship_positions[MAX_SHIPS];

    // inicializar vetores
    for (int i = 0; i < MAX_SHIPS; i++) ship_positions[i] = -1;

    // posiciona navios do jogador (auto)
    int placed = 0;
    while (placed < MAX_SHIPS) {
        int idx = rand() % cells;
        bool exists = false;
        for (int k = 0; k < placed; k++) if (ship_positions[k] == idx) exists = true;
        if (!exists) {
            ship_positions[placed++] = idx;
        }
    }
    // converte vetor 1D para board
    for (int k = 0; k < MAX_SHIPS; k++) {
        int idx = ship_positions[k];
        int r = idx / MAX;
        int c = idx % MAX;
        player_board[r][c] = 'S';
    }

    // posiciona navios do cpu (aleatorio, 1x1)
    placed = 0;
    while (placed < MAX_SHIPS) {
        int r = rand() % MAX;
        int c = rand() % MAX;
        if (cpu_board[r][c] != 'S') {
            cpu_board[r][c] = 'S';
            placed++;
        }
    }

    // gameplay simples: jogador ataca ate afundar todos os navios do cpu
    int cpu_remaining = MAX_SHIPS;
    int player_remaining = MAX_SHIPS;
    while (cpu_remaining > 0 && player_remaining > 0) {
        printf("\nSeu tabuleiro:\n");
        print_board_reveal(player_board);
        printf("\nTabuleiro adversario:\n");
        print_board_hidden(cpu_board);

        int r, c;
        printf("\nDigite a linha e coluna para atacar (ex: 2 3): ");
        if (scanf("%d %d", &r, &c) != 2) { clear_stdin(); printf("Entrada invalida\n"); continue; }
        if (!inside(r,c)) { printf("Coordenada fora do tabuleiro\n"); continue; }

        if (cpu_board[r][c] == 'S') {
            cpu_board[r][c] = 'X';
            cpu_remaining--;
            printf("Acertou! navio destruido. Restam %d\n", cpu_remaining);
        } else if (cpu_board[r][c] == '.' ) {
            cpu_board[r][c] = 'O';
            printf("Agua\n");
        } else {
            printf("Posicao ja atacada\n");
        }

        // ataque CPU aleatorio simples
        int ar, ac;
        do {
            ar = rand() % MAX;
            ac = rand() % MAX;
        } while (player_board[ar][ac] == 'X' || player_board[ar][ac] == 'O');
        if (player_board[ar][ac] == 'S') {
            player_board[ar][ac] = 'X';
            player_remaining--;
            printf("CPU acertou um de seus navios. Seus navios restantes: %d\n", player_remaining);
        } else {
            player_board[ar][ac] = 'O';
            printf("CPU errou\n");
        }
    }

    if (cpu_remaining == 0) printf("\nVoce venceu no modo Novato!\n");
    else printf("\nCPU venceu no modo Novato\n");
}

/* =========================
   Módulo Aventureiro
   - usa matrizes, posiciona navios maiores e permite diagonal
   - navios: porta-aviões 3, destruidor 2, fragata 2, barcos 1
   - permite escolher posicao ou random
   ========================= */

void place_ship_matrix(char board[MAX][MAX], int size, int r, int c, int dr, int dc) {
    for (int k = 0; k < size; k++) {
        board[r + k*dr][c + k*dc] = 'S';
    }
}

bool can_place(char board[MAX][MAX], int size, int r, int c, int dr, int dc) {
    for (int k = 0; k < size; k++) {
        int rr = r + k*dr;
        int cc = c + k*dc;
        if (!inside(rr, cc)) return false;
        if (board[rr][cc] == 'S') return false;
    }
    return true;
}

void module_aventureiro(char player_board[MAX][MAX], char cpu_board[MAX][MAX]) {
    printf("Modo Aventureiro\n");
    // limpar boards
    init_board(player_board, '.');
    init_board(cpu_board, '.');

    int ship_sizes[] = {3, 2, 2, 1, 1}; // exemplos
    int nships = sizeof(ship_sizes) / sizeof(ship_sizes[0]);

    // posicionar navios do CPU aleatoriamente, incluindo diagonal
    for (int i = 0; i < nships; i++) {
        int size = ship_sizes[i];
        bool placed = false;
        int tries = 0;
        while (!placed && tries < 500) {
            int r = rand() % MAX;
            int c = rand() % MAX;
            // direcao: dr dc pode ser -1,0,1 inclusive diagonal
            int dr = (rand() % 3) - 1;
            int dc = (rand() % 3) - 1;
            if (dr == 0 && dc == 0) continue;
            if (can_place(cpu_board, size, r, c, dr, dc)) {
                place_ship_matrix(cpu_board, size, r, c, dr, dc);
                placed = true;
            }
            tries++;
        }
        if (!placed) {
            // fallback tenta colocar em qualquer posicao horizontal
            for (int r = 0; r < MAX && !placed; r++)
                for (int c = 0; c < MAX && !placed; c++)
                    if (c + size <= MAX && can_place(cpu_board, size, r, c, 0, 1)) {
                        place_ship_matrix(cpu_board, size, r, c, 0, 1);
                        placed = true;
                    }
        }
    }

    // posicao simples do jogador: aleatoria tambem para simplificar
    for (int i = 0; i < nships; i++) {
        int size = ship_sizes[i];
        bool placed = false;
        while (!placed) {
            int r = rand() % MAX;
            int c = rand() % MAX;
            int dr = (rand() % 3) - 1;
            int dc = (rand() % 3) - 1;
            if (dr == 0 && dc == 0) continue;
            if (can_place(player_board, size, r, c, dr, dc)) {
                place_ship_matrix(player_board, size, r, c, dr, dc);
                placed = true;
            }
        }
    }

    // gameplay similar ao novato, porem aprimorei as opcoes e informacoes
    int cpu_remaining = 0;
    int player_remaining = 0;
    for (int i = 0; i < MAX; i++) for (int j = 0; j < MAX; j++) {
        if (cpu_board[i][j] == 'S') cpu_remaining++;
        if (player_board[i][j] == 'S') player_remaining++;
    }

    while (cpu_remaining > 0 && player_remaining > 0) {
        printf("\nSeu tabuleiro:\n");
        print_board_reveal(player_board);
        printf("\nTabuleiro adversario:\n");
        print_board_hidden(cpu_board);

        int r, c;
        printf("\nDigite a linha e coluna para atacar (ex: 2 3): ");
        if (scanf("%d %d", &r, &c) != 2) { clear_stdin(); printf("Entrada invalida\n"); continue; }
        if (!inside(r,c)) { printf("Coordenada fora do tabuleiro\n"); continue; }

        if (cpu_board[r][c] == 'S') {
            cpu_board[r][c] = 'X';
            cpu_remaining--;
            printf("Acertou! navio atingido. Restam %d\n", cpu_remaining);
        } else if (cpu_board[r][c] == '.' ) {
            cpu_board[r][c] = 'O';
            printf("Agua\n");
        } else {
            printf("Posicao ja atacada\n");
        }

        // CPU ataca com estrategia simples: tenta vizinhos apos acerto
        static int last_hit_r = -1, last_hit_c = -1;
        int ar=-1, ac=-1;
        // se houve ultimo acerto, tenta atingir vizinhos
        if (last_hit_r != -1) {
            int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
            for (int d = 0; d < 4; d++) {
                int rr = last_hit_r + dirs[d][0];
                int cc = last_hit_c + dirs[d][1];
                if (inside(rr,cc) && player_board[rr][cc] != 'X' && player_board[rr][cc] != 'O') {
                    ar = rr; ac = cc; break;
                }
            }
        }
        if (ar == -1) {
            do {
                ar = rand() % MAX; ac = rand() % MAX;
            } while (player_board[ar][ac] == 'X' || player_board[ar][ac] == 'O');
        }
        if (player_board[ar][ac] == 'S') {
            player_board[ar][ac] = 'X';
            player_remaining--;
            last_hit_r = ar; last_hit_c = ac;
            printf("CPU acertou um de seus navios. Restam %d\n", player_remaining);
        } else {
            player_board[ar][ac] = 'O';
            printf("CPU errou\n");
            // se errou, zera last_hit
            last_hit_r = -1; last_hit_c = -1;
        }
    }

    if (cpu_remaining == 0) printf("\nVoce venceu no modo Aventureiro!\n");
    else printf("\nCPU venceu no modo Aventureiro\n");
}

/* =========================
   Módulo Mestre
   - implementa habilidades especiais com areas de efeito
   - habilidades: cone (direcional), cruz (plus), octaedro (diamante baseado em manhattan)
   - usa condicionais dentro de loops aninhados para calcular area de efeito
   ========================= */

void apply_attack(char board[MAX][MAX], int r, int c) {
    if (!inside(r,c)) return;
    if (board[r][c] == 'S') board[r][c] = 'X';
    else if (board[r][c] == '.') board[r][c] = 'O';
}

void ability_cone(char board[MAX][MAX], int r, int c, int dir, int depth) {
    // dir: 0 norte, 1 sul, 2 oeste, 3 leste
    // cone triangular: para cada d de 0..depth-1, largura aumenta
    for (int d = 0; d < depth; d++) {
        int width = d; // aumenta a largura gradualmente
        for (int w = -width; w <= width; w++) {
            int rr = r, cc = c;
            if (dir == 0) { rr = r - d; cc = c + w; } // norte
            if (dir == 1) { rr = r + d; cc = c + w; } // sul
            if (dir == 2) { rr = r + w; cc = c - d; } // oeste
            if (dir == 3) { rr = r + w; cc = c + d; } // leste
            apply_attack(board, rr, cc);
        }
    }
}

void ability_cross(char board[MAX][MAX], int r, int c, int radius) {
    // forma de cruz: centro mais braços com comprimento radius
    apply_attack(board, r, c);
    for (int d = 1; d <= radius; d++) {
        apply_attack(board, r - d, c); // norte
        apply_attack(board, r + d, c); // sul
        apply_attack(board, r, c - d); // oeste
        apply_attack(board, r, c + d); // leste
    }
}

void ability_octaedro(char board[MAX][MAX], int r, int c, int radius) {
    // octaedro aproximado via distancia de Manhattan <= radius
    for (int i = 0; i < MAX; i++) for (int j = 0; j < MAX; j++) {
        int dist = abs(i - r) + abs(j - c);
        if (dist <= radius) apply_attack(board, i, j);
    }
}

void module_mestre(char player_board[MAX][MAX], char cpu_board[MAX][MAX]) {
    printf("Modo Mestre\n");
    init_board(player_board, '.');
    init_board(cpu_board, '.');

    // instalar alguns navios aleatorios para ambos
    int ship_sizes[] = {3,2,2,1};
    int nships = sizeof(ship_sizes) / sizeof(ship_sizes[0]);
    // CPU
    for (int i = 0; i < nships; i++) {
        int size = ship_sizes[i];
        bool placed = false;
        int tries = 0;
        while (!placed && tries < 500) {
            int r = rand() % MAX;
            int c = rand() % MAX;
            int dir = rand() % 4;
            int dr = (dir == 0) ? -1 : (dir == 1) ? 1 : 0;
            int dc = (dir == 2) ? -1 : (dir == 3) ? 1 : 0;
            if (dr == 0 && dc == 0) { tries++; continue; }
            if (can_place(cpu_board, size, r, c, dr, dc)) {
                place_ship_matrix(cpu_board, size, r, c, dr, dc);
                placed = true;
            }
            tries++;
        }
        if (!placed) {
            // fallback horizontal
            for (int r = 0; r < MAX && !placed; r++)
                for (int c = 0; c < MAX && !placed; c++)
                    if (c + size <= MAX && can_place(cpu_board, size, r, c, 0, 1)) {
                        place_ship_matrix(cpu_board, size, r, c, 0, 1);
                        placed = true;
                    }
        }
    }
    // jogador auto posicionado para simplificar
    for (int i = 0; i < nships; i++) {
        int size = ship_sizes[i];
        bool placed = false;
        while (!placed) {
            int r = rand() % MAX;
            int c = rand() % MAX;
            int dr = (rand() % 3) - 1;
            int dc = (rand() % 3) - 1;
            if (dr == 0 && dc == 0) continue;
            if (can_place(player_board, size, r, c, dr, dc)) {
                place_ship_matrix(player_board, size, r, c, dr, dc);
                placed = true;
            }
        }
    }

    int cpu_remaining = 0, player_remaining = 0;
    for (int i = 0; i < MAX; i++) for (int j = 0; j < MAX; j++) {
        if (cpu_board[i][j] == 'S') cpu_remaining++;
        if (player_board[i][j] == 'S') player_remaining++;
    }

    printf("Habilidades disponiveis:\n");
    printf("1 - Cone (direcional)\n");
    printf("2 - Cruz\n");
    printf("3 - Octaedro (diamante via distancia manhattan)\n");

    while (cpu_remaining > 0 && player_remaining > 0) {
        printf("\nSeu tabuleiro:\n");
        print_board_reveal(player_board);
        printf("\nTabuleiro adversario:\n");
        print_board_hidden(cpu_board);

        int choice;
        printf("\nEscolha habilidade (1-3) ou 0 para ataque simples: ");
        if (scanf("%d", &choice) != 1) { clear_stdin(); printf("Entrada invalida\n"); continue; }

        int r, c;
        if (choice >= 1 && choice <= 3) {
            printf("Digite linha e coluna do centro do efeito: ");
            if (scanf("%d %d", &r, &c) != 2) { clear_stdin(); printf("Entrada invalida\n"); continue; }
            if (!inside(r,c)) { printf("Coordenada invalida\n"); continue; }
            if (choice == 1) {
                int dir;
                printf("Direcao do cone 0-norte 1-sul 2-oeste 3-leste: ");
                if (scanf("%d", &dir) != 1) { clear_stdin(); printf("Entrada invalida\n"); continue; }
                int depth;
                printf("Profundidade do cone (1-3): ");
                if (scanf("%d", &depth) != 1) { clear_stdin(); printf("Entrada invalida\n"); continue; }
                ability_cone(cpu_board, r, c, dir, depth);
            } else if (choice == 2) {
                int radius;
                printf("Raio da cruz (1-3): ");
                if (scanf("%d", &radius) != 1) { clear_stdin(); printf("Entrada invalida\n"); continue; }
                ability_cross(cpu_board, r, c, radius);
            } else {
                int radius;
                printf("Raio do octaedro (1-3): ");
                if (scanf("%d", &radius) != 1) { clear_stdin(); printf("Entrada invalida\n"); continue; }
                ability_octaedro(cpu_board, r, c, radius);
            }
        } else if (choice == 0) {
            printf("Digite linha e coluna para ataque simples: ");
            if (scanf("%d %d", &r, &c) != 2) { clear_stdin(); printf("Entrada invalida\n"); continue; }
            apply_attack(cpu_board, r, c);
        } else {
            printf("Opcao invalida\n");
            continue;
        }

        // recalcula cpu_remaining
        cpu_remaining = 0;
        for (int i = 0; i < MAX; i++) for (int j = 0; j < MAX; j++) if (cpu_board[i][j] == 'S') cpu_remaining++;

        printf("Depois de seu turno, navios adversarios restantes: %d\n", cpu_remaining);

        // CPU faz ataque simples aleatorio
        int ar, ac;
        do {
            ar = rand() % MAX; ac = rand() % MAX;
        } while (player_board[ar][ac] == 'X' || player_board[ar][ac] == 'O');
        if (player_board[ar][ac] == 'S') {
            player_board[ar][ac] = 'X';
            player_remaining--;
            printf("CPU acertou um de seus navios. Restam %d\n", player_remaining);
        } else {
            player_board[ar][ac] = 'O';
            printf("CPU errou\n");
        }
    }

    if (cpu_remaining == 0) printf("\nVoce venceu no modo Mestre!\n");
    else printf("\nCPU venceu no modo Mestre\n");
}

/* =========================
   Funcoes auxiliares e menu
   ========================= */

void header() {
    printf("=====================================\n");
    printf("       Jogo Batalha Naval - C        \n");
    printf("=====================================\n");
    printf("Tamanho do tabuleiro: %dx%d\n", MAX, MAX);
    printf("Navios por jogador: %d (variavel por modo)\n", MAX_SHIPS);
}

void save_score(const char *mode, int won) {
    FILE *f = fopen(SAVEFILE, "a");
    if (!f) return;
    time_t t = time(NULL);
    fprintf(f, "%s - %s - %s", asctime(localtime(&t)), mode, won ? "VENCEU\n" : "PERDEU\n");
    fclose(f);
}

int main() {
    srand((unsigned int) time(NULL));
    char player_board[MAX][MAX];
    char cpu_board[MAX][MAX];
    init_board(player_board, '.');
    init_board(cpu_board, '.');

    header();

    int option = -1;
    while (option != 0) {
        printf("\nEscolha o modo:\n");
        printf("1 - Novato (vetor 1D, navios 1x1)\n");
        printf("2 - Aventureiro (matrizes, navios maiores e diagonal)\n");
        printf("3 - Mestre (habilidades com area de efeito)\n");
        printf("0 - Sair\n");
        printf("Opcao: ");
        if (scanf("%d", &option) != 1) {
            clear_stdin();
            printf("Entrada invalida\n");
            continue;
        }
        switch (option) {
            case 1:
                init_board(player_board, '.');
                init_board(cpu_board, '.');
                module_novato(player_board, cpu_board);
                save_score("Novato", 1); // simplificacao: salva sempre
                break;
            case 2:
                init_board(player_board, '.');
                init_board(cpu_board, '.');
                module_aventureiro(player_board, cpu_board);
                save_score("Aventureiro", 1);
                break;
            case 3:
                init_board(player_board, '.');
                init_board(cpu_board, '.');
                module_mestre(player_board, cpu_board);
                save_score("Mestre", 1);
                break;
            case 0:
                printf("Encerrando\n");
                break;
            default:
                printf("Opcao invalida\n");
        }
        pause();
    }

    return 0;
}

