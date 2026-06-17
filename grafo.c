#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

// ==========================================
// 1. ESTRUTURA DO GRAFO (UCTP + ESG/MTZ)
// ==========================================
typedef struct {
    int num_vertices;
    bool **matriz_adj;  // Hard Constraints (Conflito de Horário)
    int **matriz_dist;  // Soft Constraints (Distância Física)
    int *grau;
    int *cor;           // Slot de tempo (Variável u_i do MTZ)
    int *id_professor;  // Rastreio para cálculo de rota
} Grafo;

Grafo* criar_grafo(int num_vertices) {
    Grafo *g = (Grafo*) malloc(sizeof(Grafo));
    g->num_vertices = num_vertices;
    g->grau = (int*) calloc(num_vertices, sizeof(int));
    g->cor = (int*) malloc(num_vertices * sizeof(int));
    g->id_professor = (int*) malloc(num_vertices * sizeof(int));
    g->matriz_adj = (bool**) malloc(num_vertices * sizeof(bool*));
    g->matriz_dist = (int**) malloc(num_vertices * sizeof(int*)); 
    
    for (int i = 0; i < num_vertices; i++) {
        g->matriz_adj[i] = (bool*) calloc(num_vertices, sizeof(bool));
        g->matriz_dist[i] = (int*) calloc(num_vertices, sizeof(int));
        g->cor[i] = -1;
        g->id_professor[i] = -1;
    }
    return g;
}

void adicionar_conflito(Grafo *g, int u, int v) {
    if (!g->matriz_adj[u][v]) {
        g->matriz_adj[u][v] = true;
        g->matriz_adj[v][u] = true;
        g->grau[u]++;
        g->grau[v]++;
    }
}

void adicionar_distancia(Grafo *g, int u, int v, int distancia) {
    g->matriz_dist[u][v] = distancia;
    g->matriz_dist[v][u] = distancia;
}

void liberar_grafo(Grafo *g) {
    for (int i = 0; i < g->num_vertices; i++) {
        free(g->matriz_adj[i]);
        free(g->matriz_dist[i]);
    }
    free(g->matriz_adj);
    free(g->matriz_dist);
    free(g->grau);
    free(g->cor);
    free(g->id_professor);
    free(g);
}

// ==========================================
// 2. HEURÍSTICA DSATUR
// ==========================================
int calcular_saturacao(Grafo *g, int v) {
    bool cor_presente[g->num_vertices];
    for (int i = 0; i < g->num_vertices; i++) cor_presente[i] = false;
    
    int saturacao = 0;
    for (int i = 0; i < g->num_vertices; i++) {
        if (g->matriz_adj[v][i] && g->cor[i] != -1) {
            if (!cor_presente[g->cor[i]]) {
                cor_presente[g->cor[i]] = true;
                saturacao++;
            }
        }
    }
    return saturacao;
}

void colorir_dsatur(Grafo *g) {
    int vertices_coloridos = 0;
    while (vertices_coloridos < g->num_vertices) {
        int v_escolhido = -1, max_saturacao = -1, max_grau = -1;
        
        for (int i = 0; i < g->num_vertices; i++) {
            if (g->cor[i] == -1) {
                int sat_atual = calcular_saturacao(g, i);
                if (sat_atual > max_saturacao || (sat_atual == max_saturacao && g->grau[i] > max_grau)) {
                    max_saturacao = sat_atual;
                    max_grau = g->grau[i];
                    v_escolhido = i;
                }
            }
        }
        
        bool cor_proibida[g->num_vertices];
        for (int i = 0; i < g->num_vertices; i++) cor_proibida[i] = false;
        
        for (int i = 0; i < g->num_vertices; i++) {
            if (g->matriz_adj[v_escolhido][i] && g->cor[i] != -1) {
                cor_proibida[g->cor[i]] = true;
            }
        }
        
        int cor_escolhida = 0;
        while (cor_proibida[cor_escolhida]) cor_escolhida++;
        
        g->cor[v_escolhido] = cor_escolhida;
        vertices_coloridos++;
    }
}

// ==========================================
// 3. EXPORTAÇÃO PARA GRAPHVIZ
// ==========================================
void exportar_graphviz(Grafo *g, const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL) return;

    fprintf(arquivo, "graph UCTP {\n");
    fprintf(arquivo, "    node [style=filled, fontname=\"Arial\"];\n\n");

    const char* paleta[] = {"#38bdf8", "#10b981", "#fbbf24", "#ef4444", "#a855f7"};

    for (int i = 0; i < g->num_vertices; i++) {
        int cor_idx = g->cor[i] % 5;
        fprintf(arquivo, "    %d [label=\"Turma %d\\nSlot %d\\nProf %d\", fillcolor=\"%s\"];\n", 
                i, i, g->cor[i], g->id_professor[i], paleta[cor_idx]);
    }

    fprintf(arquivo, "\n");
    for (int i = 0; i < g->num_vertices; i++) {
        for (int j = i + 1; j < g->num_vertices; j++) {
            if (g->matriz_adj[i][j]) {
                fprintf(arquivo, "    %d -- %d [color=\"#94a3b8\"];\n", i, j);
            }
        }
    }
    fprintf(arquivo, "}\n");
    fclose(arquivo);
}

