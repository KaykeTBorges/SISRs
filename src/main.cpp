#include "data.h"
#include "random.h"
#include "solution.h"
#include "localSearch.h"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>

int main(int argc, char** argv){

    if (argc < 2 || argc > 3) {
        std::cout << "Uso: " << argv[0]
                  << " <instancia.vrp> [seed]" << std::endl;
        return 1;
    }

    if (argc == 3) {
        try {
            const std::string seedArgument = argv[2];
            size_t parsedCharacters = 0;
            const unsigned long long seed = std::stoull(seedArgument, &parsedCharacters);
            if (parsedCharacters != seedArgument.size()) {
                throw std::invalid_argument("seed invalida");
            }
            Random::randomize(static_cast<uint64_t>(seed));
        }
        catch (const std::exception& e) {
            std::cerr << "Seed invalida: " << e.what() << std::endl;
            return 1;
        }
    } else {
        Random::randomize();
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
        const auto searchStart = std::chrono::steady_clock::now();
        localSearch(s);
        const auto searchEnd = std::chrono::steady_clock::now();
        const std::chrono::duration<double> searchTime = searchEnd - searchStart;

        // 4. Mostra resultado
        std::cout << "\n--- Solucao final ---" << std::endl;

        std::cout << "Custo: "
                  << s.totalCost << std::endl;

        std::cout << "Numero de veiculos: "
                  << s.Tours.size() << std::endl;

        std::cout << "Tempo (segundos): "
              << searchTime.count() << std::endl;

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