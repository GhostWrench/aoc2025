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
    }
    ifile.close();
    return devices;
}

size_t count_paths(
    const std::unordered_map<std::string, Device>& devices,
    const std::string& start = "you",
    const std::string& end = "out",
    const std::unordered_set<std::string>& pass_thru = {},
    std::unordered_map<std::string, size_t>* cache = nullptr
) {
    std::stringstream cache_key;
    cache_key << start;
    for (const std::string& pt : pass_thru) {
        cache_key << pt;
    }
    bool do_free = false;
    if (cache == nullptr) {
        do_free = true;
        cache = new std::unordered_map<std::string, size_t>;
    }
    if (cache->contains(cache_key.str())) {
        return cache->at(cache_key.str());
    }
    size_t path_count = 0;
    const Device& device = devices.at(start);
    for (const std::string& output : device.outputs) {
        if (output == end && pass_thru.empty()) {
            path_count++;
        }
        else if (output == end) {
            continue;
        }
        else {
            std::unordered_set<std::string> pass_thru_update = pass_thru;
            pass_thru_update.erase(start);
            path_count += count_paths(devices, output, end, pass_thru_update, cache);
        }
    }
    if (do_free) {
        delete cache;
    }
    else {
        cache->insert(std::make_pair(cache_key.str(), path_count));
    }
    return path_count;
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
    size_t num_paths = count_paths(devices, "you");
    size_t svr_paths = count_paths(devices, "svr", "out", {"dac", "fft"});

    // Output the results
    std::cout << "All paths: " << num_paths << std::endl;
    std::cout << "Server paths: " << svr_paths << std::endl;

    return EXIT_SUCCESS;
}
