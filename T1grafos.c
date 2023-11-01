#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct grafo {
    int num_vertices;
    int num_arestas;
    int tipo;
    int valorado;
    double densidade;
    struct no **vertices;
    int **matriz_adjacencias;
};

struct no {
    int id;
    int grau;
    int distancia;
    int visitado;
    struct aresta *aresta;
    struct no *proxima;
    struct no *pai;
    struct aresta **lista_adjacencias;
};

struct aresta {
    int origem;
    int destino;
    int peso;
    struct aresta *proxima;
};

struct heap {
    int tamanho;
    int elementos_tamanho;
    struct no **elementos;
};

struct fila {
    struct no *inicio;
    struct no *fim;
};

struct pilha {
    struct no *topo;
};

//funções de pilha
struct pilha *criar_pilha() {
    struct pilha *pilha = (struct pilha *)malloc(sizeof(struct pilha));
    pilha->topo = NULL;
    return pilha;
}
int pilha_vazia(struct pilha *pilha) {
    return pilha->topo == NULL;
}
void empilhar(struct pilha *pilha, struct no *elemento) {
    struct no *novo_no = (struct no *)malloc(sizeof(struct no));
    novo_no->id = elemento->id;
    novo_no->grau = elemento->grau;
    novo_no->distancia = elemento->distancia;
    novo_no->visitado = elemento->visitado;
    novo_no->aresta = elemento->aresta;
    novo_no->proxima = pilha->topo;
    novo_no->pai = elemento->pai;
    novo_no->lista_adjacencias = elemento->lista_adjacencias;
    pilha->topo = novo_no;
}
struct no *desempilhar(struct pilha *pilha) {
    if (pilha->topo == NULL) {
        return NULL;
    }
    struct no *no_removido = pilha->topo;
    pilha->topo = no_removido->proxima;
    return no_removido;
}
void liberar_pilha(struct pilha *pilha) {
    while (pilha->topo != NULL) {
        struct no *no_removido = pilha->topo;
        pilha->topo = no_removido->proxima;
        free(no_removido);
    }
    free(pilha);
}

//funções de heap
struct heap *criar_heap(int tamanho) {
    struct heap *heap = malloc(sizeof(struct heap));
    heap->tamanho = 0;
    heap->elementos_tamanho = tamanho;
    heap->elementos = malloc(sizeof(struct no *) * tamanho);
    return heap;
}
void heapify(struct heap *heap, int posicao) {
    int i = posicao;
    while (i > 0 && heap->elementos[i]->distancia < heap->elementos[i / 2]->distancia) {
        // Trocando os elementos
        struct no *temp = heap->elementos[i];
        heap->elementos[i] = heap->elementos[i / 2];
        heap->elementos[i / 2] = temp;

        // Atualizando a posição
        i = i / 2;
    }
}
void inserir_heap(struct heap *heap, struct no *elemento) {
    if (heap->tamanho == heap->elementos_tamanho) {
        // A heap está cheia
        return;
    }

    // Adicionando o elemento na última posição da heap
    heap->elementos[heap->tamanho++] = elemento;

    // Heapify o elemento
    heapify(heap, heap->tamanho - 1);
}
struct no *remover_heap(struct heap *heap) {
    if (heap->tamanho == 0) {
        // A heap está vazia
        return NULL;
    }

    // Removendo o elemento da primeira posição da heap
    struct no *elemento = heap->elementos[0];
    heap->elementos[0] = heap->elementos[heap->tamanho - 1];
    heap->tamanho--;

    // Heapify o elemento
    heapify(heap, 0);

    return elemento;
}
void empurrar_heap(struct heap *heap, struct no *elemento) {
    heap->elementos[heap->tamanho++] = elemento;
    heapify(heap, heap->tamanho - 1);
}
int heap_vazio(struct heap *heap) {
    return heap->tamanho == 0;
}
void liberar_heap(struct heap *heap) {
    free(heap->elementos);
    free(heap);
}

