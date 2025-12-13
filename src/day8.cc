#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

using Point3D = std::array<uint64_t, 3>;

std::vector<Point3D> read_input(const std::string& fname) {
    // Read the file
    std::ifstream ifile(fname);
    std::string line;
    std::vector<std::array<uint64_t, 3>> output;
    while (std::getline(ifile, line)) {
        if (line.empty()) break;
        std::stringstream ss(line);
        output.push_back({0, 0, 0});
        for (size_t ii=0; ii<3; ii++){
            std::string cell;
            std::getline(ss, cell, ',');
            output[output.size()-1][ii] = std::stoull(cell);
        }
    }
    ifile.close();
    return output;
}

uint64_t point_distance(const Point3D& p1, const Point3D& p2) {
        uint64_t dx = p2[0] - p1[0];
        uint64_t dy = p2[1] - p1[1];
        uint64_t dz = p2[2] - p1[2];
        return dx*dx + dy*dy + dz*dz;
}

using IndexPair = std::array<size_t, 2>;
using DistanceInfo = std::tuple<uint64_t, IndexPair>;

std::vector<DistanceInfo> get_distances(const std::vector<Point3D>& nodes) {
    // Calculate all the distances
    std::vector<DistanceInfo> distances;
    for (size_t ii=0; ii<nodes.size()-1; ii++) {
        for (size_t jj=(ii+1); jj<nodes.size(); jj++) {
            uint64_t dist = point_distance(nodes[ii], nodes[jj]);
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
    return distances;
}

using Circuit = std::unordered_set<size_t>;

void make_connection(IndexPair pair, std::vector<Circuit>& circuits) {
    std::vector<int> circuit_idxs = {-1, -1};
    size_t circuit_idx = 0;
    // Find if the indexs are alread in a circuit
    for (const Circuit& circuit : circuits) {
        for (size_t jj=0; jj<2; jj++) {
            if (circuit_idxs[jj] == -1 && circuit.contains(pair[jj])) {
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
        Circuit new_circuit = {pair[0], pair[1]};
        circuits.push_back(new_circuit);
    }
    else if (circuit_idxs[0] == circuit_idxs[1]) {
        // Do nothing, already connected
    }
    else if (circuit_idxs[0] >= 0 && circuit_idxs[1] == -1) {
        circuits[circuit_idxs[0]].insert(pair[1]);
    }
    else if (circuit_idxs[0] == -1 && circuit_idxs[1] >= 0) {
        circuits[circuit_idxs[1]].insert(pair[0]);
    }
    else if (circuit_idxs[0] >= 0 && circuit_idxs[1] >= 0) {
        circuits[circuit_idxs[0]].insert(
            circuits[circuit_idxs[1]].begin(),
            circuits[circuit_idxs[1]].end()
        );
        circuits.erase(circuits.begin() + circuit_idxs[1]);
    }
}

uint64_t do_n_connections(const std::vector<DistanceInfo>& distances, const size_t n) {

    // Loop through the first 'n' distances and make circuits
    std::vector<Circuit> circuits;
    for (size_t ii=0; ii<n; ii++) {
        IndexPair pair = std::get<1>(distances[ii]);
        make_connection(pair, circuits);
    }

    // Sort the final circuit sizes
    std::sort(
        circuits.begin(),
        circuits.end(),
        [](const Circuit& a, const Circuit& b) {
            return a.size() > b.size();
        }
    );

    // Calculate the product of the three biggest circuits
    uint64_t product = 1;
    for (size_t ii=0; ii<3; ii++) {
        product *= circuits[ii].size();
    }
    return product;
}

uint64_t find_last_connection(const std::vector<Point3D>& junctions, const std::vector<DistanceInfo>& dist_info) {
    std::unordered_set<size_t> connected_nodes;
    std::vector<Circuit> circuits;
    size_t x1 = 0;
    size_t x2 = 0;
    for (size_t ii=0; ii<dist_info.size(); ii++) {
        IndexPair pair = std::get<1>(dist_info[ii]);
        make_connection(pair, circuits);
        connected_nodes.insert(
            pair.begin(), pair.end()
        );
        if (connected_nodes.size() >= junctions.size() && circuits.size() == 1) {
            x1 = junctions[pair[0]][0];
            x2 = junctions[pair[1]][0];
            break;
        }
    }
    return x1 * x2;
}

int main(int argc, char **argv) {

    // Check inputs
    if (argc != 2) {
        std::cout << "Input filename must be provided" << std::endl;
        return EXIT_FAILURE;
    }

    // Read the input file
    std::vector<Point3D> junction_boxes = read_input(argv[1]);
    std::vector<DistanceInfo> distances = get_distances(junction_boxes);

    // Process the inputs
    uint64_t product = do_n_connections(distances, 1000);
    uint64_t wall_distance = find_last_connection(junction_boxes, distances);

    // Output the results
    std::cout << "Circuit product: " << product << std::endl;
    std::cout << "Wall Distance: " << wall_distance << std::endl;

    return EXIT_SUCCESS;
}
