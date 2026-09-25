#include "solution.h"

Solution buildTrivial(){
    CVRPInstance & instance = CVRPInstance::getInstance();

    Solution s;
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
    
    s.localizacao.resize(instance.dimension+1);
    for(int t = 0; t < s.Tours.size(); t++){
        for(int idx = 1; idx < s.Tours[t].route.size() - 1; idx++){
            int clienteId = s.Tours[t].route[idx];
            s.localizacao[clienteId] = {t, idx};
        }
    }
}

void recalculateTotalCost(Solution &s){
    s.totalCost = 0.0;
    for(int t = 0; t < s.Tours.size(); t++){
        s.Tours[t].cost = recalculateTourCost(s.Tours[t].route);
        s.totalCost += s.Tours[t].cost;
        
    }
}

