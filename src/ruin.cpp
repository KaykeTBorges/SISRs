#include "ruin.h"
#include <algorithm>
#include <math.h>
#include <unordered_set>

void removerString(Solution &s, int tour, int idxInicial, int cardinalidade) {
    CVRPInstance& instance = CVRPInstance::getInstance();

    double custoAntigo = s.Tours[tour].cost;

    int fim = idxInicial + cardinalidade;

    // nós antes e depois da string antes do erase
    int anterior = s.Tours[tour].route[idxInicial - 1];
    int posterior = s.Tours[tour].route[fim];

    double custoArcoRemovidos = 0.0;

    // processa e acumula os arcos do valor a ser removido
    for (int idx = idxInicial; idx < fim; idx++) {
        int cliente = s.Tours[tour].route[idx];

        // começa em 1 antes porque ai já calculo aqui o inicial, quebrando os removidos
        // não calculando ele fora, só calculando fora o fim, porque precisa, já que para antes do fim
        custoArcoRemovidos += instance.distanceMatrix[s.Tours[tour].route[idx - 1]][cliente];

        s.localizacao[cliente] = {-1, -1};
        s.ausentes.push_back(cliente);
        s.Tours[tour].usedCapacity -= instance.nodes[cliente].demand;
    }
    custoArcoRemovidos += instance.distanceMatrix[s.Tours[tour].route[fim - 1]][posterior];

    s.Tours[tour].route.erase(s.Tours[tour].route.begin() + idxInicial, s.Tours[tour].route.begin() + fim);

    // 1 - tour ficou vazio, logo é preciso apagar
    // uso ele como menor igual a dois porque se ocorrer algum bug ele aceita mesmo assim
    // mas a ideia é que fosse um conferimento de se é igual a 2 apenas
    if (s.Tours[tour].route.size() <= 2) {
        s.totalCost -= custoAntigo;

        // se não for o último move o último para cá, pra não precisar dar erase
        if (tour != s.Tours.size() - 1) {
            s.Tours[tour] = std::move(s.Tours.back());

            s.Tours.pop_back();

            // o último tour mudou de índice
            recalculateLocalizacaoClientesNoTour(s, tour, 1);
        } else {
            // se for o ultimo, ai já dá pop back de gratis
            s.Tours.pop_back();
        }
        return;
    }

    // 2 - tour ainda possui clientes
    recalculateLocalizacaoClientesNoTour(s, tour, idxInicial);

    s.Tours[tour].cost = custoAntigo - custoArcoRemovidos
                       + instance.distanceMatrix[anterior][posterior];

    s.totalCost += s.Tours[tour].cost - custoAntigo;
}

void removerSplitString(Solution &s, int tour, int idxInicial, int cardinalidade, int m) {
    CVRPInstance& instance = CVRPInstance::getInstance();

    double custoAntigo = s.Tours[tour].cost;

    int fim = idxInicial + cardinalidade;

    // nós antes e depois da string antes do erase
    int anterior = s.Tours[tour].route[idxInicial - 1];
    int posterior = s.Tours[tour].route[fim];

    int mDentro = Random::getInt(idxInicial+1, fim-1-m);

    int primeiroPreservado = s.Tours[tour].route[mDentro];
    int ultimoPreservado = s.Tours[tour].route[mDentro + m - 1];

    double custoArcoRemovidos = 0.0;

    double custoNovoM = instance.distanceMatrix[anterior][primeiroPreservado];
    custoNovoM += instance.distanceMatrix[ultimoPreservado][posterior];

    // processa e acumula os arcos do valor a ser removido
    int idx;
    for(idx = idxInicial; idx < mDentro; idx++){
        int cliente = s.Tours[tour].route[idx];

        custoArcoRemovidos += instance.distanceMatrix[s.Tours[tour].route[idx - 1]][cliente];
        s.localizacao[cliente] = {-1, -1};
        s.ausentes.push_back(cliente);
        s.Tours[tour].usedCapacity -= instance.nodes[cliente].demand;
    }
    custoArcoRemovidos += instance.distanceMatrix[s.Tours[tour].route[mDentro - 1]][primeiroPreservado];

    for(idx = mDentro + m; idx < fim; idx++){
        int cliente = s.Tours[tour].route[idx];

        custoArcoRemovidos += instance.distanceMatrix[s.Tours[tour].route[idx - 1]][cliente];
        s.localizacao[cliente] = {-1, -1};
        s.ausentes.push_back(cliente);
        s.Tours[tour].usedCapacity -= instance.nodes[cliente].demand;
    }
    custoArcoRemovidos += instance.distanceMatrix[s.Tours[tour].route[fim - 1]][posterior];

    // tira o final depois o primeiro, que ai da validade aos indices do primeiro
    // porque se tirado os primeiros, o indice dos ultimos mudam e perdemos a referencia
    s.Tours[tour].route.erase(s.Tours[tour].route.begin() + mDentro + m, s.Tours[tour].route.begin() + fim);
    s.Tours[tour].route.erase(s.Tours[tour].route.begin() + idxInicial, s.Tours[tour].route.begin() + mDentro);

    recalculateLocalizacaoClientesNoTour(s, tour, idxInicial);

    s.Tours[tour].cost = custoAntigo - custoArcoRemovidos + custoNovoM;

    s.totalCost += s.Tours[tour].cost - custoAntigo;
}

