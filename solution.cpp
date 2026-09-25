#include "solution.h"

Solution buildTrivial(){
    CVRPInstance & instance = CVRPInstance::getInstance();

    Solution s;
    s.localizacao.resize(instance.dimension);
    s.ausentes.reserve(instance.dimension);

    for (int c = 1; c < instance.dimension; c++) {
        Veiculo v;
        v.route = {0, c, 0};
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

void recalculateLocalizacao(Solution &s){
    CVRPInstance & instance = CVRPInstance::getInstance();
    
    for(int t = 0; t < s.Tours.size(); t++){
        for(int idx = 1; idx < s.Tours[t].route.size() - 1; idx++){
            int clienteId = s.Tours[t].route[idx];
            s.localizacao[clienteId] = {t, idx, true};
        }
    }
}

void recalculateLocalizacaoTour(Solution &s, int tour, int idx_new = 1){
    for(int idx = idx_new; idx < s.Tours[tour].route.size() - 1; idx++){
        int clienteId = s.Tours[tour].route[idx];
        s.localizacao[clienteId] = {tour, idx};
    }
}

void recalculateLocalizacaoAusente(Solution &s){
    for(int idx = 0; idx < s.ausentes.size(); idx++){
        int clienteAusente = s.ausentes[idx];
        s.localizacao[clienteAusente] = {-1, idx, false};
    }
}

void recalculateTotalCost(Solution &s){
    s.totalCost = 0.0;
    for(int t = 0; t < s.Tours.size(); t++){
        s.Tours[t].cost = recalculateTourCost(s.Tours[t].route);
        s.totalCost += s.Tours[t].cost;
        
    }
}

void removerClenteTour(Solution &s, int tour, int idx){
    CVRPInstance& instance = CVRPInstance::getInstance();
    int clienteRemovido = s.Tours[tour].route[idx];

    s.Tours[tour].usedCapacity -= instance.nodes[clienteRemovido].demand;

    s.localizacao[clienteRemovido] = {-1, s.ausentes.size(), false};

    s.ausentes.push_back(clienteRemovido);

    s.Tours[tour].route.erase(s.Tours[tour].route.begin() + idx);

    recalculateLocalizacaoTour(s, tour, idx);
    s.Tours[tour].cost = recalculateTourCost(s.Tours[tour].route);  

}

void inseriClienteTour(Solution &s, int tour, int idx, int clienteId){
    CVRPInstance& instance = CVRPInstance::getInstance();

    s.Tours[tour].route.insert(s.Tours[tour].route.begin() + idx, clienteId);

    s.ausentes.erase(s.ausentes.begin() + s.localizacao[clienteId].idx);

    s.localizacao[clienteId] = {tour, idx, true};

    s.Tours[tour].usedCapacity += instance.nodes[clienteId].demand;

    recalculateLocalizacaoTour(s, tour, idx);
    s.Tours[tour].cost = recalculateTourCost(s.Tours[tour].route);
}
