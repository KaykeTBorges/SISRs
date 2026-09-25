#ifndef CVRP_INSTANCE_H
#define CVRP_INSTANCE_H

#include <string>
#include <vector>

struct Node {
    int id;
    double x, y;
    int demand;
};

class CVRPInstance {
public:
    static CVRPInstance& getInstance();

    std::string name;
    int dimension = -1;
    int capacity = -1;
    std::string type;
        
    std::vector<Node> nodes;
    std::vector<std::vector<double>> distanceMatrix;
    std::vector<std::vector<int>> adjacencyList;
};

    CVRPInstance& readInstance(const std::string& filepath);

    double distance(const Node& a, const Node& b, const std::string& edgeWeightType);

    std::vector<std::vector<double>> buildDistanceMatrix(const CVRPInstance& instance);
    std::vector<std::vector<int>> buildAdjacencyLists(const CVRPInstance& instance);
    CVRPInstance& loadInstance(const std::string& filepath);



#endif // CVRP_INSTANCE_H