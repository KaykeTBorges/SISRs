#include "random.h"

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
