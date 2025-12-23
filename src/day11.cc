#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Device {
    std::string name;
    std::vector<std::string> outputs;
    std::vector<std::string> inputs;
};

std::unordered_map<std::string, Device> read_input(const std::string& fname) {
    // Read the file
    std::ifstream ifile(fname);
    std::string line;
    std::unordered_map<std::string, Device> devices;
    while (std::getline(ifile, line)) {
        if (line.empty()) break;
        std::string name;
        std::stringstream ss(line);
        ss >> name;
        name = name.substr(0, name.size()-1);
        Device& device = devices[name];
        device.name = name;
        std::string output;
        while (ss >> output) {
            device.outputs.push_back(output);
        }
        for (const std::string& output_name : device.outputs) {
            Device& output_device = devices[output_name];
            output_device.inputs.push_back(name);
        }
    }
    ifile.close();
    return devices;
}

size_t count_paths(
    const std::unordered_map<std::string, Device>& devices,
    const std::string& name = "you",
    const std::unordered_set<std::string>& pass_thru = {},
    const std::unordered_set<std::string>& path = {}
) {
    size_t paths = 0;
    const Device& device = devices.at(name);
    for (const std::string& output : device.outputs) {
        if (output == "out" && pass_thru.empty()) {
            paths++;
        }
        else if (output == "out") {
            continue;
        }
        else if (path.contains(output)) {
            continue;
        }
        else {
            std::unordered_set<std::string> path_update = path;
            path_update.insert(output);
            std::unordered_set<std::string> pass_thru_update = pass_thru;
            pass_thru_update.erase(name);
            paths += count_paths(devices, output, pass_thru_update, path_update);
        }
    }
    return paths;
}

int main(int argc, char **argv) {

    // Check inputs
    if (argc != 2) {
        std::cout << "Input filename must be provided" << std::endl;
        return EXIT_FAILURE;
    }

    // Read the input file
    std::unordered_map<std::string, Device> devices = read_input(argv[1]);

    // Process the inputs
    size_t num_paths = count_paths(devices);
    //size_t srv_paths = count_paths(devices, "svr", {"dac","fft"});

    // Output the results
    std::cout << "All paths: " << num_paths << std::endl;
    //std::cout << "Server paths: " << srv_paths << std::endl;

    return EXIT_SUCCESS;
}
