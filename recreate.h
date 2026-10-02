#ifndef RECREATE_H
#define RECREATE_H

#include "data.h"
#include "solution.h"
#include "random.h"
#include <vector>
#include <algorithm> 

constexpr double BLINK_RATE = 0.01;

struct MelhorPosicao{
    int idx = -1;
    int tourId = -1;
    double custo = INFINITY;
};

double evaluateInsertion(int anterior, int posterior, int atual);
bool evaluateCapacity(const Veiculo &v, int atual);
void evaluatePosition(const Solution &s, int clienteId, MelhorPosicao &best);
bool pulaBlinkRate();

void recreate(Solution &s);

void adcionarNovoTour(Solution &s, int clienteId);

void decidirSort(std::vector<int>& ausentes);
void sortRandom(std::vector<int>& ausentes);
bool sortDemand(int a, int b);
bool sortFar(int a, int b);
bool sortClose(int a, int b);

#endif // RUIN_H