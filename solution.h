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
void recalculateLocalizacao(Solution &s);
void recalculateLocalizacaoClienteNoTour(Solution &s, int tour, int idx_new = 1);
void recalculateLocalizacaoTour(Solution &s);

double recalculateTourCost(std::vector<int> & route);
void recalculateTotalCost(Solution &s);

void removerClienteTour(Solution &s, int tour, int idx);
void inserirClienteTour(Solution &s, int tour, int idx, int clienteId);

void localSearch(Solution &s);
double funcaoLog(double &temp);
double calcularC(int f);



#endif // SOLUTION_H