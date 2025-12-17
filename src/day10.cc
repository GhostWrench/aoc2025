#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
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

    // Find the pivot rows and do row reduction
    std::vector<int> button_pivots;
    size_t pivot_search_col = 0;
    size_t matrix_rank = 0;
    while (pivot_search_col < n) {
        int pivot_row = -1;
        for (size_t ii=0; ii<m; ii++) {
            // Skip if alread a pivot row
            auto it = std::find(button_pivots.begin(), button_pivots.end(), ii);
            if (it != button_pivots.end())
            {
                continue;
            }
            if (system[ii][pivot_search_col] != 0) {
                // Pivot found
                pivot_row = ii;
                // Ensure the row is positive
                //if (system[ii][pivot_search_col] < 0) {
                //    for (size_t jjj=0; jjj<n; jjj++) {
                //        system[ii][jjj] *= -1;
                //    }
                //}
                // Do row reduction on all other rows
                // Find the lowest common multiple, the pivot row and operation row scaler.
                for (size_t iii=0; iii<m; iii++) {
                    // Skip rows that already have 0 in the pivot column
                    if (system[iii][pivot_search_col] == 0) continue;
                    // Skip current pivot row
                    if (ii == iii) continue;
                    // Skip other existing pivot rows
                    auto it = std::find(button_pivots.begin(), button_pivots.end(), iii);
                    if (it != button_pivots.end()) continue;
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
                }
#ifdef DEBUG
                disp_matrix(system);
#endif
                matrix_rank++;
                break;
            }
        }
        // If we get here a pivot wasn't found
        button_pivots.push_back(pivot_row);
        pivot_search_col++;
    }

#ifdef DEBUG
    disp_matrix(system);
    std::cout << "\n" << "Pivots: {";
    for (size_t ii=0; ii<button_pivots.size(); ii++) {
        std::cout << button_pivots[ii] << ",";
    }
    std::cout << "}" << "\n";
    std::cout << std::endl;
#endif

    // Attempt to limit the range of of the searches by looking for purely 
    // positive rows or rows with a single value
    int max_range = 0;
    for (size_t ii=0; ii<machine.joltage_requirements.size(); ii++) {
        max_range += machine.joltage_requirements[ii];
    }
    using SearchRange = std::array<int, 2>;
    std::vector<SearchRange> search_ranges;
    search_ranges.reserve(n);
    for (size_t ii=0; ii<n; ii++) {
        SearchRange range = {-1, max_range};
        search_ranges.push_back(range);
    }
    // Look for easy ranges to determine in all positive rows and single value 
    // rows
    for (size_t ii=0; ii<m; ii++) {
        int non_zero_count = 0;
        size_t non_zero_index = 0;
        bool all_positive = true;
        for (size_t jj=0; jj<n; jj++) {
            if (system[ii][jj] < 0) {
                all_positive = false;
            }
            if (system[ii][jj] != 0) {
                non_zero_count += 1;
                non_zero_index = jj;
            }
        }
        if (non_zero_count == 1) {
            int value = system[ii][n] / system[ii][non_zero_index];
            if (value > search_ranges[non_zero_index][0]) {
                search_ranges[non_zero_index][0] = value;
            }
            if (value < search_ranges[non_zero_index][1]) {
                search_ranges[non_zero_index][1] = value;
            }
        }
        else if (all_positive && non_zero_count > 0) {
            for (size_t jj=0; jj<n; jj++) {
                // Skip non-pivot rows
                if (system[ii][jj] == 0) continue;
                int new_range = system[ii][n] / system[ii][jj];
                if (new_range < search_ranges[jj][1]) {
                    search_ranges[jj][1] = new_range;
                }
                if (0 > search_ranges[jj][0]) {
                    search_ranges[jj][0] = 0;
                }
            }
        }
    }

    // Now loop through and attempt to find places where only one range is 
    // undefined
    bool ranges_updated;
    do {
        ranges_updated = false;
        for (size_t ii=0; ii<m; ii++) {
            int undefined_count = 0;
            size_t undefined_index = 0;
            for (size_t jj=0; jj<n; jj++) {
                if (system[ii][jj] != 0 && search_ranges[jj][0] == -1) {
                    undefined_count++;
                    undefined_index = jj;
                }
            }
            if (undefined_count == 1) {
                search_ranges[undefined_index][0] = 0;
                search_ranges[undefined_index][1] = system[ii][n];
                for (size_t jj=0; jj<n; jj++) {
                    if (jj == undefined_index) continue;
                    if (system[ii][jj] == 0) continue;
                    if (system[ii][jj] > 0) {
                        search_ranges[undefined_index][0] += (system[ii][jj] * search_ranges[jj][0]);
                    }
                    else
                    {
                        search_ranges[undefined_index][1] -= (system[ii][jj] * search_ranges[jj][1]);
                    }
                }
                ranges_updated = true;
            }
        }
    } while (ranges_updated);

#ifdef DEBUG
    std::cout << "Input ranges: " << "\n";
    for (size_t ii=0; ii<button_pivots.size(); ii++) {
        std::cout << "b" << ii << " [" << search_ranges[ii][0] << "," << search_ranges[ii][1] << "]\n";
    }
    std::cout << "\n" << std::endl;
#endif 

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
