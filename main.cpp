#include "data.h"
#include "solution.h"
#include "random.h"
#include "ruin.h"
#include "recreate.h"
#include <iostream>

int main(int argc, char** argv)
{
    Random::randomize();
    
    if (argc < 2) {
        std::cout << "Uso: " << argv[0] << " <arquivo.vrp>" << std::endl;
        return 1;
    }

    try {
        loadInstance(argv[1]);

        std::cout << "\n=== Teste do Evaluate Position ===" << std::endl;
        Solution s = buildTrivial();
        ruin(s);

        std::cout << "Clientes ausentes apos ruin: ";
        for (int c : s.ausentes) std::cout << c << " ";
        std::cout << std::endl;

        if (!s.ausentes.empty()) {
            int clienteTeste = s.ausentes[0];
            MelhorPosicao best;
            evaluatePosition(s, clienteTeste, best);

            std::cout << "Melhor posicao para cliente " << clienteTeste << ":" << std::endl;
            std::cout << "  tourId: " << best.tourId << std::endl;
            std::cout << "  idx: " << best.idx << std::endl;
            std::cout << "  custo: " << best.custo << std::endl;

            if (best.tourId != -1) {
                std::cout << "  Rota do tour escolhido: ";
                for (int c : s.Tours[best.tourId].route) std::cout << c << " ";
                std::cout << std::endl;
            }
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Erro ao carregar instancia: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}