//funções de fila
struct fila *criar_fila() {
    struct fila *fila = malloc(sizeof(struct fila));
    fila->inicio = NULL;
    fila->fim = NULL;
    return fila;
}
int fila_vazia(struct fila *fila) {
    return fila->inicio == NULL;
}
void enfileirar(struct fila *fila, struct no *elemento) {
    if (fila_vazia(fila)) {
        fila->inicio = fila->fim = elemento;
    } else {
        fila->fim->proxima = elemento;
        fila->fim = elemento;
    }
}
struct no *desenfileirar(struct fila *fila) {
    struct no *elemento = fila->inicio;
    if (fila->inicio == fila->fim) {
        fila->inicio = fila->fim = NULL;
    } else {
        fila->inicio = fila->inicio->proxima;
    }
    return elemento;
}
void liberar_fila(struct fila *fila) {
    struct no *no = fila->inicio;
    while (no != NULL) {
        struct no *proximo = no->proxima;
        free(no);
        no = proximo;
    }
    free(fila);
}


int existe_aresta(struct grafo *g, int v1, int v2) {
    struct no *vertice = g->vertices[v1];
    struct aresta *aresta = vertice->aresta;
    while (aresta != NULL) {
        if (aresta->destino == v2) {
            return 1; // A aresta entre v1 e v2 já existe
        }
        aresta = aresta->proxima;
    }
    return 0; // A aresta entre v1 e v2 não existe
}

void adicionar_aresta(struct grafo *g, int origem, int destino, int peso) {
    struct aresta *aresta = malloc(sizeof(struct aresta));
    if (aresta == NULL) {
        printf("Erro ao alocar memória para a aresta\n");
        return;
    }

    aresta->origem = origem;
    aresta->destino = destino;
    aresta->peso = peso;

    struct no *vertice = g->vertices[origem];

    if (vertice->aresta == NULL) {
        vertice->aresta = aresta;
        aresta->proxima = NULL; // Definindo o próximo como NULL
    } else {
        struct aresta *atual = vertice->aresta;
        while (atual->proxima != NULL) {
            atual = atual->proxima;
        }
        atual->proxima = aresta;
        aresta->proxima = NULL; // Definindo o próximo como NULL
    }

    g->num_arestas++;
}

void adicionar_arco(struct grafo *g, int origem, int destino, int peso) {
    adicionar_aresta(g, origem, destino, peso);
    adicionar_aresta(g, destino, origem, peso);
}

struct grafo *criar_grafo_aleatorio(int num_vertices, double densidade, char *arquivo, int tipo, int valorado) {
    struct grafo *g = malloc(sizeof(struct grafo));
    if (g == NULL) {
        printf("Erro ao alocar memória para o grafo\n");
        return NULL; // Tratar o erro adequadamente de acordo com o seu caso
    }
    g->num_vertices = num_vertices;
    g->num_arestas = 0;
    g->tipo = tipo; // Defina o valor correto de acordo com a entrada
    g->valorado = valorado; // Defina o valor correto de acordo com a entrada
    g->vertices = malloc(sizeof(struct no *) * num_vertices);
    if (g->vertices == NULL) {
        printf("Erro ao alocar memória para os vértices\n");
        free(g);
        return NULL; // Tratar o erro adequadamente de acordo com o seu caso
    }

    // Inicializar os vértices
    for (int i = 0; i < num_vertices; i++) {
        g->vertices[i] = malloc(sizeof(struct no));
        if (g->vertices[i] == NULL) {
            printf("Erro ao alocar memória para o vértice %d\n", i);
            for (int j = 0; j < i; j++) {
                free(g->vertices[j]);
            }
            free(g->vertices);
            free(g);
            return NULL; // Tratar o erro adequadamente de acordo com o seu caso
        }
        g->vertices[i]->id = i;
        g->vertices[i]->grau = 0;
        g->vertices[i]->aresta = NULL;
    }

    int num_arestas = (int) (num_vertices * densidade);

    // Gerando as arestas
    srand(time(NULL)); // Inicializa a semente para a função rand() com o tempo atual
    for (int i = 0; i < num_arestas; i++) {
        int v1 = rand() % num_vertices;
        int v2 = rand() % num_vertices;
        int peso_aleatorio = rand() % 100; // Gera um peso aleatório de 0 a 99

        // Se a aresta já existe, continue para a próxima iteração
        if (existe_aresta(g, v1, v2)) {
            continue;
        }

        adicionar_aresta(g, v1, v2, peso_aleatorio);
        if (tipo == 0) { // Se o grafo não for direcionado, adicione a aresta no sentido oposto também
            adicionar_aresta(g, v2, v1, peso_aleatorio);
        }
    }

