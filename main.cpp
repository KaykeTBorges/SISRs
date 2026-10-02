#include "data.h"
#include "solution.h"
#include "ruin.h"
#include "recreate.h"

#include <iostream>

int main(int argc, char** argv){

    Random::randomize();

    if (argc < 2) {
        std::cout << "Uso: " << argv[0]
                  << " <a/A-n32-k5.vrp>" << std::endl;
        return 1;
    }

    try {

        // 1. Carrega a instância
        loadInstance(argv[1]);

        // 2. Cria solução trivial
        Solution s = buildTrivial();

        std::cout << "=== LOCAL SEARCH SISRs ===" << std::endl;

        std::cout << "\n--- Solucao inicial ---" << std::endl;
        std::cout << "Custo: "
                  << s.totalCost << std::endl;

        std::cout << "Numero de veiculos: "
                  << s.Tours.size() << std::endl;

        // 3. Executa o Local Search
        localSearch(s);

        // 4. Mostra resultado
        std::cout << "\n--- Solucao final ---" << std::endl;

        std::cout << "Custo: "
                  << s.totalCost << std::endl;

        std::cout << "Numero de veiculos: "
                  << s.Tours.size() << std::endl;

        std::cout << "Clientes ausentes: "
                  << s.ausentes.size() << std::endl;

        // 5. Mostra as rotas finais
        std::cout << "\n--- Rotas finais ---" << std::endl;

        for (int t = 0; t < s.Tours.size(); t++) {

            std::cout << "Tour " << t << ": ";

            for (int cliente : s.Tours[t].route) {
                std::cout << cliente << " ";
            }

            std::cout << "| custo = "
                      << s.Tours[t].cost
                      << std::endl;
        }

        // 6. Mostra clientes ausentes, caso existam
        if (!s.ausentes.empty()) {

            std::cout << "\nClientes ausentes: ";

            for (int cliente : s.ausentes) {
                std::cout << cliente << " ";
            }

            std::cout << std::endl;
        }

    }
    catch (const std::runtime_error& e) {

        std::cerr << "Erro ao carregar instancia: "
                  << e.what() << std::endl;

        return 1;
    }

    return 0;
}