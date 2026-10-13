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
    // O gerador mt19937_64 gera números inteiros de 0 até o seu limite máximo (2^64 - 1)
    // Esse limite é 18.4 quintilhões
    static constexpr uint64_t limiteMaximo = std::mt19937_64::max();
    
    // inves de converter o número sorteado gigante em um double entre 0 e 1 o que exigiria processamento de ponto flutuante
    //  é feito então a lógica inversa transforma a probabilidade (p) -> ex: 0.01 em um número inteiro gigante baseado no gerador de numeros
    // assim se p=0.01 o valor limitante vira exatamente 1% de 18.4 quintilhões
    uint64_t valorLimitante = static_cast<uint64_t>(p * static_cast<double>(limiteMaximo));
    
    // a CPU é seria mais eficiente comparando dois números inteiros
    // como o número sorteado cai em qualquer lugar entre 0 e o limite maximo com a mesma chance
    // ele tem exatamente (p) de probabilidade de ser menor que valor limitante
    return gen()() < valorLimitante;
}