// ==========================================
// 4. AVALIAÇÃO MTZ E META-HEURÍSTICA
// ==========================================
int calcular_custo_total_esg(Grafo *g, int num_professores) {
    int custo_total = 0;
    int penalidade_ociosidade = 100; // Penalidade por cada slot vago (buraco na grade)

    // Encontra o número máximo de slots usados para otimizar o laço temporal
    int max_slot = 0;
    for (int i = 0; i < g->num_vertices; i++) {
        if (g->cor[i] > max_slot) max_slot = g->cor[i];
    }

    for (int p = 0; p < num_professores; p++) {
        int sala_anterior = -1;
        int slot_anterior = -1;

        // Ordem cronológica atua como a variável u_i do MTZ, evitando subciclos
        for (int slot = 0; slot <= max_slot; slot++) {
            for (int v = 0; v < g->num_vertices; v++) {
                if (g->id_professor[v] == p && g->cor[v] == slot) {
                    if (sala_anterior != -1) {
                        // 1. Soft Constraint: Distância Geográfica
                        custo_total += g->matriz_dist[sala_anterior][v];
                        
                        // 2. Soft Constraint: Minimização de Janelas Ociosas
                        int janelas_vazias = slot - slot_anterior - 1;
                        if (janelas_vazias > 0) {
                            custo_total += (janelas_vazias * penalidade_ociosidade);
                        }
                    }
                    sala_anterior = v;
                    slot_anterior = slot;
                }
            }
        }
    }
    return custo_total;
}

void otimizar_rotas_metaheuristica(Grafo *g, int num_professores) {
    int custo_atual = calcular_custo_total_esg(g, num_professores);
    int melhor_custo = custo_atual;
    printf("\n[Meta-heuristica] Iniciando Simulated Annealing. Custo inicial: %d\n", custo_atual);
    
    srand(time(NULL)); 
    
    // Parâmetros do Simulated Annealing
    double temperatura = 1000.0;
    double taxa_resfriamento = 0.995;
    double temp_minima = 0.001;
    int iteracoes_na_temperatura = 100;
    
    // Vetor para armazenar a melhor solução global encontrada
    int *melhor_cor = (int*) malloc(g->num_vertices * sizeof(int));
    for (int i = 0; i < g->num_vertices; i++) melhor_cor[i] = g->cor[i];

    while (temperatura > temp_minima) {
        for (int iter = 0; iter < iteracoes_na_temperatura; iter++) {
            int v_random = rand() % g->num_vertices;
            int cor_original = g->cor[v_random];
            
            // Descobre max_cor dinamicamente para tentar cores viáveis ou apenas UMA nova cor
            int max_cor = 0;
            for (int i = 0; i < g->num_vertices; i++) {
                if (g->cor[i] > max_cor) max_cor = g->cor[i];
            }
            int nova_cor = rand() % (max_cor + 2); 
            
            if (nova_cor == cor_original) continue;

            // 1. Verifica Violação de Hard Constraints (Conflitos)
            bool conflito = false;
            for (int i = 0; i < g->num_vertices; i++) {
                if (g->matriz_adj[v_random][i] && g->cor[i] == nova_cor) {
                    conflito = true;
                    break;
                }
            }

            // 2. Avaliação de Impacto e Critério de Metropolis
            if (!conflito) {
                g->cor[v_random] = nova_cor;
                int novo_custo = calcular_custo_total_esg(g, num_professores);
                int delta = novo_custo - custo_atual;

                // Aceita se for melhor (delta < 0) OU pela probabilidade termodinâmica
                if (delta < 0 || ((double)rand() / RAND_MAX) < exp(-delta / temperatura)) {
                    custo_atual = novo_custo; 
                    
                    // Atualiza a melhor solução global
                    if (custo_atual < melhor_custo) {
                        melhor_custo = custo_atual;
                        for (int i = 0; i < g->num_vertices; i++) melhor_cor[i] = g->cor[i];
                    }
                } else {
                    // Desfaz a troca se rejeitada
                    g->cor[v_random] = cor_original;
                }
            }
        }
        // Reduz a temperatura
        temperatura *= taxa_resfriamento;
    }
    
    // Restaura o grafo para a melhor configuração global encontrada no processo
    for (int i = 0; i < g->num_vertices; i++) g->cor[i] = melhor_cor[i];
    free(melhor_cor);
    
    printf("[Meta-heuristica] Otimizacao concluida. Custo final reduzido: %d\n\n", melhor_custo);
}

// ==========================================
// 4.5. AUDITORIA E MÉTRICAS (SLIDE 9)
// ==========================================

// Verifica a Conformidade Rígida (Violações de Conflito)
int auditar_violacoes(Grafo *g) {
    int violacoes = 0;
    for (int i = 0; i < g->num_vertices; i++) {
        for (int j = i + 1; j < g->num_vertices; j++) {
            // Verifica conflito de grade OU conflito de professor no mesmo slot
            if ((g->matriz_adj[i][j] || g->id_professor[i] == g->id_professor[j]) && g->cor[i] == g->cor[j]) {
                violacoes++;
            }
        }
    }
    return violacoes;
}

