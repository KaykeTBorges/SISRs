#ifndef SOLUTION_H
#define SOLUTION_H

#include "data.h"
#include <vector>

struct Veiculo {
    std::vector<int> route;
    int usedCapacity = 0;
    double cost = 0.0; // custo do deslocamento da rota
    
};

// isso me garante saber a localização de cada cliente
struct ClienteLocalizacao {
    int tourId = -1; // quando ele não está dentro vai estar -1
    int idx = -1; // nos dois daqui (-1)
    bool naRota = true;
};

struct Solution {
    std::vector<Veiculo> Tours;
    std::vector<int> ausentes;
    // aqui o indice desse vetor, é com base no dado cliente
    // localizacao[clienteID]
    std::vector<ClienteLocalizacao> localizacao; 
    double totalCost = 0.0;
};

Solution buildTrivial();
double recalculateTourCost(std::vector<int> & route);
void recalculateLocalizacao(Solution &s);
void recalculateTotalCost(Solution &s);



#endif // SOLUTION_H