#include "recreate.h"

bool evaluateCapacity(const Veiculo &v, int atual){
    CVRPInstance& instance = CVRPInstance::getInstance();

    return v.usedCapacity + instance.nodes[atual].demand <= instance.capacity;
}

bool pulaBlinkRate(){
    // Random::getBool(p) retorna true com probabilidade p usando comparação inteira
    // no output bruto do mt19937_64 — sem uniform_real_distribution, sem float.
    return Random::getBool(BLINK_RATE);
}

void evaluatePosition(const Solution &s, int clienteId, MelhorPosicao &best){
    CVRPInstance& instance = CVRPInstance::getInstance();

    const std::vector<double>& distCliente = instance.distanceMatrix[clienteId];

    for (int tourId = 0; tourId < s.Tours.size(); tourId++) {
        const Veiculo& t = s.Tours[tourId];

        if (!evaluateCapacity(t, clienteId)) continue;

        for (int i = 0; i < t.route.size() - 1; i++) {
            if (pulaBlinkRate()) continue;

            double delta = distCliente[t.route[i]] + distCliente[t.route[i+1]]
                         - instance.distanceMatrix[t.route[i]][t.route[i+1]];

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
    t.route = {instance.depotId, clienteId, instance.depotId};
    t.cost = instance.distanceMatrix[instance.depotId][clienteId] 
            + instance.distanceMatrix[clienteId][instance.depotId];
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
    s.ausentes.clear();
    // reconstrói localizacao uma única vez
    // inserirClienteTour não a mantém atualizada durante o loop
    recalculateLocalizacao(s);
}

void recreateFleet(Solution &s){
    if (s.ausentes.empty()) return;
    decidirSort(s.ausentes);

    for(int i = s.ausentes.size() - 1; i >= 0; i--){
        MelhorPosicao mp;

        evaluatePosition(s, s.ausentes[i], mp);
        if(mp.idx != -1){
            inserirClienteTour(s, mp.tourId, mp.idx, s.ausentes[i]);
            s.ausentes[i] = s.ausentes.back();
            s.ausentes.pop_back();
        }
    }
    recalculateLocalizacao(s);
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
    return instance.distanceMatrix[a][instance.depotId] > instance.distanceMatrix[b][instance.depotId];
}

bool sortClose(int a, int b){
    CVRPInstance& instance = CVRPInstance::getInstance();
    return instance.distanceMatrix[a][instance.depotId] < instance.distanceMatrix[b][instance.depotId];
}
