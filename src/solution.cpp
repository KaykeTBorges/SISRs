#include "solution.h"
#include "ruin.h"
#include "recreate.h"

Solution buildTrivial(){
    CVRPInstance & instance = CVRPInstance::getInstance();

    Solution s;
    s.localizacao.resize(instance.dimension + 1);
    s.ausentes.reserve(instance.dimension);

    for (int c = 1; c <= instance.dimension; c++) {
        if (c == instance.depotId) continue;
        Veiculo v;
        v.route = {instance.depotId, c, instance.depotId};
        v.usedCapacity = instance.nodes[c].demand;
        s.Tours.push_back(v);
    }

    recalculateLocalizacao(s);

    recalculateTotalCost(s);

    return s;
}

double recalculateTourCost(std::vector<int> & route){
    CVRPInstance & instance = CVRPInstance::getInstance();
    double value = 0;
    for(int c = 0; c < route.size() - 1; c++){
        value += instance.distanceMatrix[route[c]][route[c+1]];
    }
    return value;
}

// usado tbm para tirar veiculos sem clientes
void recalculateLocalizacao(Solution &s){
    for(int t = 0; t < s.Tours.size(); t++){
        if(s.Tours[t].route.size() <= 2){
            s.Tours[t] = std::move(s.Tours.back());
            s.Tours.pop_back();
            t--;
            continue;
        }
        for(int idx = 1; idx < s.Tours[t].route.size() - 1; idx++){
            int clienteId = s.Tours[t].route[idx];
            s.localizacao[clienteId] = {t, idx};
        }
    }
}

void recalculateLocalizacaoClientesNoTour(Solution &s, int tour, int idx_new){
    for(int idx = idx_new; idx < s.Tours[tour].route.size() - 1; idx++){
        int clienteId = s.Tours[tour].route[idx];
        s.localizacao[clienteId] = {tour, idx};
    }
}

void recalculateTour(Solution &s, int tourInicial){
    for(int t = tourInicial; t < s.Tours.size(); t++){
        recalculateLocalizacaoClientesNoTour(s, t);
    }
}

void recalculateTotalCost(Solution &s){
    s.totalCost = 0.0;
    for(int t = 0; t < s.Tours.size(); t++){
        s.Tours[t].cost = recalculateTourCost(s.Tours[t].route);
        s.totalCost += s.Tours[t].cost;
        
    }
}

void removerClienteTour(Solution &s, int tour, int idx){
    CVRPInstance& instance = CVRPInstance::getInstance();
    int clienteRemovido = s.Tours[tour].route[idx];

    s.Tours[tour].usedCapacity -= instance.nodes[clienteRemovido].demand;

    s.localizacao[clienteRemovido] = {-1, -1};

    s.ausentes.push_back(clienteRemovido);

    s.Tours[tour].route.erase(s.Tours[tour].route.begin() + idx);

    // isso aqui vai dar merda porque quando retirar o cliente da rota e atualizar a localização
    // a retirada em cadeia da ruin vai dar errado, logo talvez seja melhor eu só tirar 
    // e chamar essas funções depois
    
    // recalculateLocalizacaoTour(s, tour, idx);
    // s.Tours[tour].cost = recalculateTourCost(s.Tours[tour].route);  

}

void inserirClienteTour(Solution &s, int tour, int idx, int clienteId){
    CVRPInstance& instance = CVRPInstance::getInstance();
    Veiculo& veiculo = s.Tours[tour];

    int anterior = veiculo.route[idx - 1];
    int posterior = veiculo.route[idx];
    double delta = instance.distanceMatrix[anterior][clienteId] + instance.distanceMatrix[clienteId][posterior]
                 - instance.distanceMatrix[anterior][posterior];

    std::vector<int> novaRoute;
    novaRoute.reserve(veiculo.route.size() + 1);

    novaRoute.insert(novaRoute.end(), veiculo.route.begin(), veiculo.route.begin() + idx);
    novaRoute.push_back(clienteId);
    novaRoute.insert(novaRoute.end(), veiculo.route.begin() + idx, veiculo.route.end());

    veiculo.route = std::move(novaRoute);

    s.localizacao[clienteId] = {tour, idx};
    veiculo.usedCapacity += instance.nodes[clienteId].demand;

    recalculateLocalizacaoClientesNoTour(s, tour, idx);
    veiculo.cost += delta;
    s.totalCost += delta;
}