    // Gerando o arquivo .txt
    FILE *fp = fopen(arquivo, "w");
    if (fp == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        for (int i = 0; i < num_vertices; i++) {
            free(g->vertices[i]);
        }
        free(g->vertices);
        free(g);
        return NULL; // Tratar o erro adequadamente de acordo com o seu caso
    }
    fprintf(fp, "%d %d %c %d\n", num_vertices, g->num_arestas, (tipo == 0 ? 'G' : 'D'), valorado);
    for (int i = 0; i < num_vertices; i++) {
        if (g->vertices[i]->aresta != NULL) {
            for (struct aresta *aresta = g->vertices[i]->aresta; aresta != NULL; aresta = aresta->proxima) {
                if (valorado) {
                    fprintf(fp, "%d %d %d\n", i, aresta->destino, aresta->peso);
                } else {
                    fprintf(fp, "%d %d\n", i, aresta->destino);
                }
            }
        }
    }
    fclose(fp);

    return g;
}

struct grafo *ler_grafo(struct grafo *g, char *arquivo) {
    FILE *fp;
    int num_vertices, num_arestas, tipo, valorado;
    int vi, vj, peso;

    fp = fopen(arquivo, "r");
    if (fp == NULL) {
        printf("Erro ao abrir o arquivo %s\n", arquivo);
        return NULL; // Deve retornar um ponteiro do tipo 'struct grafo *'
    }

    fscanf(fp, "%d %d %d %d\n", &num_vertices, &num_arestas, &tipo, &valorado);

    g->num_vertices = num_vertices;
    g->num_arestas = num_arestas;
    g->tipo = tipo;
    g->valorado = valorado;

    g->vertices = malloc(sizeof(struct no *) * num_vertices);
    if (g->vertices == NULL) {
        printf("Erro ao alocar memória para os vértices\n");
        return NULL;
    }

    for (int i = 0; i < num_vertices; i++) {
        g->vertices[i] = malloc(sizeof(struct no));
        g->vertices[i]->id = i; // Use '->' em vez de '.'
        g->vertices[i]->grau = 0;
        g->vertices[i]->aresta = NULL;
    }

    for (int i = 0; i < num_arestas; i++) {
        fscanf(fp, "%d %d %d\n", &vi, &vj, &peso);

        struct aresta *aresta = malloc(sizeof(struct aresta));
        if (aresta == NULL) {
            printf("Erro ao alocar memória para a aresta\n");
            return NULL;
        }

        aresta->origem = vi;
        aresta->destino = vj;
        aresta->peso = peso;

        if (g->tipo == 0) { // Se 'G' era uma constante, use o valor correspondente aqui
            adicionar_aresta(g, vi, vj, peso);
        } else {
            adicionar_arco(g, vi, vj, peso);
        }
    }

    fclose(fp);

    return g;
}

void criar_matriz_adjacencias(struct grafo *g) {
    int i, j;

    // Alocando memória para a matriz de adjacências
    g->matriz_adjacencias = malloc(sizeof(int *) * g->num_vertices);
    if (g->matriz_adjacencias == NULL) {
        printf("Erro ao alocar memória para a matriz de adjacências\n");
        return;
    }

    for (i = 0; i < g->num_vertices; i++) {
        g->matriz_adjacencias[i] = malloc(sizeof(int) * g->num_vertices);
        if (g->matriz_adjacencias[i] == NULL) {
            printf("Erro ao alocar memória para a matriz de adjacências\n");
            return;
        }

        for (j = 0; j < g->num_vertices; j++) {
            g->matriz_adjacencias[i][j] = 0;
        }
    }

    // Preenchendo a matriz de adjacências
    for (i = 0; i < g->num_vertices; i++) {
        for (j = 0; j < g->num_vertices; j++) {
            g->matriz_adjacencias[i][j] = 0;
        }
    }

    for (i = 0; i < g->num_vertices; i++) {
        struct no *vertice = g->vertices[i];
        for (struct aresta *aresta = vertice->aresta; aresta != NULL; aresta = aresta->proxima) {
            g->matriz_adjacencias[i][aresta->destino] = 1;
        }
    }

    // Imprimindo a matriz de adjacências
    for (i = 0; i < g->num_vertices; i++) {
        for (j = 0; j < g->num_vertices; j++) {
            printf("%d ", g->matriz_adjacencias[i][j]);
        }
        printf("\n");
    }
}

