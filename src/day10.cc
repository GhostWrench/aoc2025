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
    enum class VarType {
        PIVOT,
        FREE,
        COUNT,
    };
    struct DependentEq {
        int base;
        int scale;
        std::vector<int> coeffs;
    };
    struct Variable {
        std::array<int, 2> range;
        VarType var_type;
        int pivot_row;
        DependentEq eq;
    };
    std::vector<Variable> variables;
    for (size_t ii=0; ii<n; ii++) {
        Variable variable = {
            .range = {0, std::numeric_limits<int>::max()},
            .var_type = VarType::PIVOT,
            .pivot_row = -1,
            .eq {
                .base = 0,
                .scale = 0,
                .coeffs = {}
            }
        };
        variables.push_back(variable);
    }

    // Find an initial range for each variable while the matrix is positive
    for (size_t ii=0; ii<m; ii++) {
        for (size_t jj=0; jj<n; jj++) {
            if (system[ii][jj] == 0) continue;
            int max = system[ii][n] / system[ii][jj];
            if (max < variables[jj].range[1]) {
                variables[jj].range[1] = max;
            }
        }
    }

    // Find the pivot rows and do row reduction
    std::unordered_set<int> pivot_rows;
    for (size_t pivot_search_col=0; pivot_search_col<n; pivot_search_col++) {
        for (size_t row=0; row<m; row++) {
            if (pivot_rows.contains(row)) {
                // When looking for pivots, skip ones that have already been found
                continue;
            }
            if (system[row][pivot_search_col] != 0) {
                // Pivot Found, perform row reduction
                variables[pivot_search_col].pivot_row = row;
                variables[pivot_search_col].var_type = VarType::PIVOT;
                // Find the lowest common multiple, the pivot row and operation row scaler.
                for (size_t oprow=0; oprow<m; oprow++) {
                    // Skip rows that already have 0 in the pivot column
                    if (system[oprow][pivot_search_col] == 0) continue;
                    // Skip current pivot row
                    if (oprow == row) continue;
                    // Find the least common multiple and the value to scale each row by
                    int lcm = std::lcm(system[row][pivot_search_col], system[oprow][pivot_search_col]);
                    int pivot_scale = lcm / system[row][pivot_search_col];
                    int row_scale = lcm / system[oprow][pivot_search_col];
                    // Scale the op row
                    for (size_t opcol=0; opcol<n+1; opcol++) {
                        system[oprow][opcol] *= row_scale;
                    }
                    // Subtract the pivot row
                    for (size_t opcol=0; opcol<n+1; opcol++) {
                        system[oprow][opcol] -= system[row][opcol] * pivot_scale;
                    }
                    // If the last column is negative, multiply the row by -1
                    if (system[oprow][n] < 0) {
                        for (size_t opcol=0; opcol<n+1; opcol++) {
                            system[oprow][opcol] *= -1;
                        }
                    }
                }
                pivot_rows.insert(row);
#ifdef DEBUG
                disp_matrix(system);
#endif
                // break out of two top two loops
                goto next_col;
            }
        }
        variables[pivot_search_col].var_type = VarType::FREE;
        //}
next_col:
    }

    // Create an equation for all of the pivot variables
    for (size_t vi=0; vi<n; vi++) {
        if (variables[vi].var_type == VarType::PIVOT) {
            variables[vi].eq.base = system[variables[vi].pivot_row][n];
            variables[vi].eq.scale = system[variables[vi].pivot_row][vi];
            for (size_t col=0; col<n; col++) {
                if (variables[col].var_type == VarType::FREE) {
                    variables[vi].eq.coeffs.push_back(-system[variables[vi].pivot_row][col]);
                }
            }
        }
    }

    // Attempt to refine the ranges again for positive rows in the new matrix
    for (size_t row=0; row<m; row++) {
        std::vector<int> update_vars;
        bool all_positive = true;
        for (size_t col=0; col<n; col++) {
            if (system[row][col] == 0) continue;
            if (system[row][col] < 0) {
                all_positive = false;
                break;
            }
            update_vars.push_back(col);
        }
        if (all_positive) {
            for (int var_idx : update_vars) {
                int max = system[row][n] / system[row][var_idx];
                if (max < variables[var_idx].range[1]) {
                    variables[var_idx].range[1] = max;
                }
            }
        }
    }

