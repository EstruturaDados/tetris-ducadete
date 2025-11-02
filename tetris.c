#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define CAP_FILA   5
#define CAP_PILHA  3

typedef struct {
    char tipo;   // 'I', 'O', 'T', 'L'
    int  id;     // identificador único sequencial
} Peca;

/* -------------------------- FILA CIRCULAR -------------------------- */
typedef struct {
    Peca dados[CAP_FILA];
    int ini;       // índice do primeiro elemento
    int fim;       // índice da próxima posição livre
    int tam;       // quantidade atual de elementos
} Fila;

void inicializarFila(Fila *f) {
    f->ini = 0;
    f->fim = 0;
    f->tam = 0;
}

bool filaVazia(const Fila *f) { return f->tam == 0; }
bool filaCheia(const Fila *f) { return f->tam == CAP_FILA; }

bool enqueue(Fila *f, Peca p) {
    if (filaCheia(f)) return false;
    f->dados[f->fim] = p;
    f->fim = (f->fim + 1) % CAP_FILA;
    f->tam++;
    return true;
}

bool dequeue(Fila *f, Peca *removida) {
    if (filaVazia(f)) return false;
    if (removida) *removida = f->dados[f->ini];
    f->ini = (f->ini + 1) % CAP_FILA;
    f->tam--;
    return true;
}

/* Acessa o i-ésimo elemento lógico (0 = frente), respeitando circularidade */
Peca* filaAcessar(Fila *f, int i) {
    if (i < 0 || i >= f->tam) return NULL;
    int idx = (f->ini + i) % CAP_FILA;
    return &f->dados[idx];
}

/* ------------------------------ PILHA ------------------------------ */
typedef struct {
    Peca dados[CAP_PILHA];
    int topo; // -1 vazia; topo aponta para o último elemento válido
} Pilha;

void inicializarPilha(Pilha *p) { p->topo = -1; }
bool pilhaVazia(const Pilha *p) { return p->topo == -1; }
bool pilhaCheia(const Pilha *p) { return p->topo == CAP_PILHA - 1; }

bool push(Pilha *p, Peca x) {
    if (pilhaCheia(p)) return false;
    p->dados[++p->topo] = x;
    return true;
}

bool pop(Pilha *p, Peca *x) {
    if (pilhaVazia(p)) return false;
    if (x) *x = p->dados[p->topo];
    p->topo--;
    return true;
}

/* -------------------- GERAÇÃO AUTOMÁTICA DE PEÇAS ------------------ */
Peca gerarPeca(void) {
    static int proxId = 0;           // id sequencial
    const char tipos[] = { 'I', 'O', 'T', 'L' };
    Peca p;
    p.tipo = tipos[rand() % 4];
    p.id   = proxId++;
    return p;
}

/* ------------------------- IMPRESSÃO/ESTADO ------------------------ */
void mostrarFila(const Fila *f) {
    printf("Fila de peças (%d/%d): ", f->tam, CAP_FILA);
    if (filaVazia(f)) { printf("(vazia)\n"); return; }

    for (int i = 0; i < f->tam; i++) {
        int idx = (f->ini + i) % CAP_FILA;
        printf("[%c %d] ", f->dados[idx].tipo, f->dados[idx].id);
    }
    printf("\n");
}

void mostrarPilha(const Pilha *p) {
    printf("Pilha de reserva (%d/%d): ", p->topo + 1, CAP_PILHA);
    if (pilhaVazia(p)) { printf("(vazia)\n"); return; }

    // imprime do topo ao fundo
    for (int i = p->topo; i >= 0; i--) {
        printf("[%c %d] ", p->dados[i].tipo, p->dados[i].id);
    }
    printf("\n");
}

void mostrarEstado(const Fila *f, const Pilha *p) {
    puts("-------------------------------------------------");
    mostrarFila(f);
    if (p) mostrarPilha(p);
    puts("-------------------------------------------------");
}

/* ------------------------- OPERAÇÕES MESTRE ------------------------ */
/* 4) Trocar frente da fila com topo da pilha */
bool trocarFrenteComTopo(Fila *f, Pilha *p) {
    if (filaVazia(f) || pilhaVazia(p)) return false;
    int idxFrente = f->ini;
    Peca tmp = f->dados[idxFrente];
    f->dados[idxFrente] = p->dados[p->topo];
    p->dados[p->topo] = tmp;
    return true;
}

/* 5) Trocar 3 primeiros da fila com as 3 peças da pilha */
bool trocarTresFilaComPilha(Fila *f, Pilha *p) {
    if (f->tam < 3 || p->topo + 1 != 3) return false; // pilha precisa ter exatamente 3
    for (int i = 0; i < 3; i++) {
        int idxFila = (f->ini + i) % CAP_FILA;
        int idxPilha = p->topo - i; // topo, topo-1, topo-2
        Peca tmp = f->dados[idxFila];
        f->dados[idxFila] = p->dados[idxPilha];
        p->dados[idxPilha] = tmp;
    }
    return true;
}