void criar_lista_adjacencias(struct grafo *g) {
    int i;

    for (i = 0; i < g->num_vertices; i++) {
        struct no *vertice = g->vertices[i]; 

        g->vertices[i]->lista_adjacencias = malloc(sizeof(struct aresta *) * g->num_vertices);
        if (g->vertices[i]->lista_adjacencias == NULL) {
            printf("Erro ao alocar memória para a lista de adjacências do vértice %d\n", i);
            return;
        }

        int j = 0;
        for (struct aresta *aresta = vertice->aresta; aresta != NULL; aresta = aresta->proxima) {
            g->vertices[i]->lista_adjacencias[j] = aresta;
            j++;
        }
    }

    // Imprimindo a lista de adjacências
    for (i = 0; i < g->num_vertices; i++) {
        struct no *vertice = g->vertices[i]; // Removendo o operador '&'

        printf("Vértice %d: ", vertice->id);

        for (int j = 0; j < g->vertices[i]->grau; j++) { // Usando 'grau' para controlar o loop
            printf("%d ", g->vertices[i]->lista_adjacencias[j]->destino);
        }

        printf("\n");
    }
}

void calcular_grau_vertices(struct grafo *g) {
    int i;

    for (i = 0; i < g->num_vertices; i++) {
        struct no *vertice = g->vertices[i]; // Modificação aqui

        int grau = 0;
        for (struct aresta *aresta = vertice->aresta; aresta != NULL; aresta = aresta->proxima) {
            grau++;
        }

        vertice->grau = grau;

        printf("Vértice %d: %d\n", vertice->id, vertice->grau);
    }
}

void imprimir_arvore_geradora_minima(struct grafo *agm) {
    printf("Árvore Geradora Mínima:\n");
    for (int i = 0; i < agm->num_vertices; i++) {
        if (agm->vertices[i]->pai != NULL) {
            printf("Aresta: %d - %d\n", agm->vertices[i]->pai->id, i);
        }
    }
}

struct grafo *arv_geradora_minima(struct grafo *g) {
    struct grafo *agm = malloc(sizeof(struct grafo));
    if (agm == NULL) {
        printf("Erro ao alocar memória para o grafo da árvore geradora mínima\n");
        return NULL;
    }

    agm->num_vertices = g->num_vertices;
    agm->num_arestas = 0;
    agm->tipo = g->tipo;
    agm->valorado = g->valorado;

    agm->vertices = malloc(sizeof(struct no *) * g->num_vertices);
    if (agm->vertices == NULL) {
        printf("Erro ao alocar memória para os vértices da árvore geradora mínima\n");
        return NULL;
    }

    for (int i = 0; i < g->num_vertices; i++) {
        agm->vertices[i] = malloc(sizeof(struct no));
        agm->vertices[i]->id = i;
        agm->vertices[i]->grau = 0;
        agm->vertices[i]->aresta = NULL;
        agm->vertices[i]->lista_adjacencias = NULL;
        agm->vertices[i]->pai = NULL; // Agora o campo pai é um ponteiro para o nó anterior
    }

    for (int i = 0; i < g->num_vertices; i++) {
        agm->vertices[i]->pai = NULL;
    }
    int *custos = malloc(sizeof(int) * g->num_vertices);
    if (custos == NULL) {
        printf("Erro ao alocar memória para os custos da árvore geradora mínima\n");
        return NULL;
    }

    for (int i = 0; i < g->num_vertices; i++) {
        custos[i] = INT_MAX;
    }

    int *visitados = malloc(sizeof(int) * g->num_vertices);
    if (visitados == NULL) {
        printf("Erro ao alocar memória para os visitados da árvore geradora mínima\n");
        return NULL;
    }

    for (int i = 0; i < g->num_vertices; i++) {
        visitados[i] = 0;
    }

    custos[0] = 0;