// Calcula a Eficiência Cromática (Total de Slots/Cores usados)
int contar_slots_utilizados(Grafo *g) {
    int max_cor = -1;
    for (int i = 0; i < g->num_vertices; i++) {
        if (g->cor[i] > max_cor) {
            max_cor = g->cor[i];
        }
    }
    return max_cor + 1; // +1 porque as cores começam em 0
}

Grafo* ler_grafo_arquivo(const char *nome_arquivo, int *num_professores_out) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo de instancia: %s\n", nome_arquivo);
        exit(1);
    }

    int num_vertices, num_professores;
    fscanf(arquivo, "%d %d", &num_vertices, &num_professores);
    *num_professores_out = num_professores;

    Grafo *g = criar_grafo(num_vertices);

    // 1. Ler o mapeamento de Turma -> Professor
    for (int i = 0; i < num_vertices; i++) {
        fscanf(arquivo, "%d", &g->id_professor[i]);
    }

    // Aplicação Automática da Invariância de Alocação de Pessoal
    for (int i = 0; i < num_vertices; i++) {
        for (int j = i + 1; j < num_vertices; j++) {
            if (g->id_professor[i] == g->id_professor[j]) {
                adicionar_conflito(g, i, j);
            }
        }
    }

    // 2. Ler Hard Constraints Curriculares
    int num_conflitos;
    fscanf(arquivo, "%d", &num_conflitos);
    for (int i = 0; i < num_conflitos; i++) {
        int u, v;
        fscanf(arquivo, "%d %d", &u, &v);
        adicionar_conflito(g, u, v);
    }

    // 3. Ler Soft Constraints Geográficas
    int num_distancias;
    fscanf(arquivo, "%d", &num_distancias);
    for (int i = 0; i < num_distancias; i++) {
        int u, v, dist;
        fscanf(arquivo, "%d %d %d", &u, &v, &dist);
        adicionar_distancia(g, u, v, dist);
    }

    fclose(arquivo);
    return g;
}

// ==========================================
// 5. FUNÇÃO PRINCIPAL
// ==========================================
int main() {
    int total_professores;
    
    // 1. CARREGAMENTO DO CASO DE TESTE (AMPLIAÇÃO)
    // O arquivo "instancia.txt" deve estar na mesma pasta do executável
    Grafo *meu_grafo = ler_grafo_arquivo("instancia.txt", &total_professores);
    int V = meu_grafo->num_vertices;
    
    printf("--- 1. SOLUCAO INICIAL (DSATUR) ---\n");
    colorir_dsatur(meu_grafo);
    for (int i = 0; i < V; i++) {
        printf("Turma %d (Prof %d) alocada no Slot: %d\n", i, meu_grafo->id_professor[i], meu_grafo->cor[i]);
    }
    
    // Registra o custo ANTES da meta-heurística para o relatório
    int custo_inicial = calcular_custo_total_esg(meu_grafo, total_professores);
    
    // 2. OTIMIZAÇÃO COM SIMULATED ANNEALING
    clock_t inicio = clock();
    otimizar_rotas_metaheuristica(meu_grafo, total_professores);
    clock_t fim = clock();
    
    double tempo_gasto = (double)(fim - inicio) / CLOCKS_PER_SEC * 1000.0;
    
    printf("\n--- 2. SOLUCAO OTIMIZADA (ESG) ---\n");
    for (int i = 0; i < V; i++) {
        printf("Turma %d (Prof %d) alocada no Slot: %d\n", i, meu_grafo->id_professor[i], meu_grafo->cor[i]);
    }
    
    printf("\n[Benchmark] Tempo de otimizacao: %.2f ms\n", tempo_gasto);

    // 3. RELATÓRIO FINAL DE MÉTRICAS
    int slots_usados = contar_slots_utilizados(meu_grafo);
    int violacoes = auditar_violacoes(meu_grafo);
    int custo_final = calcular_custo_total_esg(meu_grafo, total_professores);
    
    double reducao_ociosidade = 0.0;
    if (custo_inicial > 0) {
        reducao_ociosidade = ((double)(custo_inicial - custo_final) / custo_inicial) * 100.0;
    }

    printf("\n======================================================\n");
    printf("   RELATORIO DE METRICAS (VALIDACAO UCTP/ESG) \n");
    printf("======================================================\n");
    printf(" 1. Eficiencia Cromatica (Slots) : %d\n", slots_usados);
    printf(" 2. Conformidade Rigida          : %d violacoes\n", violacoes);
    printf(" 3. Otimizacao Soft (Distancia)  : Reducao de %.1f%%\n", reducao_ociosidade);
    printf(" 4. Desempenho Temporal          : %.2f ms\n", tempo_gasto);
    printf("======================================================\n");
    
    exportar_graphviz(meu_grafo, "grafo_esg.dot");
    liberar_grafo(meu_grafo);
    return 0;
}