#ifdef DEBUG
    disp_matrix(system);
    std::cout << "\n" << "Variables:\n";
    for (size_t ii=0; ii<variables.size(); ii++) {
        std::cout << ii << ": ";
        if (variables[ii].var_type == VarType::FREE) {
            std::cout << "[" << variables[ii].range[0] << "," << variables[ii].range[1] << "]" << "\n";
        }
        else {
            size_t fvar_idx = 0;
            std::cout << "( " << variables[ii].eq.base << " ";
            for (size_t vi=0; vi<n; vi++) {
                if (variables[vi].var_type == VarType::FREE) {
                    int coeff = variables[ii].eq.coeffs[fvar_idx];
                    if (coeff > 0) {
                        std::cout << " + " << coeff << "x" << vi;
                    }
                    else {
                        std::cout << " - " << -coeff << "x" << vi;
                    }
                    fvar_idx++;
                }
            }
            std::cout << " ) / " << variables[ii].eq.scale << "\n";
        }
    }
    std::cout << "\n";
    std::cout << std::endl;
#endif

    // Loop through the possible variables and check if they are a solution
    struct IterCounter {
        size_t var_idx;
        size_t count;
    };
    std::vector<IterCounter> iter_count;
    for (size_t jj=0; jj<n; jj++) {
        if (variables[jj].var_type == VarType::FREE) {
            IterCounter ic = {
                .var_idx = jj,
                .count = static_cast<size_t>(variables[jj].range[0]),
            };
            iter_count.push_back(ic);
        }
    }
    bool iter_done = false;
    int min_presses = std::numeric_limits<int>::max();

#ifdef DEBUG
    // Calculate the required number of iterations
    uint64_t dims = 1;
    for (size_t col=0; col<n; col++) {
        if (variables[col].var_type == VarType::FREE) {
            dims *= static_cast<uint64_t>(variables[col].range[1] - variables[col].range[0] + 1);
        }
    }
    std::cout << "Dimensions: " << dims << std::endl;
#endif

    while (!iter_done) {
        std::vector<int> guess(n, 0);
        std::vector<int> result;
        bool is_equal = true;
        int sum = 0;
        // Iterate through free variables and assign a value from the range
        for (size_t ci=0; ci<iter_count.size(); ci++) {
            guess[iter_count[ci].var_idx] = iter_count[ci].count;
        }
        // Calculate the pivot variables
        bool positive_guess = true;
        for (size_t vi=0; vi<variables.size(); vi++) {
            if (variables[vi].var_type == VarType::PIVOT) {
                int value = variables[vi].eq.base;
                for (size_t fi=0; fi<iter_count.size(); fi++) {
                    value += guess[iter_count[fi].var_idx] * variables[vi].eq.coeffs[fi];
                }
                value /= variables[vi].eq.scale;
                if (value < 0) positive_guess = false;
                guess[vi] = value;
            }
        }
        if (!positive_guess) {
            goto update_iter;
        }

#ifdef DEBUG
        std::cout << "[";
        for (size_t vi=0; vi<guess.size(); vi++) {
            std::cout << guess[vi] << ",";
        }
        std::cout << "] => ";
#endif

        // Check if the guess works
        for (size_t ii=0; ii<m; ii++) {
            int row_total = 0;
            for (size_t jj=0; jj<n; jj++) {
                row_total += guess[jj] * system[ii][jj];
            }
            result.push_back(row_total);
        }

#ifdef DEBUG
        std::cout << "[";
        for (size_t vi=0; vi<result.size(); vi++) {
            std::cout << result[vi] << ",";
        }
        std::cout << "] ";
#endif

        for (size_t jj=0; jj<n; jj++) {
            sum += guess[jj];
        }
        for (size_t ii=0; ii<m; ii++) {
            if (result[ii] != system[ii][n]) {
                is_equal = false;
                break;
            }
        }

#ifdef DEBUG
        if (is_equal && positive_guess) {
            std::cout << "X (" << sum << " presses)";
        }
        std::cout << std::endl;
#endif

        if (is_equal && positive_guess && sum < min_presses) {
            min_presses = sum;
        }

        // Update the iteration
update_iter:
        bool carry = true;
        size_t iter_idx = 0;
        while (carry) {
            if (iter_count.size() == 0) {
                iter_done = true;
                break;
            }
            int count = static_cast<int>(iter_count[iter_idx].count);
            if (count >= variables[iter_count[iter_idx].var_idx].range[1]) {
                iter_count[iter_idx].count = variables[iter_count[iter_idx].var_idx].range[0];
                carry = true;
                iter_idx++;
                if (iter_idx >= iter_count.size()) {
                    iter_done = true;
                    break;
                }
            }
            else {
                carry = false;
                iter_count[iter_idx].count++;
            }
        }
    }

    return min_presses;
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
