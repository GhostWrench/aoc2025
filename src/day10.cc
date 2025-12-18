#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

struct MachineInfo {
    uint16_t indicator_diagram;
    std::vector<uint16_t> wiring_schematics;
    std::vector<uint16_t> joltage_requirements;
};

std::vector<MachineInfo> read_input(const std::string& fname) {
    // Read the file
    std::ifstream ifile(fname);
    std::string line;
    std::vector<MachineInfo> machines;
    while (std::getline(ifile, line)) {
        if (line.empty()) break;
        std::stringstream ss(line);
        std::string segment;
        MachineInfo machine;
        while(std::getline(ss, segment, ' ')) {
            if (segment[0] == '[') {
                uint16_t indicator_diagram = 0;
                for (int idx=0; idx<16; idx++) {
                    char indicator = segment[idx+1];
                    if (indicator == '#') {
                        indicator_diagram |= (1 << idx);
                    }
                    else if (indicator == ']') {
                        break;
                    }
                }
                machine.indicator_diagram = indicator_diagram;
            }
            else if (segment[0] == '(') {
                std::stringstream ssi(segment.substr(1, segment.length()-2));
                std::string wire_str;
                uint16_t wiring_schematic = 0;
                while (std::getline(ssi, wire_str, ',')) {
                    uint16_t wire_idx = std::stoul(wire_str);
                    wiring_schematic |= (1 << wire_idx);
                }
                machine.wiring_schematics.push_back(wiring_schematic);
            }
            else if (segment[0] == '{') {
                std::stringstream ssi(segment.substr(1, segment.length()-2));
                std::string joltage_requirement_str;
                while (std::getline(ssi, joltage_requirement_str, ',')) {
                    machine.joltage_requirements.push_back(std::stoul(joltage_requirement_str));
                }
            }
        }
        machines.push_back(machine);
    }
    ifile.close();
    return machines;
}

size_t count_min_presses(const MachineInfo& machines) {
    // Store all the routes in a map
    std::unordered_set<uint16_t> prev_states;
    std::unordered_set<uint16_t> next_states;
    size_t current_depth = 0;
    if (machines.indicator_diagram == 0) {
        return current_depth;
    }
    prev_states.insert(0);
    while (true) {
        current_depth++;
        next_states.clear();
        for (const uint16_t& state : prev_states) {
            for (size_t ii=0; ii<machines.wiring_schematics.size(); ii++) {
                uint16_t new_state = state ^ machines.wiring_schematics[ii];
                // Check if we have found the solution
                if (new_state == machines.indicator_diagram) {
                    return current_depth;
                }
                next_states.insert(new_state);
            }
        }
        prev_states.swap(next_states);
    }
}

size_t count_all_presses(const std::vector<MachineInfo>& machines) {
    size_t total = 0;
    for (const MachineInfo& machine : machines) {
        total += count_min_presses(machine);
    }
    return total;
}

