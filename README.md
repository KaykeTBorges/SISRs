# SISRs

## Requisitos

- `g++` com suporte a C++20
- `make`
- Python 3.10 ou superior, somente para executar a bateria automatizada

Execute os comandos a partir da pasta raiz do projeto, onde está este README.

## Organização

- `src/`: arquivos de implementação C++ (`.cpp`)
- `include/`: headers C++ (`.h`)
- `instances/`: instâncias `.vrp` e soluções de referência `.sol`
- `benchmark/`: script de automação e CSVs de resultados

O `Makefile` compila o executável `leitor` na raiz do projeto.

## Compilar

```bash
make
```

Isso gera o executável `leitor`. Para recompilar do zero:

```bash
make clean
make
```

## Executar normalmente

Passe o caminho de um arquivo `.vrp` como argumento:

```bash
./leitor instances/X-n101-k25.vrp
```

O programa imprime a solução final, o custo, o número de veículos, o tempo da busca local em segundos e as rotas. Sem uma seed explícita, o gerador usa sua seed padrão.

## Executar uma instância com seed específica

Informe primeiro o arquivo da instância e depois a seed inteira:

```bash
./leitor instances/X-n101-k25.vrp 7
```

Para executar outra combinação de instância e seed:

```bash
./leitor instances/X-n176-k26.vrp 3
```

A seed afeta as escolhas aleatórias do algoritmo. Os arquivos `.vrp` das instâncias ficam em `instances/`, na raiz do projeto.

## Executar a bateria automatizada

Compile o programa e rode o script:

```bash
make
python3 benchmark/benchmark.py
```

O script executa, em sequência, as nove instâncias configuradas, dez vezes cada. Para cada instância, usa as seeds de 1 a 10. Ao terminar, registra os dados em três arquivos:

```text
benchmark/resultados.csv
instance,run,seed,cost,vehicles,time

benchmark/rotas.csv
instance,run,seed,tour,route,cost

benchmark/medias_por_instancia.csv
instance,runs,mean_cost,mean_vehicles,mean_time
```

Em `benchmark/rotas.csv`, cada linha representa uma rota de um veículo em uma execução; `route` contém os nós na ordem impressa pelo C++ e `cost` é o custo daquela rota. `benchmark/medias_por_instancia.csv` resume a média de custo final, número de veículos e tempo das dez execuções de cada instância. As médias são gravadas e mostradas no terminal somente após a conclusão das 90 execuções.

Uma nova bateria substitui os três CSVs dentro de `benchmark/`. Se a execução for interrompida, os arquivos de resultados e rotas conterão somente as execuções concluídas, e o arquivo de médias ficará sem linhas de dados. O campo `time` é o tempo em segundos impresso pelo C++ para a busca local (`localSearch`); não inclui o carregamento do arquivo nem a criação da solução inicial. O executável precisa estar compilado antes de iniciar o script.

## Resumo dos comandos

```bash
make                                  # compila o executável
./leitor instances/X-n101-k25.vrp     # executa sem seed explícita
./leitor instances/X-n101-k25.vrp 7   # executa com seed 7
python3 benchmark/benchmark.py        # roda 90 vezes; salva os CSVs
make clean                            # remove o executável compilado
```
