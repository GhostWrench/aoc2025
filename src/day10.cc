#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
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
    //size_t counter = 0;
    for (const MachineInfo& machine : machines) {
        total += count_min_presses(machine);
        //std::cout << ++counter << ": " << total << std::endl;
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
    //int result1 = get_result1(data);
    //int result2 = get_result2(data);

    // Output the results
    std::cout << "Min Button Presses : " << min_presses << std::endl;
    //std::cout << "Result 2: " << result2 << std::endl;

    return EXIT_SUCCESS;
}