/* ------------------------------ MENU ------------------------------- */
void imprimirMenu(void) {
    puts("\n=== TETRIS STACK ===");
    puts("1 - Jogar peça (dequeue) [auto repoe para manter 5]");
    puts("2 - Inserir nova peça (enqueue)");
    puts("3 - Reservar peça da frente (push na pilha)");
    puts("4 - Usar peça reservada (pop da pilha)");
    puts("5 - Trocar frente da fila com topo da pilha");
    puts("6 - Trocar 3 primeiros da fila com as 3 da pilha");
    puts("9 - Mostrar estado");
    puts("0 - Sair");
    printf("Escolha: ");
}

/* ------------------------------- MAIN ------------------------------ */
int main(void) {
    srand((unsigned)time(NULL));

    Fila fila;
    Pilha pilha;
    inicializarFila(&fila);
    inicializarPilha(&pilha);

    /* Inicializa a fila com 5 peças (requisito do nível Novato) */
    for (int i = 0; i < CAP_FILA; i++) {
        enqueue(&fila, gerarPeca());
    }

    int opcao;
    do {
        mostrarEstado(&fila, &pilha);
        imprimirMenu();
        if (scanf("%d", &opcao) != 1) {
            // limpeza simples do buffer e continua
            int c; while ((c = getchar()) != '\n' && c != EOF) {}
            puts("Entrada inválida.");
            continue;
        }

        // consome '\n' restante
        int c; while ((c = getchar()) != '\n' && c != EOF) {}

        switch (opcao) {
            case 1: { // Jogar peça (dequeue) — e reabastece para manter 5
                Peca jogada;
                if (dequeue(&fila, &jogada)) {
                    printf("Jogou a peça: [%c %d]\n", jogada.tipo, jogada.id);
                    // Aventureiro: manter sempre cheia (auto-reposição)
                    if (enqueue(&fila, gerarPeca())) {
                        puts("Fila reabastecida automaticamente.");
                    } else {
                        puts("Aviso: não foi possível reabastecer (fila cheia).");
                    }
                } else {
                    puts("Fila vazia: não há peça para jogar.");
                }
                break;
            }
            case 2: { // Inserir nova peça (enqueue)
                Peca nova = gerarPeca();
                if (enqueue(&fila, nova)) {
                    printf("Inserida nova peça: [%c %d]\n", nova.tipo, nova.id);
                } else {
                    puts("Fila cheia: não foi possível inserir.");
                }
                break;
            }
            case 3: { // Reservar peça (push): tira da frente da fila e envia para pilha
                if (filaVazia(&fila)) {
                    puts("Fila vazia: nada para reservar.");
                    break;
                }
                if (pilhaCheia(&pilha)) {
                    puts("Pilha cheia: não é possível reservar mais peças.");
                    break;
                }
                Peca frente;
                dequeue(&fila, &frente);
                push(&pilha, frente);
                printf("Reservada a peça [%c %d] para a pilha.\n", frente.tipo, frente.id);
                // mantém a fila cheia
                if (enqueue(&fila, gerarPeca())) {
                    puts("Fila reabastecida automaticamente.");
                }
                break;
            }
            case 4: { // Usar peça reservada (pop)
                Peca usada;
                if (pop(&pilha, &usada)) {
                    printf("Usou a peça reservada: [%c %d]\n", usada.tipo, usada.id);
                } else {
                    puts("Pilha vazia: não há peça reservada para usar.");
                }
                break;
            }
            case 5: { // Trocar frente da fila com topo da pilha
                if (trocarFrenteComTopo(&fila, &pilha)) {
                    puts("Troca realizada: frente da fila ↔ topo da pilha.");
                } else {
                    puts("Não foi possível trocar (verifique se há peças em ambas).");
                }
                break;
            }
            case 6: { // Trocar 3 primeiros da fila com as 3 da pilha
                if (trocarTresFilaComPilha(&fila, &pilha)) {
                    puts("Troca 3↔3 realizada com sucesso.");
                } else {
                    puts("Não foi possível trocar (fila precisa de 3 e pilha de exatamente 3).");
                }
                break;
            }
            case 9: { // Mostrar estado
                // já é mostrado a cada loop, mas deixo aqui para debug manual
                break;
            }
            case 0:
                puts("Saindo. Até a próxima rodada!");
                break;

            default:
                puts("Opção inválida.");
        }
    } while (opcao != 0);

    return 0;
}
