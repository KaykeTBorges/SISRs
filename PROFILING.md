# Guia de Profiling de Desempenho (SISRs)

Este projeto possui rotinas matematicamente pesadas (especialmente a fase de `recreate`). Para garantir que estamos atacando os gargalos corretos (como otimizar o `BLINK_RATE`, loops em `evaluatePosition` e chamadas de RNG), é fundamental usar ferramentas de profiling em vez de depender de achismos.

Abaixo estão os passos detalhados de como usar o **perf**, a principal e mais precisa ferramenta para Linux.

## 1. Liberação do Kernel para uso do `perf`

Por segurança, o kernel do Linux bloqueia o acesso a contadores de hardware e amostragem de CPU por usuários comuns. Para que o `perf` funcione corretamente, você precisa abaixar o nível de "paranoia" do kernel.

Execute o comando abaixo (exige senha de sudo):

```bash
sudo sysctl kernel.perf_event_paranoid=-1
```

> **Nota:** Essa alteração é temporária e será revertida ao reiniciar o computador. Se quiser torná-la permanente (não recomendado para ambientes de produção web, mas OK para desktops de desenvolvimento), edite o arquivo `/etc/sysctl.conf` e adicione a linha `kernel.perf_event_paranoid = -1`.

## 2. Preparação do código (Flags de Compilação)

Para o profiling fazer sentido, o compilador precisa inserir **símbolos de depuração**, mas **mantendo as otimizações** (senão você estaria perfilando código lento não otimizado, o que gera falsos gargalos).

As flags `-O3`, `-march=native` e `-flto` devem ser mantidas, e a flag `-g` (debug symbols) deve ser adicionada. 

O `Makefile` padrão do projeto pode não conter a flag `-g`. Você pode compilar manualmente com o seguinte comando:

```bash
g++ -std=c++20 -O3 -march=native -flto -g -Iinclude -o leitor src/*.cpp
```

## 3. Coletando os dados (Record)

Para gravar onde a CPU passa a maior parte do tempo, execute o seu binário através do `perf record`. É recomendável usar uma `seed` fixa e uma instância específica para garantir a reprodutibilidade.

A flag `-g` no `perf record` diz para ele gravar as _call trees_ (quem chamou quem), o que é essencial.

```bash
perf record -g ./leitor instances/X-n101-k25.vrp 42
```

O programa rodará normalmente. Pode ficar ligeiramente mais lento, mas nada absurdo. Quando ele terminar, você verá uma mensagem dizendo que um arquivo `perf.data` foi gerado.

## 4. Analisando os dados (Report)

Com o `perf.data` gerado, você tem várias formas de visualizar o resultado.

### A) Visão interativa (Recomendada)
Apenas digite:
```bash
perf report
```
Isso abrirá uma interface no próprio terminal. 
- Use as **setas para cima/baixo** para navegar;
- Aperte **Enter** em cima de uma função para ver mais opções (como dar *Annotate* para ver qual linha de código em C++ dentro da função está custando mais);
- Aperte **Esc** ou **q** para sair ou voltar.

### B) Visão em texto corrido (Boa para copiar/colar)
Se quiser imprimir o relatório diretamente no terminal (como fizemos no chat):
```bash
perf report --stdio | head -100
```

### C) Visão Flat (Ignora quem chamou quem)
Às vezes você só quer saber "qual função gasta mais tempo" sem se importar se ela foi chamada pela Main ou por outra:
```bash
perf report --stdio --no-children -g none | head -50
```

---

## 5. E se eu não tiver Sudo para o Perf?

Se o seu sistema não permite alterar o `perf_event_paranoid` e você está travado, existem duas alternativas principais:

### Alternativa A: Valgrind (Callgrind)
O Callgrind simula a execução na CPU e conta instruções. A vantagem é que **sempre funciona**, não precisa de sudo, e é exato. A desvantagem é que é **10 a 20 vezes mais lento** que rodar normalmente.
Para instâncias grandes de milhões de iterações, você **deve** reduzir o `ITERATIONS` (ex: para 300.000) no `.h` antes de usar.

Para usar:
```bash
valgrind --tool=callgrind --callgrind-out-file=cg.out ./leitor instances/X-n101-k25.vrp 42
callgrind_annotate --auto=yes cg.out | head -80
```

### Alternativa B: GProf
É mais antigo, mas simples. Exige recompilar adicionando a flag `-pg`. É cerca de 2x mais lento.
```bash
g++ -std=c++20 -O2 -pg -g -Iinclude -o leitor_prof src/*.cpp
./leitor_prof instances/X-n101-k25.vrp 42
gprof leitor_prof gmon.out | head -40
```
*(Nota: Gprof requer compilação estática ou flags especiais para analisar threads e certas otimizações, então o perf continua sendo o padrão ouro).*
