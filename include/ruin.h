#ifndef RUIN_H
#define RUIN_H

#include "data.h"
#include "solution.h"
#include "random.h"
#include <vector>

constexpr double C_BAR = 10.0;
constexpr double L_MAX = 10.0;

void removerString(Solution &s, int tour, int idxInicial, int cardinalidade);
int decidirIdxInicial(const Solution &s, int cliente, int lt);

double cardinalidadeTour(const Veiculo& v);
double cardinalidadeMediaTours(const Solution &s);

double calcularLsMax(const Solution &s);
double calcularKsMax(double lsMax);
double calcularLtMax(const Veiculo& v, double lsMax);
int sortearInteiroUniforme(double max);

void ruin(Solution &s);

#endif // RUIN_H