#include "recreate.h"

double evaluateInsertion(int anterior, int posterior, int atual){
    CVRPInstance& instance = CVRPInstance::getInstance();
    
    return instance.distanceMatrix[anterior][atual] + instance.distanceMatrix[atual][posterior]
            - instance.distanceMatrix[anterior][posterior];
}

bool evaluateCapacity(const Veiculo &v, int atual){
    CVRPInstance& instance = CVRPInstance::getInstance();

    return v.usedCapacity + instance.nodes[atual].demand <= instance.capacity;
}

bool pulaBlinkRate(){
    return Random::getReal(0, 1) >= 1 - BLINK_RATE;
}

void evaluatePosition(const Solution &s, int clienteId, MelhorPosicao &best){
    CVRPInstance& instance = CVRPInstance::getInstance();

    for (int tourId = 0; tourId < s.Tours.size(); tourId++) {
        const Veiculo& t = s.Tours[tourId];
        if (!evaluateCapacity(t, clienteId)) continue;
        for (int i = 0; i < t.route.size() - 1; i++) {
            if (pulaBlinkRate()) continue;
            double delta = evaluateInsertion(t.route[i], t.route[i+1], clienteId);
            if (delta < best.custo) {
                best.custo = delta;
                best.tourId = tourId;  
                // i + 1 por conta do insert de acontecer naquela posição em especifica e dar um shift 
                best.idx = i + 1;       
            }
        }
    } 
}

void adcionarNovoTour(Solution &s, int clienteId){
    CVRPInstance& instance = CVRPInstance::getInstance();

    Veiculo t;
    t.route = {0, clienteId, 0};
    t.cost = recalculateTourCost(t.route);
    t.usedCapacity = instance.nodes[clienteId].demand;
    s.Tours.push_back(t);

    s.localizacao[clienteId].idx = 1;
    s.localizacao[clienteId].tourId = s.Tours.size() - 1;
    s.totalCost += t.cost;
}

void recreate(Solution &s){
    decidirSort(s.ausentes);

    for(int i = 0; i < s.ausentes.size(); i++){
        MelhorPosicao mp;

        evaluatePosition(s, s.ausentes[i], mp);
        if(mp.idx != -1){
            inserirClienteTour(s, mp.tourId, mp.idx, s.ausentes[i]);
        }else{
            adcionarNovoTour(s, s.ausentes[i]);
        }
    }
}

void decidirSort(std::vector<int>& ausentes){
    int tipo = Random::getInt(0, 10);

    if (tipo < 4)
        sortRandom(ausentes);
    else if (tipo < 8)
        std::sort(ausentes.begin(), ausentes.end(), sortDemand);
    else if (tipo < 10)
        std::sort(ausentes.begin(), ausentes.end(), sortFar);
    else
        std::sort(ausentes.begin(), ausentes.end(), sortClose);
}

void sortRandom(std::vector<int>& ausentes){
    std::shuffle(ausentes.begin(), ausentes.end(), Random::gen());
}

bool sortDemand(int a, int b){
    CVRPInstance& instance = CVRPInstance::getInstance();
    return instance.nodes[a].demand > instance.nodes[b].demand;
}

bool sortFar(int a, int b){
    CVRPInstance& instance = CVRPInstance::getInstance();
    return instance.distanceMatrix[a] > instance.distanceMatrix[b];
}

bool sortClose(int a, int b){
    CVRPInstance& instance = CVRPInstance::getInstance();
    return instance.distanceMatrix[a] < instance.distanceMatrix[b];
}