int decidirIdxInicial(const Solution &s, int cliente, int lt){
    int posicaoCliente = s.localizacao[cliente].idx;
    int tour = s.localizacao[cliente].tourId;
    int tamanhoTour = cardinalidadeTour(s.Tours[tour]);

    // aqui é -1 porque um vetor está indexado em 0 e por isso, aqui ele já elimina a posição por conta disso
    // porque ele limita [0, 1, 2, 3] sendo a posição 3, temos espaco a esquerda, 2 que é o indice, mas tbm a contagem
    // de elementos desconsiderando o começo
    int espacoEsquerda = posicaoCliente - 1;
    // a cardinalidade já tem tirado tanto o inicio como o fim, então a subtração, gera as posições posteriores ao cliente
    int espacoDireita = tamanhoTour - posicaoCliente;

    // a quantidade de clientes antes do cliente c_t podem entrar na retirada
    // os indices deles, sempre é 0 ou um valor maior com base na cardinalidade e o espaco a direita
    // esse valor vai ser quanto eu preciso de posições antes do cliente com base no espaço da direita
    // porque eu subtraio o espaço da direita porque descubro quanto que sobre de cardinalidade para ser preenchida
    // com o outro lado

    // aqui define o escopo do começo do random, se der 0 
    // quer dizer que meu espaço da direita é maior que a cardinalidade
    // ou voce não precisa retirar nada para caber, ou precisa retirar com base no espaço da direita, para caber
    int clienteAntesMin = std::max(0, lt - 1 - espacoDireita);
    // aqui defini o escopo do fim do random, o espaco a esquerda é o limitante
    // ele que define se pode ou não voltar mais atras na posição atual do cliente
    // não pode extrapolar o espaço da esquerda, se não vai acessar coisa errada
    int clienteAntesMax = std::min(lt - 1, espacoEsquerda);

    // porque no fim o que queremos com isso é subtrair a posição atual do cliente
    // onde seja possivel não ultrapassar o começo nem o fim
    int posicoesAtras = Random::getInt(clienteAntesMin, clienteAntesMax);

    return posicaoCliente - posicoesAtras;
}

// é const aqui porque só vai fazer o cáculo não vai mudar a solução
double cardinalidadeTour(const Veiculo& v) {
    return v.route.size() - 2;
}

double cardinalidadeMediaTours(const Solution &s){
    double somaCardinalidade = 0.0;
    for(int t = 0; t < s.Tours.size(); t++){
        somaCardinalidade += cardinalidadeTour(s.Tours[t]);
    }
    return somaCardinalidade / s.Tours.size();
}

double calcularLsMax(const Solution &s){
    return std::min(L_MAX, cardinalidadeMediaTours(s));
}

double calcularKsMax(double lsMax){
    return ((4 * C_BAR) / (lsMax + 1)) - 1; 
}

double calcularLtMax(const Veiculo& v, double lsMax) {
    return std::min(cardinalidadeTour(v), lsMax);
}

int sortearInteiroUniforme(double max){
    return static_cast<int>(floor(Random::getReal(1, max + 1)));
}

int calcularM(int mMax){
    if(mMax < 1) return 0;
    int m = 1;
    while(m < mMax){
        if(Random::getBool(ALPHA)) return m;
        else{
            m += 1;
        }
    }
    return m;
}

void ruin(Solution &s){
    CVRPInstance& instance = CVRPInstance::getInstance();

    double lsMax = calcularLsMax(s);
    double ksMax = calcularKsMax(lsMax);
    int ks = sortearInteiroUniforme(ksMax);

    // c seed
    // intervalo fechado dos dois lados (inclusos)
    // retirando o 1 que é o deposito
    // ai ele é uma contagem, logo não uma indexação
    int cSeed = Random::getInt(2, instance.dimension);
    
    // R 
    std::unordered_set<int> R;
    R.reserve(s.Tours.size());

    size_t i = 0;
    // enquanto ainda tiver clientes perto dele, continua procurando
    while(i < instance.adjacencyList[cSeed].size() && R.size() < ks){
        int c = instance.adjacencyList[cSeed][i];

        int t = s.localizacao[c].tourId;

        // algo legal que tenho é que se na localizacao o meu cliente tiver me -1 ele está na lista de ausentes
        // o uso do count é mais indicado porque se eu usar find ele vai gerar um iterador, e não vou usar para nada
        // além do mais por ser um set o count só vai poder gerar 0 ou 1
        if(t != -1 && !R.count(t)){
            int c_t = c;
            double ltMax = calcularLtMax(s.Tours[t], lsMax);
            
            if(Random::getInt(0, 1)){
                int lt = sortearInteiroUniforme(ltMax);
                int idxInicial = decidirIdxInicial(s, c_t, lt);
                removerString(s, t, idxInicial, lt);
            }else{
                int l = sortearInteiroUniforme(ltMax);
                int mMax = cardinalidadeTour(s.Tours[t]) - l;

                if (l < 2 || mMax < 1) {
                    // Remoção normal
                    int idxInicial = decidirIdxInicial(s, c_t, l);
                    removerString(s, t, idxInicial, l);
                } else {
                    // Split string
                    int m = calcularM(mMax);
                    int lt = l + m;

                    int idxInicial = decidirIdxInicial(s, c_t, lt);
                    removerSplitString(s, t, idxInicial, lt, m);
                }
            }
            R.insert(t);
        }
        i++;
    }
}