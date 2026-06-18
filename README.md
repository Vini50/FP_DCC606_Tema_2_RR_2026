# FP_DCC606_Tema_2_RR_2026

**Coloração de Vértices em Grafos para Otimização de Cronogramas Institucionais Complexos: Uma Abordagem ESG-Driven**

Este repositório contém o código-fonte, instâncias de teste e o relatório técnico do Projeto Final da disciplina DCC606. O projeto aborda o **University Course Timetabling Problem (UCTP)**, utilizando teoria dos grafos para gerar grades horárias otimizadas, livres de conflitos e alinhadas aos critérios socioambientais (ESG).

## 👥 Autores
* **Helian Vinicius Filinto da Silva**
* **Kaylon Gutierre Peres Gonçalves**

## 🎯 Resumo do Projeto
O sistema recebe uma matriz curricular, a disponibilidade de docentes e as restrições logísticas do campus para gerar um cronograma validado. O desenvolvimento foi estritamente focado no gerenciamento de memória em baixo nível (Linguagem C estruturada), evitando contêineres abstratos para garantir o mínimo de latência e estabilidade em tempo polinomial.

A solução foi estruturada em duas fases algorítmicas:
1. **Heurística Construtiva (DSATUR):** Responsável por gerar uma solução inicial viável, garantindo o cumprimento de todas as restrições rígidas (*Hard Constraints*), com 0% de sobreposição de aulas ou professores alocados simultaneamente.
2. **Meta-heurística Estocástica (Simulated Annealing):** Responsável pelo refinamento do grafo, minimizando as restrições flexíveis (*Soft Constraints*). Esta fase otimiza a grade sob a ótica ESG, reduzindo drasticamente as distâncias geográficas percorridas e eliminando janelas ociosas (*idle times*) na jornada dos professores.

## ⚙️ Pré-requisitos
* **Compilador C:** GCC ou qualquer compilador compatível com a linguagem C.
* **GraphViz (Opcional):** Necessário apenas se desejar renderizar a topologia visual do grafo de saída (`grafo_esg.dot`).

## 🚀 Como Compilar e Executar

1. **Baixe o código:** Faça o download dos arquivos deste repositório para o seu computador. Certifique-se de que o arquivo fonte (`codigo.c`) e o arquivo de instância de teste (`instancia.txt`) estejam salvos na mesma pasta.
2. **Abra o terminal:** Navegue pelo terminal (ou Prompt de Comando) até o diretório onde os arquivos foram extraídos.
3. **Compile o código em C:** Digite o comando abaixo para compilar. 
gcc grafo.c -o grafo.exe
.\grafo.exe       

📂 Formato do Arquivo de Instância (.txt)

O algoritmo realiza a leitura das malhas curriculares a partir de arquivos de texto (ex: instancia.txt), que devem estar rigorosamente estruturados da seguinte forma:
Plaintext

<Qtd_Turmas> <Qtd_Professores>
<ID_Prof_Turma_0> <ID_Prof_Turma_1> ... <ID_Prof_Turma_N>
<Qtd_Conflitos_Obrigatorios>
<Turma_A> <Turma_B>
...
<Qtd_Distancias_Prédios>
<Turma_X> <Turma_Y> <Distancia_em_Metros>
...

📊 Saídas e Métricas

Ao final da execução, o terminal exibirá um Relatório de Métricas contendo:

    Eficiência Cromática: Número final de slots (cores) utilizados na grade institucional.

    Conformidade Rígida: Quantidade de violações de grade (obrigatoriamente 0 para soluções válidas).

    Otimização Soft (Distância/Ociosidade): Redução percentual das penalidades logísticas e de tempo ocioso atingida pela meta-heurística.

    Desempenho Temporal: Tempo total de convergência dos algoritmos mensurado em milissegundos (ms).
