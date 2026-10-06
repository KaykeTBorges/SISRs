#include "ruin.h"
#include <algorithm>
#include <math.h>
#include <unordered_set>

void removerString(Solution &s, int tour, int idxInicial, int cardinalidade) {
    CVRPInstance& instance = CVRPInstance::getInstance();

    double custoAntigo = s.Tours[tour].cost;

    int fim = idxInicial + cardinalidade;

    std::vector<int> novaRoute;
    novaRoute.reserve(s.Tours[tour].route.size() - cardinalidade);

    for (int idx = 0; idx < s.Tours[tour].route.size(); idx++) {
        int cliente = s.Tours[tour].route[idx];

        if (idx >= idxInicial && idx < fim) {

            s.localizacao[cliente] = {-1, -1};

            s.ausentes.push_back(cliente);

            s.Tours[tour].usedCapacity -= instance.nodes[cliente].demand;

            continue;
        }

        novaRoute.push_back(cliente);
    }

    s.Tours[tour].route = std::move(novaRoute);

    // 1. Tour ficou vazio
    if (s.Tours[tour].route.size() <= 2) {
        s.totalCost -= custoAntigo;

        // Se não for o último, move o último para cá.
        if (tour != s.Tours.size() - 1) {
            s.Tours[tour] = std::move(s.Tours.back());

            s.Tours.pop_back();

            // O último tour mudou de índice
            recalculateLocalizacaoClientesNoTour(s, tour, 1);

        } else {
            s.Tours.pop_back();
        }
        return;
    }

    // 2. Tour ainda possui clientes
    recalculateLocalizacaoClientesNoTour(s, tour, idxInicial);

    s.Tours[tour].cost = recalculateTourCost(s.Tours[tour].route);

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

    // aqui defini o escopo do começo do random, se der 0 
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

void ruin(Solution &s){
    CVRPInstance& instance = CVRPInstance::getInstance();

    double lsMax = calcularLsMax(s);
    double ksMax = calcularKsMax(lsMax);
    int ks = sortearInteiroUniforme(ksMax);

    // c seed
    // intervalo fechado dos dois lados (inclusos)
    // retirando o 0 que é o deposito e -1 porque o dimension é conseguido como o size
    // ai ele é uma contagem, logo não uma indexação
    int cSeed = Random::getInt(2, instance.dimension);
    
    // R 
    std::unordered_set<int> R;


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
            int lt = sortearInteiroUniforme(ltMax);
            int idxInicial = decidirIdxInicial(s, c_t, lt);
            removerString(s, t, idxInicial, lt);
            R.insert(t);
        }
        i++;
    }
}



