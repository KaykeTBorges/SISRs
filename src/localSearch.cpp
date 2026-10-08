#include "localSearch.h"

#include "random.h"
#include "recreate.h"
#include "ruin.h"

#include <cmath>
#include <limits>
#include <utility>
#include <vector>

namespace {

double funcaoLog(double &temp) {
    double uni = Random::getReal(0, 1);
    return temp * std::log(uni);
}

double calcularC(int f) {
    double elevado = 1.0 / f;
    double t = TEMP_FINAL / TEMP_INICIAL;

    return std::pow(t, elevado);
}

int sumAbs(const Solution& s, const std::vector<int>& absenceCounter) {
    int sum = 0;

    for (int cliente : s.ausentes) {
        sum += absenceCounter[cliente];
    }

    return sum;
}

int encontrarTourMenorSumAbs(const Solution& s, const std::vector<int>& absenceCounter) {
    int menorSumAbs = std::numeric_limits<int>::max();
    int tourRemover = -1;

    for (int t = 0; t < s.Tours.size(); t++) {
        int soma = 0;
        for (int idx = 1; idx < s.Tours[t].route.size() - 1; idx++) {
            soma += absenceCounter[s.Tours[t].route[idx]];
        }
        if (soma < menorSumAbs) {
            menorSumAbs = soma;
            tourRemover = t;
        }
    }
    return tourRemover;
}

void removerTour(Solution &s, int tour) {
    if (tour == -1) return;

    double custoRemovido = s.Tours[tour].cost;

    for (int idx = 1; idx < s.Tours[tour].route.size() - 1; idx++) {
        int clienteId = s.Tours[tour].route[idx];

        s.ausentes.push_back(clienteId);
        s.localizacao[clienteId] = {-1, -1};
    }

    if (tour != s.Tours.size() - 1) {
        s.Tours[tour] = std::move(s.Tours.back());
        s.Tours.pop_back();
        recalculateLocalizacaoClientesNoTour(s, tour, 1);
    } else {
        s.Tours.pop_back();
    }
    s.totalCost -= custoRemovido;
}

}

void localSearch(Solution &s) {
    CVRPInstance& instance = CVRPInstance::getInstance();

    Solution sBest = s;
    Solution sEstrela;

    double temp = TEMP_INICIAL;
    double c = calcularC(ITERATIONS);
    int i;

    std::vector<int> absenceCounter;
    absenceCounter.resize(instance.dimension + 1, 0);

    for (i = 0; i < ITERATIONS_FLEET; i++) {
        int cardinalidadeA = 0;
        int cardinalidadeAEstrela = 0;

        sEstrela = s;

        ruin(sEstrela);
        recreateFleet(sEstrela);

        cardinalidadeA = s.ausentes.size();
        cardinalidadeAEstrela = sEstrela.ausentes.size();

        int sumAbsS = sumAbs(s, absenceCounter);
        if (sEstrela.ausentes.size() < s.ausentes.size() || sumAbs(sEstrela, absenceCounter) < sumAbsS) {
            s = sEstrela;
        }

        if (sEstrela.ausentes.empty()) {
            s = sEstrela;
            sBest = s;
            if (s.Tours.size() > 1) removerTour(s, encontrarTourMenorSumAbs(s, absenceCounter));
        }

        if (!sEstrela.ausentes.empty()) {
            for (int idx = 0; idx < sEstrela.ausentes.size(); idx++) {
                absenceCounter[sEstrela.ausentes[idx]]++;
            }
        }
    }

    s = sBest;

    for (i; i < ITERATIONS; i++) {
        sEstrela = s;

        ruin(sEstrela);
        recreate(sEstrela);

        if (sEstrela.totalCost < s.totalCost - funcaoLog(temp)) {
            s = sEstrela;
        }
        if (sEstrela.totalCost < sBest.totalCost) {
            sBest = sEstrela;
        }
        temp = temp * c;
    }

    s = sBest;
}