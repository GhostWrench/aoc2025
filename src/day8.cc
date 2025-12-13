#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
//#include <list>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

using Point3D = std::array<double, 3>;

std::vector<Point3D> read_input(const std::string& fname) {
    // Read the file
    std::ifstream ifile(fname);
    std::string line;
    std::vector<std::array<double, 3>> output;
    while (std::getline(ifile, line)) {
        if (line.empty()) break;
        std::stringstream ss(line);
        output.push_back({0, 0, 0});
        for (size_t ii=0; ii<3; ii++){
            std::string cell;
            std::getline(ss, cell, ',');
            output[output.size()-1][ii] = std::stod(cell);
        }
    }
    ifile.close();
    return output;
}

double point_distance(const Point3D& p1, const Point3D& p2) {
    return std::sqrt(
          std::pow(p2[0] - p1[0], 2.0) 
        + std::pow(p2[1] - p1[1], 2.0) 
        + std::pow(p2[2] - p1[2], 2.0)
    );
}

uint64_t do_n_connections(const std::vector<Point3D>& nodes, const size_t n) {
    using IndexPair = std::array<size_t, 2>;
    using DistanceInfo = std::tuple<double, IndexPair>;
    // Get a list of distances
    std::vector<DistanceInfo> distances;
    for (size_t ii=0; ii<nodes.size()-1; ii++) {
        for (size_t jj=(ii+1); jj<nodes.size(); jj++) {
            double dist = point_distance(nodes[ii], nodes[jj]);
            DistanceInfo value = {
                dist, {ii, jj}
            };
            distances.push_back(value);
        }
    }
    // Sort the list
    std::sort(
        distances.begin(),
        distances.end(),
        [](const DistanceInfo& a, const DistanceInfo& b) {
            return std::get<0>(a) < std::get<0>(b);
        }
    );

    // Loop through the first 'n' distances and make circuits
    struct Circuit {
        double wire_length;
        std::unordered_set<size_t> node_idxs;
    };
    std::vector<Circuit> circuits;
    for (size_t ii=0; ii<n; ii++) {
        double pair_dist = std::get<0>(distances[ii]);
        IndexPair pair = std::get<1>(distances[ii]);
        std::vector<int> circuit_idxs = {-1, -1};
        size_t circuit_idx = 0;
        // Find if the indexs are alread in a circuit
        for (const Circuit& circuit : circuits) {
            for (size_t jj=0; jj<2; jj++) {
                //if (circuit_idxs[jj] == -1 && circuit.node_idxs.find(pair[jj]) != circuit.node_idxs.end()) {
                if (circuit_idxs[jj] == -1 && circuit.node_idxs.contains(pair[jj])) {
                    circuit_idxs[jj] = static_cast<int>(circuit_idx);
                }
            }
            if (circuit_idxs[0] >= 0 && circuit_idxs[1] >= 0) {
                break;
            }
            circuit_idx++;
        }
        // Put the indexs into the list
        if (circuit_idxs[0] == -1 && circuit_idxs[1] == -1) {
            Circuit new_circuit = {
                .wire_length = pair_dist,
                .node_idxs = {pair[0], pair[1]},
            };
            circuits.push_back(new_circuit);
        }
        else if (circuit_idxs[0] == circuit_idxs[1]) {
            // Do nothing, already connected
        }
        else if (circuit_idxs[0] >= 0 && circuit_idxs[1] == -1) {
            circuits[circuit_idxs[0]].node_idxs.insert(pair[1]);
            circuits[circuit_idxs[0]].wire_length += pair_dist;
        }
        else if (circuit_idxs[0] == -1 && circuit_idxs[1] >= 0) {
            circuits[circuit_idxs[1]].node_idxs.insert(pair[0]);
            circuits[circuit_idxs[1]].wire_length += pair_dist;
        }
        else if (circuit_idxs[0] >= 0 && circuit_idxs[1] >= 0) {
            circuits[circuit_idxs[0]].node_idxs.insert(
                circuits[circuit_idxs[1]].node_idxs.begin(),
                circuits[circuit_idxs[1]].node_idxs.end()
            );
            circuits.erase(circuits.begin() + circuit_idxs[1]);
        }
    }

    // Sort the final circuit sizes
    std::sort(
        circuits.begin(),
        circuits.end(),
        [](const Circuit& a, const Circuit& b) {
            return a.node_idxs.size() > b.node_idxs.size();
        }
    );

    // Calculate the product of the three biggest circuits
    uint64_t product = 1;
    for (size_t ii=0; ii<3; ii++) {
        product *= circuits[ii].node_idxs.size();
    }
    return product;
}

int main(int argc, char **argv) {

    // Check inputs
    if (argc != 2) {
        std::cout << "Input filename must be provided" << std::endl;
        return EXIT_FAILURE;
    }

    // Read the input file
    std::vector<Point3D> junction_boxes = read_input(argv[1]);

    // Process the inputs
    uint64_t product = do_n_connections(junction_boxes, 1000);

    // Output the results
    std::cout << "Circuit product: " << product << std::endl;

    return EXIT_SUCCESS;
}
