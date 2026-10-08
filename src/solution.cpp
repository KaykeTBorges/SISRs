#include "solution.h"

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

    veiculo.route.insert(veiculo.route.begin() + idx, clienteId);

    s.localizacao[clienteId] = {tour, idx};
    veiculo.usedCapacity += instance.nodes[clienteId].demand;
    veiculo.cost += delta;
    s.totalCost += delta;
    // localizacao dos clientes posteriores (idx+1 em diante) fica desatualizada intencionalmente
    // recreate e recreateFleet chamam recalculateLocalizacao(s) ao final para reconstruir
}
