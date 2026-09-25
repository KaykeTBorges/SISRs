#include "data.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <algorithm>

CVRPInstance& CVRPInstance::getInstance(){
    static CVRPInstance instance;
    return instance;
}

namespace {
    enum class Section {NONE, NODE_COORD, DEMAND, DEPOT};

    std::string trim(const std::string& s){
        // espaço em branco
        const std::string whitespace = " \t\n\r\f\v";

        size_t inicio = s.find_first_not_of(whitespace);
        size_t fim = s.find_last_not_of(whitespace);

        // a ultima ocorrencia é de letra, logo o tamanho tem que ser fim - inicio + 1
        // essa regra sem o +1 é não inclusiva
        size_t tamanho = fim - inicio + 1;

        return s.substr(inicio, tamanho);
    }
}

CVRPInstance& readInstance(const std::string& filepath){
    std::ifstream in(filepath);
    if(!in){
        throw std::runtime_error("Nao foi possivel abrir o arquivo: " + filepath);
    }

    CVRPInstance& instance = CVRPInstance::getInstance();
    instance.dimension = -1;
    instance.capacity = -1;

    Section currentSection = Section::NONE;
    std::string line;

    while (std::getline(in, line)){
        std::string trimmed = trim(line);

        // Ignora linhas vazias
        if (trimmed.empty()) continue;

        if(trimmed == "EOF") break;

        if(trimmed == "NODE_COORD_SECTION"){
            if(instance.dimension < 0) {
                throw std::runtime_error("Dimension não foi inicializado (while) " + filepath);
            }
            currentSection = Section::NODE_COORD;
            continue;
        }

        if(trimmed == "DEMAND_SECTION"){
            currentSection = Section::DEMAND;
            continue;
        }

        if(trimmed == "DEPOT_SECTION"){
            currentSection = Section::DEPOT;
            continue;
        }

        // Se estamos dentro de alguma seção de dados, delega o parsing
        if (currentSection == Section::NODE_COORD) {
            std::istringstream iss(trimmed);
            int id;
            double x, y;
            iss >> id >> x >> y;

            instance.nodes[id - 1] = {id, x, y, 0};

            continue;
        }

        if (currentSection == Section::DEMAND) {
            std::istringstream iss(trimmed);
            int id;
            double demand;
            iss >> id >> demand;

            instance.nodes[id - 1].demand = demand;
            continue;
        }

        // Se chegou aqui currentSection == NONE estamos no cabeçalho
        auto colonPos = trimmed.find(':');
        if (colonPos == std::string::npos) {
            // Linha de cabeçalho sem ':' -- pode ignorar ou lançar erro
            continue;
        }

        std::string key = trim(trimmed.substr(0, colonPos));
        std::string value = trim(trimmed.substr(colonPos + 1));

        // comparar 'key' com "NAME", "DIMENSION", "CAPACITY"
        if(key == "NAME") instance.name = value;
        else if(key == "DIMENSION") {
            instance.dimension = std::stoi(value);
            instance.nodes.resize(instance.dimension);
        }
        else if(key == "CAPACITY") instance.capacity = std::stoi(value);
        else if(key == "EDGE_WEIGHT_TYPE") instance.type = value;
    }

    // validações finais antes de retornar
    if(instance.dimension < 0){
        throw std::runtime_error("Dimension não foi inicializado (fora while) " + filepath);
    }
    if(instance.capacity < 0){
        throw std::runtime_error("Capacity menor que zero " + filepath);
    }
    if(instance.nodes.size() != static_cast<size_t>(instance.dimension)){
        throw std::runtime_error("Nodes tamanho errado " + filepath);
    }

    return instance;
}

double distance(const Node& a, const Node& b, const std::string& edgeWeightType){
    if (edgeWeightType != "EUC_2D") {
        throw std::runtime_error("Type não suportado " + edgeWeightType);
    }

    double dx = a.x - b.x;
    double dy = a.y - b.y;

    double result = std::sqrt((dx*dx) + (dy*dy));

    return std::round(result);
}

std::vector<std::vector<double>> buildDistanceMatrix(const CVRPInstance& instance){
    size_t n = instance.dimension;

    std::vector<std::vector<double>> matrix(n, std::vector<double>(n, 0.0));

    for(size_t i = 0; i < n; i++){
        for(size_t j = i + 1; j < n; j++){
            matrix[i][j] = distance(instance.nodes[i], instance.nodes[j], instance.type);
            matrix[j][i] = matrix[i][j];
        }
    }

    return matrix;
}   

CVRPInstance& loadInstance(const std::string& filepath){
    CVRPInstance& instance = readInstance(filepath);

    instance.distanceMatrix = buildDistanceMatrix(instance);
    instance.adjacencyList = buildAdjacencyLists(instance);

    return instance;
}

std::vector<std::vector<int>> buildAdjacencyLists(const CVRPInstance& instance){
    size_t n = instance.dimension;

    std::vector<std::vector<int>> adjacencyList(n, std::vector<int>(n-1));

    // deposito não entra, logo começa no 1
    for(size_t i = 1; i < n; i++){
        for(size_t j = 1; j < n; j++){
            adjacencyList[i][j - 1] = j;
        }
        std::sort(adjacencyList[i].begin(), adjacencyList[i].end(),[&](int a, int b) {
            return instance.distanceMatrix[i][a] < instance.distanceMatrix[i][b];
        });
    }
    return adjacencyList;
}