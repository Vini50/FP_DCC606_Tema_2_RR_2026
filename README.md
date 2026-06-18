# FP_DCC606_Tema_2_RR_2026

**Coloração de Vértices em Grafos para Otimização de Cronogramas Institucionais Complexos: Uma Abordagem ESG-Driven**

Repositório do Projeto Final da disciplina **DCC606 – Análise de Algoritmos (UFRR, 2026)**. O projeto aborda o *University Course Timetabling Problem* (UCTP), modelando-o como **coloração de vértices em grafos** para gerar grades horárias livres de conflitos e alinhadas a critérios socioambientais (ESG).

---

## 👥 Autores

- Helian Vinicius Filinto da Silva
- Kaylon Gutierre Peres Gonçalves

---

## 🎯 Resumo do Projeto

O sistema recebe uma matriz curricular, a disponibilidade de docentes e as restrições logísticas do campus e gera um cronograma validado. O desenvolvimento foi focado no gerenciamento de memória em baixo nível (C estruturada), evitando contêineres abstratos para garantir baixa latência e estabilidade em tempo polinomial.

A solução é estruturada em duas fases:

1. **Heurística Construtiva (DSATUR):** gera uma solução inicial viável, garantindo o cumprimento de todas as restrições rígidas (*hard constraints*), com 0% de sobreposição de aulas ou de professores no mesmo horário.
2. **Meta-heurística Estocástica (Simulated Annealing):** refina a solução minimizando as restrições flexíveis (*soft constraints*), reduzindo distâncias geográficas percorridas e eliminando janelas ociosas (*idle times*) na jornada dos docentes.

---

## 📁 Estrutura do Repositório

| Arquivo | Descrição |
|---|---|
| `grafo.c` | Código-fonte principal (estruturas, DSATUR, Simulated Annealing, auditoria e exportação). |
| `instancia.txt` | Caso de teste lido pelo programa (15 turmas / 4 professores). |
| `grafo_esg.dot` | Visualização **gerada automaticamente** ao executar o programa. |
| `grafo_uctp.dot` | Visualização auxiliar do grafo de conflitos. |
| `relatorio.pdf` | Relatório técnico (formato IEEE). |

---

## ⚙️ Pré-requisitos

- **Compilador C:** GCC (Linux/macOS) ou MinGW/GCC (Windows). Qualquer compilador C compatível com C99 serve.
- **GraphViz** *(opcional)*: necessário apenas para renderizar a imagem do grafo a partir dos arquivos `.dot`.

> ⚠️ **Importante:** o programa usa a função `exp()` da biblioteca matemática (`math.h`). Por isso, **é obrigatório compilar com a flag `-lm`** no Linux e no macOS. Sem ela, o `gcc` retorna o erro `undefined reference to 'exp'`.

---

## 🚀 Como Compilar e Executar

> O programa lê **`instancia.txt`** automaticamente (nome fixo no código). Mantenha esse arquivo **na mesma pasta** do executável.

### 🐧 Linux / 🍎 macOS

```bash
# 1. Compilar (a flag -lm é obrigatória)
gcc grafo.c -o grafo -lm

# 2. Executar
./grafo
```

### 🪟 Windows (MinGW / GCC)

```powershell
# 1. Compilar
gcc grafo.c -o grafo.exe -lm

# 2. Executar
.\grafo.exe
```

### 🖼️ (Opcional) Gerar a imagem do grafo com o GraphViz

Após executar o programa, o arquivo `grafo_esg.dot` é (re)gerado. Para convertê-lo em imagem:

```bash
dot -Tpng grafo_esg.dot -o grafo_esg.png
```

---

## 📂 Formato do Arquivo de Instância (`.txt`)

O programa lê a malha curricular a partir de um arquivo de texto estruturado **exatamente** na ordem abaixo:

```text
<Qtd_Turmas> <Qtd_Professores>
<ID_Prof_Turma_0> <ID_Prof_Turma_1> ... <ID_Prof_Turma_N-1>
<Qtd_Conflitos_Curriculares>
<Turma_A> <Turma_B>      # uma linha por conflito
...
<Qtd_Distancias>
<Turma_X> <Turma_Y> <Distancia>   # uma linha por distância (soft constraint)
...
```

**Regras de leitura:**

- As turmas são indexadas a partir de **0**.
- A segunda linha mapeia cada turma ao seu professor; turmas com o mesmo professor recebem **automaticamente** um conflito rígido (Invariância de Alocação de Pessoal).
- Os conflitos curriculares são arestas adicionais do grafo (mesmo período/semestre).
- As distâncias representam o custo de deslocamento entre as salas das duas turmas (usado na otimização ESG).

### Exemplo comentado (`instancia.txt` deste repositório)

```text
15 4                  # 15 turmas, 4 professores
0 0 0 0 1 1 1 2 2 2 2 3 3 3 3   # professor de cada turma (turma 0..14)
12                    # 12 conflitos curriculares a seguir
0 1
1 2
2 3
4 5
5 6
7 8
8 9
9 10
11 12
12 13
0 4
7 11
8                     # 8 distâncias a seguir
0 1 500
1 2 300
4 5 800
5 6 200
7 8 150
8 9 400
11 12 600
12 13 100
```

---

## 📊 Saídas e Métricas

Durante a execução, o terminal exibe a solução inicial (DSATUR), a solução otimizada (Simulated Annealing) e, ao final, um **Relatório de Métricas**:

```text
======================================================
   RELATORIO DE METRICAS (VALIDACAO UCTP/ESG)
======================================================
 1. Eficiencia Cromatica (Slots) : 14
 2. Conformidade Rigida          : 0 violacoes
 3. Otimizacao Soft (Distancia)  : Reducao de 92.7%
 4. Desempenho Temporal          : 323.23 ms
======================================================
```

| Métrica | Significado |
|---|---|
| **Eficiência Cromática** | Número final de slots (cores) usados na grade. |
| **Conformidade Rígida** | Quantidade de violações de horário — deve ser **0** para uma solução válida. |
| **Otimização Soft** | Redução percentual das penalidades de distância e ociosidade obtida pela meta-heurística. |
| **Desempenho Temporal** | Tempo de convergência dos algoritmos, em milissegundos. |

> **Observação:** o Simulated Annealing é estocástico (usa `srand(time(NULL))`), portanto o custo final, o tempo e a imagem do grafo podem variar levemente entre execuções. A **conformidade rígida**, no entanto, permanece sempre em 0 violações.

---

## 📌 Repositório

<https://github.com/Vini50/FP_DCC606_Tema_2_RR_2026>