    for (int i = 0; i < g->num_vertices - 1; i++) {
        int u = -1;
        for (int j = 0; j < g->num_vertices; j++) {
            if (!visitados[j] && (u == -1 || custos[j] < custos[u])) {
                u = j;
            }
        }

        visitados[u] = 1;

        if (agm->vertices[u]->pai != NULL) {
            adicionar_aresta(agm, agm->vertices[u]->pai->id, u, custos[u]);
        }

        for (struct aresta *a = g->vertices[u]->aresta; a != NULL; a = a->proxima) {
            if (!visitados[a->destino] && a->peso < custos[a->destino]) {
                agm->vertices[a->destino]->pai = agm->vertices[u];
                custos[a->destino] = a->peso;
            }
        }
    }

    free(custos);
    free(visitados);
    imprimir_arvore_geradora_minima(agm);

    return agm;
}

void imprimir_caminho(struct grafo *g, int origem, int destino) {
    printf("%d ", destino);
    struct no *vertice = g->vertices[destino]->pai;
    while (vertice != NULL && vertice->id != origem) {
        printf("<-- %d ", vertice->id);
        vertice = vertice->pai;
    }
    if (vertice == NULL) {
        printf("\nCaminho não encontrado");
    } else {
        printf("<-- %d\n", origem);
    }
}

void caminho_mais_curto(struct grafo *g, int origem, int destino) {
    int num_vertices = g->num_vertices;

    // Inicializando os vértices

    for (int i = 0; i < num_vertices; i++) {
        g->vertices[i]->distancia = INT_MAX;
        g->vertices[i]->pai = NULL;
    }

    // Marcando o vértice de origem como visitado

    g->vertices[origem]->distancia = 0;

    // Fila de prioridade para os vértices não visitados

    struct heap *fila_prioridade = criar_heap(num_vertices);
    for (int i = 0; i < num_vertices; i++) {
        inserir_heap(fila_prioridade, g->vertices[i]);
    }

    // Percorrendo os vértices não visitados

    while (!heap_vazio(fila_prioridade)) {
        // Removendo o vértice com a menor distância

        struct no *vertice = remover_heap(fila_prioridade);

        // Se o vértice for o destino, imprime o caminho
        if (vertice->id == destino) {
            printf("Caminho mais curto entre %d e %d:\n", origem, destino);
            imprimir_caminho(g, origem, destino);
            return;
        }

        // Atualizando as distâncias dos vértices adjacentes
        for (struct aresta *aresta = vertice->aresta; aresta != NULL; aresta = aresta->proxima) {
            int novo_custo = vertice->distancia + aresta->peso;
            if (novo_custo < g->vertices[aresta->destino]->distancia) {
                g->vertices[aresta->destino]->distancia = novo_custo;
                g->vertices[aresta->destino]->pai = vertice;
                empurrar_heap(fila_prioridade, g->vertices[aresta->destino]);
            }
        }
    }
}

void busca_largura(struct grafo *g, int origem) {
    int num_vertices = g->num_vertices;

    // Inicializando os vértices

    for (int i = 0; i < num_vertices; i++) {
        g->vertices[i]->visitado = 0;
    }

    // Fila de prioridades para os vértices não visitados

    struct fila *fila = criar_fila();
    enfileirar(fila, g->vertices[origem]);

    // Percorrendo os vértices

    printf("\nBusca largura: ");

    while (!fila_vazia(fila)) {
        // Removendo o vértice da frente da fila

        struct no *vertice = desenfileirar(fila);

        // Marcando o vértice como visitado

        vertice->visitado = 1;

        // Imprimindo o vértice

        printf("%d ", vertice->id);

        // Adicionando os vértices adjacentes à fila

        for (struct aresta *aresta = vertice->aresta; aresta != NULL; aresta = aresta->proxima) {
            if (!g->vertices[aresta->destino]->visitado) {
                enfileirar(fila, g->vertices[aresta->destino]);
            }
        }
    }

    // Liberando a memória da fila

    liberar_fila(fila);
}

void busca_profundidade(struct grafo *g, int origem) {
    int num_vertices = g->num_vertices;

    // Inicializando os vértices

    for (int i = 0; i < num_vertices; i++) {
        g->vertices[i]->visitado = 0;
    }

    // Pilha de vértices

    struct pilha *pilha = criar_pilha();
    empilhar(pilha, g->vertices[origem]);

    // Percorrendo os vértices

    printf("\nBusca profundidade: ");

    while (!pilha_vazia(pilha)) {
        // Removendo o vértice do topo da pilha

        struct no *vertice = desempilhar(pilha);

        // Marcando o vértice como visitado

        vertice->visitado = 1;

        // Imprimindo o vértice

        printf("%d ", vertice->id);

        // Atualizando os vértices adjacentes

        for (struct aresta *aresta = vertice->aresta; aresta != NULL; aresta = aresta->proxima) {
            if (!g->vertices[aresta->destino]->visitado) {
                empilhar(pilha, g->vertices[aresta->destino]);
            }
        }
    }

    // Liberando a memória da pilha

    liberar_pilha(pilha);
}



