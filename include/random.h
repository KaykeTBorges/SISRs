#ifndef _RANDOM_H_
#define _RANDOM_H_

#include <random>

namespace Random
{
std::mt19937_64 &gen();
void randomize(uint64_t s = std::mt19937_64::default_seed);
int getInt();
int getInt(int min, int max);
double getReal(double min, double max);
// Retorna true com probabilidade p usando comparação inteira direta no output do mt19937_64.
// Equivale a getReal(0,1) < p sem construir uniform_real_distribution nem converter para double.
bool getBool(double p);
} // namespace Random

#endif

// end of random.h