void localSearch(Solution &s){
    CVRPInstance& instance = CVRPInstance::getInstance();

    Solution sBest = s;
    Solution sEstrela;

    double temp = TEMP_INICIAL;
    double c = calcularC(ITERATIONS);
    int i;

    std::vector<int> absenceCounter;
    absenceCounter.resize(instance.dimension + 1, 0);

    for(i = 0; i < ITERATIONS_FLEET; i++){
        int cardinalidadeA = 0;
        int cardinalidadeAEstrela = 0;

        sEstrela = s;

        ruin(sEstrela);
        recreateFleet(sEstrela);
        
        cardinalidadeA = s.ausentes.size();
        cardinalidadeAEstrela = sEstrela.ausentes.size();

        if(cardinalidadeAEstrela < cardinalidadeA){
            s = sEstrela;
        }else{
            int somaA = sumAbs(s, absenceCounter);
            int somaAEstrela = sumAbs(sEstrela, absenceCounter);
            if(somaAEstrela < somaA){
                s = sEstrela;
            }
        }

        if (cardinalidadeAEstrela == 0){
            sBest = sEstrela;

            removerTour(sEstrela, encontrarTourMenorSumAbs(s, absenceCounter));

            s = sEstrela;
        }
        

        if(!sEstrela.ausentes.empty()){
            for(int idx = 0; idx < sEstrela.ausentes.size(); idx++){
                absenceCounter[sEstrela.ausentes[idx]]++;
            }
        }
    }

    for(i; i < ITERATIONS; i++){
        sEstrela = s;

        ruin(sEstrela);
        recreate(sEstrela);

        if(sEstrela.totalCost < s.totalCost - funcaoLog(temp)){
            s = sEstrela;
        }
        if(sEstrela.totalCost < sBest.totalCost){
            sBest = sEstrela;
        }
        temp = temp * c;
    }
}

double funcaoLog(double &temp){
    double uni = Random::getReal(0, 1);
    return temp * std::log(uni);
}

double calcularC(int f){
        double elevado = 1.0 / f;
        double t = TEMP_FINAL / TEMP_INICIAL;

        return std::pow(t, elevado);
}

int sumAbs(const Solution& s, const std::vector<int>& absenceCounter) {
    int sum = 0;

    for (int cliente : s.ausentes) {
        sum += absenceCounter[cliente];
    }

    return sum;
}

int encontrarTourMenorSumAbs(const Solution& s, const std::vector<int>& absenceCounter){
    int menorSumAbs = std::numeric_limits<int>::max();
    int tourRemover = -1;

    for(int t = 0; t < s.Tours.size(); t++){
        int soma = 0;
        for(int idx = 1; idx < s.Tours[t].route.size() - 1; idx++){
            soma += absenceCounter[s.Tours[t].route[idx]];
        }
        if(soma < menorSumAbs){
            menorSumAbs = soma;
            tourRemover = t;
        }
    }
    return tourRemover;
}

void removerTour(Solution &s, int tour){
    if(tour == -1) return;

    // Coloca todos os clientes do tour em ausentes
    for (int idx = 1; idx < s.Tours[tour].route.size() - 1; idx++){

        int clienteId = s.Tours[tour].route[idx];

        s.ausentes.push_back(clienteId);
        s.localizacao[clienteId] = {-1, -1};
    }

    // Se não for o último, move o último para a posição removida
    if (tour != s.Tours.size() - 1){

        s.Tours[tour] = std::move(s.Tours.back());
        // Remove o último elemento
        s.Tours.pop_back();

        recalculateTour(s, tour);
    }else{
        s.Tours.pop_back();
    }
    recalculateTotalCost(s);
}