int main() {
    int opcao;
    struct grafo *g = malloc(sizeof(struct grafo));
    char nome_arquivo[100];

    do {
        // Imprimindo o menu
        printf("Escolha uma opção:\n");
        printf("1 - Criar um grafo aleatório\n");
        printf("2 - Ler um grafo existente\n");
        printf("3 - Sair\n");
        scanf("%d", &opcao);

        if (opcao == 3) {
            break;
        }
        // Criando um grafo aleatório
        else if (opcao == 1) {
        printf("Digite o número de vértices: ");
        scanf("%d", &g->num_vertices);
        printf("Digite a densidade: ");
        scanf("%lf", &g->densidade);
        int tipo, valorado;
        printf("Digite o tipo (0 - Não dirigido, 1 - Dirigido): ");
        scanf("%d", &tipo);
        printf("Digite se é valorado (0 - Não valorado, 1 - Valorado): ");
        scanf("%d", &valorado);
        printf("Digite o nome do arquivo para salvar o grafo: ");
        scanf("%s", nome_arquivo);
        g = criar_grafo_aleatorio(g->num_vertices, g->densidade, nome_arquivo, tipo, valorado);
        }

        // Lendo um grafo existente
        else if (opcao == 2) {
        printf("Digite o nome do arquivo para carregar o grafo: ");
        scanf("%s", nome_arquivo);
        g = ler_grafo(g, nome_arquivo);
        }

        else{
            printf("Opção inválida.\n");
            break;
        }

        printf("Lendo grafo '%s'\n", nome_arquivo);
        printf("Escolha a ação que deseja realizar com este grafo:\n");
        printf("1- Criar matriz adjacencias;\n");
        printf("2- Criar lista de adjacencias;\n");
        printf("3- Calcular e mostrar o grau de cada vertice;\n");
        printf("4- Encontrar a arvore geradora minima;");
        printf("5- Encontrar um caminho mais curto usando Djiskara;\n");
        printf("6- Realizar busca em largura;\n");
        printf("7- Realizar busca em profundidade;\n");
        printf("0- Sair.\n");

        int acao;
        scanf("%d", &acao);
        int origem, destino;
        while (acao != 0) {
            switch (acao) {
                case 0:
                    // Sair
                    break;
                case 1:
                    printf("\nMatriz de Adjacencias:\n");
                    criar_matriz_adjacencias(g);//funciona para grafos sem peso
                    break;
                case 2:
                    criar_lista_adjacencias(g);//não funciona
                    break;
                case 3:
                    calcular_grau_vertices(g);//funciona para grafos sem peso
                    break;
                case 4:
                    arv_geradora_minima(g);//?
                    break;
                case 5:
                    printf("Defina a origem: ");
                    scanf("%d", &origem);
                    printf("Defina o destino: ");
                    scanf("%d", &destino);
                    caminho_mais_curto(g, origem, destino);
                    break;
                case 6:
                    busca_largura(g, 0); // Supondo que a origem é o vértice 0
                    break;
                case 7:
                    busca_profundidade(g, 0); // Supondo que a origem é o vértice 0; gera loop no ultimo numero
                    break;
                default:
                    printf("Opção inválida.\n");
            }
            printf("Escolha a ação que deseja realizar com este grafo:\n");
            printf("1- Criar matriz adjacencias;\n");
            printf("2- Criar lista de adjacencias;\n");
            printf("3- Calcular e mostrar o grau de cada vertice;\n");
            printf("4- Encontrar a arvore geradora minima;");
            printf("5- Encontrar um caminho mais curto usando Djiskara;\n");
            printf("6- Realizar busca em largura;\n");
            printf("7- Realizar busca em profundidade;\n");
            printf("0- Sair.\n");
            scanf("%d", &acao);
        }
    } while (opcao != 3);

    return 0;
}