#ifdef DEBUG
void disp_matrix(const std::vector<std::vector<int>>& matrix) {
    for (size_t ii=0; ii<matrix.size(); ii++) {
        for (size_t jj=0; jj<matrix[ii].size(); jj++) {
            std::cout << " " << std::setw(4) << matrix[ii][jj];
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}
#endif

size_t count_joltage_presses(const MachineInfo& machine) {
    
    // create the augmented matrix for the system
    size_t m = machine.joltage_requirements.size();
    size_t n = machine.wiring_schematics.size();
    std::vector<std::vector<int>> system;
    system.reserve(m);
    for (size_t ii=0; ii<m; ii++) {
        std::vector<int> row;
        row.reserve(n + 1);
        for (size_t jj=0; jj<n; jj++) {
            row.push_back((machine.wiring_schematics[jj] & (1 << ii)) >> ii);
        }
        row.push_back(machine.joltage_requirements[ii]);
        system.push_back(row);
    }

#ifdef DEBUG
    disp_matrix(system);
#endif

    // Create a vector with information about the vectors
    enum class PivotType {
        INDEPENDENT,
        DEPENDENT,
        COUNT,
    };
    struct DependentEq {
        int base;
        int scale;
        std::vector<int> coeffs;
    };
    struct Variable {
        std::array<int, 2> range;
        PivotType pivot_type;
        int pivot_row;
        bool solved;
        DependentEq eq;
    };
    std::vector<Variable> variables;
    for (size_t ii=0; ii<n; ii++) {
        Variable variable = {
            .range = {-1, std::numeric_limits<int>::max()},
            .pivot_type = PivotType::INDEPENDENT,
            .pivot_row = -1,
            .solved = false,
            .eq {
                .base = 0,
                .scale = 0,
                .coeffs = {}
            }
        };
        variables.push_back(variable);
    }

    // Find the pivot rows and do row reduction
    std::unordered_set<int> independent_pivot_rows;
    //size_t matrix_rank = 0;
    for (size_t pivot_search_col=0; pivot_search_col<n; pivot_search_col++) {
        //int pivot_row = -1;
        //for (PivotType pivot_type=PivotType::INDEPENDENT; pivot_type<PivotType::COUNT; pivot_type++) {
        for (PivotType pivot_type : {PivotType::INDEPENDENT, PivotType::DEPENDENT}) {
            for (size_t ii=0; ii<m; ii++) {
                if (pivot_type == PivotType::INDEPENDENT && independent_pivot_rows.contains(ii)) {
                    // When looking for independent pivots, skip ones that have already been found
                    continue;
                }
                if (system[ii][pivot_search_col] != 0) {
                    // Pivot Found, perform row reduction
                    variables[pivot_search_col].pivot_row = ii;
                    variables[pivot_search_col].pivot_type = pivot_type;
                    // Find the lowest common multiple, the pivot row and operation row scaler.
                    for (size_t iii=0; iii<m; iii++) {
                        // Skip rows that already have 0 in the pivot column
                        if (system[iii][pivot_search_col] == 0) continue;
                        // Skip current pivot row
                        if (ii == iii) continue;
                        // Skip other existing pivot rows
                        if (pivot_type == PivotType::INDEPENDENT && independent_pivot_rows.contains(iii)) {
                            continue;
                        }
                        // Find the least common multiple and the value to scale each row by
                        int lcm = std::lcm(system[ii][pivot_search_col], system[iii][pivot_search_col]);
                        int pivot_scale = lcm / system[ii][pivot_search_col];
                        int row_scale = lcm / system[iii][pivot_search_col];
                        // Scale the op row
                        for (size_t jjj=0; jjj<n+1; jjj++) {
                            system[iii][jjj] *= row_scale;
                        }
                        // Subtract the pivot row
                        for (size_t jjj=0; jjj<n+1; jjj++) {
                            system[iii][jjj] -= system[ii][jjj] * pivot_scale;
                        }
                        // If the last column is negative, multiply the row by -1
                        if (system[iii][n] < 0) {
                            for (size_t jjj=0; jjj<n+1; jjj++) {
                                system[iii][jjj] *= -1;
                            }
                        }
                    }
                    if (pivot_type == PivotType::INDEPENDENT) {
                        independent_pivot_rows.insert(ii);
                    }
#ifdef DEBUG
                    disp_matrix(system);
#endif
                    // break out of two top two loops
                    goto next_col;
                }
            }
        }
next_col:
    }

    // Create an equation for all of the dependent variables
    for (size_t jj=0; jj<n; jj++) {
        if (variables[jj].pivot_type == PivotType::DEPENDENT) {
            variables[jj].eq.base = system[variables[jj].pivot_row][n];
            variables[jj].eq.scale = system[variables[jj].pivot_row][jj];
            for (size_t jjj=0; jjj<n; jjj++) {
                if (jjj == jj) {
                    variables[jj].eq.coeffs.push_back(0);
                }
                else {
                    variables[jj].eq.coeffs.push_back(-system[variables[jj].pivot_row][jjj]);
                }
            }
        }
    }

    // Find the maximum range for each variable
    bool ranges_updated = true;
    while (ranges_updated) {
        ranges_updated = false;
        for (size_t ii=0; ii<m; ii++) {
            int non_zero_count = 0;
            size_t non_zero_idx = 0;
            int undefined_range_count = 0;
            size_t undefined_range_idx = 0;
            bool all_positive = true;
            for (size_t jj=0; jj<n; jj++) {
                if (system[ii][jj] != 0) {
                    non_zero_count++;
                    non_zero_idx = jj;
                    if (variables[jj].range[0] == -1) {
                        undefined_range_count++;
                        undefined_range_idx = jj;
                    }
                }
                if (system[ii][jj] < 0) {
                    all_positive = false;
                }
            }
            if (non_zero_count == 1 && !variables[non_zero_idx].solved) {
                // Best case, variable can be solved
                int value = system[ii][n] / system[ii][non_zero_idx];
                variables[non_zero_idx].range = {value, value};
                variables[non_zero_idx].solved = true;
                ranges_updated = true;
            }
            else if (undefined_range_count == 1 && !variables[undefined_range_idx].solved) {
                // Second best case, a range can be solved for
                std::array<int, 2> new_range({system[ii][n], system[ii][n]});
                int scale = system[ii][undefined_range_idx];
                for (size_t jj=0; jj<n; jj++) {
                    if (jj == undefined_range_idx) continue;
                    int value = system[ii][jj];
                    if (value == 0) continue;
                    std::array<int, 2> range = variables[jj].range;
                    if (value > 0) {
                        new_range[0] -= value * range[1];
                        new_range[1] -= value * range[0];
                    }
                    else
                    {
                        new_range[0] -= value * range[0];
                        new_range[1] -= value * range[1];
                    }
                }
                new_range[0] = new_range[0] / scale;
                new_range[1] = new_range[1] / scale;
                if (new_range[0] < 0) {
                    new_range[0] = 0;
                }
                if (new_range[0] > variables[undefined_range_idx].range[0]) {
                    variables[undefined_range_idx].range[0] = new_range[0];
                    ranges_updated = true;
                }
                if (new_range[1] < variables[undefined_range_idx].range[1]) {
                    variables[undefined_range_idx].range[1] = new_range[1];
                    ranges_updated = true;
                }
                if (variables[undefined_range_idx].range[0] == variables[undefined_range_idx].range[1]) {
                    variables[undefined_range_idx].solved = true;
                }
            }
            else if (all_positive) {
                // Last case, can infer a broad range from last value
                for (size_t jj=0; jj<n; jj++) {
                    if (variables[jj].solved) continue;
                    int value = system[ii][jj];
                    if (value == 0) continue;
                    if (0 > variables[jj].range[0]) {
                        variables[jj].range[0] = 0;
                        ranges_updated = true;
                    }
                    int max = system[ii][n] / value;
                    if (max < variables[jj].range[1]) {
                        variables[jj].range[1] = max;
                        ranges_updated = true;
                    }
                }
            }
        }
    }

//#ifdef DEBUG
    disp_matrix(system);
    std::cout << "\n" << "Variables:\n";
    for (size_t ii=0; ii<variables.size(); ii++) {
        std::cout << ii << ": ";
        if (variables[ii].pivot_type == PivotType::INDEPENDENT) {
            std::cout << "[" << variables[ii].range[0] << "," << variables[ii].range[1] << "]" << "\n";
        }
        else {
            std::cout << "( " << variables[ii].eq.base << " ";
            for (size_t vi=0; vi<variables[ii].eq.coeffs.size(); vi++) {
                int coeff = variables[ii].eq.coeffs[vi];
                if (coeff == 0) {
                    continue;
                } 
                else if (coeff > 0) {
                    std::cout << " + " << coeff << "x" << vi;
                }
                else {
                    std::cout << " - " << -coeff << "x" << vi;
                }
            }
            std::cout << " ) / " << variables[ii].eq.scale << "\n";
        }
    }
    std::cout << "\n";
    std::cout << std::endl;
//#endif

    return 0;
}

size_t count_all_joltage_presses(const std::vector<MachineInfo>& machines) {
    size_t total = 0;
    for (const MachineInfo& machine : machines) {
        total += count_joltage_presses(machine);
    }
    return total;
}

int main(int argc, char **argv) {

    // Check inputs
    if (argc != 2) {
        std::cout << "Input filename must be provided" << std::endl;
        return EXIT_FAILURE;
    }

    // Read the input file
    std::vector<MachineInfo> machines = read_input(argv[1]);

    // Process the inputs
    size_t min_presses = count_all_presses(machines);
    size_t joltage_presses = count_all_joltage_presses(machines);

    // Output the results
    std::cout << "Min Button Presses : " << min_presses << std::endl;
    std::cout << "Min Joltage Presses: " << joltage_presses << std::endl;

    return EXIT_SUCCESS;
}
