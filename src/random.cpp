#include "random.h"
#include <limits>

std::mt19937_64 &Random::gen()
{
    static std::mt19937_64 u{};
    return u;
}
void Random::randomize(uint64_t s)
{
    gen().seed(s);
}
int Random::getInt()
{
    return gen()();
}
int Random::getInt(int min, int max)
{
    static std::uniform_int_distribution<> d{};
    using parm_t = decltype(d)::param_type;
    return d(gen(), parm_t{min, max});
}
double Random::getReal(double min, double max)
{
    static std::uniform_real_distribution<> d{};
    using parm_t = decltype(d)::param_type;
    return d(gen(), parm_t{min, max});
}

bool Random::getBool(double p)
{
    // Converte o threshold p para o espaço inteiro do mt19937_64 ([0, 2^64)).
    // Comparar o output bruto com esse threshold é equivalente a getReal(0,1) < p
    // mas sem construir uniform_real_distribution, sem multiplicação float, sem shift.
    
    // Precisão: uniform_real_distribution<double> tem 2^53 valores distintos em [0,1);
    // esta abordagem tem 2^64 — mais granular, sem impacto prático para p=0.01.
    const uint64_t threshold = static_cast<uint64_t>(p * static_cast<double>(std::numeric_limits<uint64_t>::max()));
    return gen()() < threshold;
}
