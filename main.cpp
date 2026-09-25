#include "data.h"
#include "solution.h"
#include <iostream>

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cout << "Uso: " << argv[0] << " <arquivo.vrp>" << std::endl;
        return 1;
    }

    try {
        loadInstance(argv[1]);

        std::cout << "\n=== Solucao Inicial ===" << std::endl;
        Solution s = buildTrivial();

        std::cout << "Numero de veiculos: " << s.Tours.size() << std::endl;

        // pega o primeiro veiculo como amostra
        std::cout << "Rota do veiculo 0: ";
        for (int c : s.Tours[0].route) std::cout << c << " ";
        std::cout << std::endl;

        std::cout << "UsedCapacity veiculo 0: " << s.Tours[0].usedCapacity << std::endl;
        std::cout << "Cost veiculo 0: " << s.Tours[0].cost << std::endl;

        std::cout << "Custo total da solucao: " << s.totalCost << std::endl;

        // checagem cruzada: pega um cliente, confirma que localizacao bate com route
        int clienteTeste = 5;
        int tourId = s.localizacao[clienteTeste].tourId;
        int pos = s.localizacao[clienteTeste].idx;
        std::cout << "Cliente " << clienteTeste << " deveria estar no tour " << tourId
                << ", posicao " << pos << std::endl;
        std::cout << "route[" << tourId << "][" << pos << "] = "
                << s.Tours[tourId].route[pos] << " (deveria ser " << clienteTeste << ")" << std::endl;

    } catch (const std::runtime_error& e) {
        std::cerr << "Erro ao carregar instancia: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}