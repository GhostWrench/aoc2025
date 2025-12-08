#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> read_input(const std::string& fname) {
    std::string line;
    std::ifstream ifile(fname);
    std::vector<std::string> map;
    while (std::getline(ifile, line)) {
        if (line.empty()) break;
        map.push_back(line);
    }
    ifile.close();
    return map;
}

int count_accessible(const std::vector<std::string>& map) {
    int accessible = 0;
    for (int yy=0; yy<map.size(); yy++) {
        for (int xx=0; xx<map[yy].length(); xx++) {
            char value = map[yy][xx];
            if (value != '@') continue;
            int nearby_count = 0;
            for (int y_offset=-1; y_offset<=1; y_offset++) {
                for (int x_offset=-1; x_offset<=1; x_offset++) {
                    int y_test = yy + y_offset;
                    if (y_test < 0 || y_test >= map.size()) continue;
                    int x_test = xx + x_offset;
                    if (x_test < 0 || x_test >= map[xx].length()) continue;
                    if (y_test == yy && x_test == xx) continue;
                    if (map[y_test][x_test] == '@') nearby_count++;
                }
            }
            if (nearby_count < 4) accessible++;
        }
    }
    return accessible;
}

int count_all(const std::vector<std::string>& map) {
    std::vector<std::string> copy = map;
    int accessible = 0;
    int new_removals = 1;
    while(new_removals) {
        new_removals = 0;
        for (int yy=0; yy<copy.size(); yy++) {
            for (int xx=0; xx<copy[yy].length(); xx++) {
                char value = copy[yy][xx];
                if (value != '@') continue;
                int nearby_count = 0;
                for (int y_offset=-1; y_offset<=1; y_offset++) {
                    for (int x_offset=-1; x_offset<=1; x_offset++) {
                        int y_test = yy + y_offset;
                        if (y_test < 0 || y_test >= copy.size()) continue;
                        int x_test = xx + x_offset;
                        if (x_test < 0 || x_test >= copy[xx].length()) continue;
                        if (y_test == yy && x_test == xx) continue;
                        if (copy[y_test][x_test] == '@') nearby_count++;
                    }
                }
                if (nearby_count < 4) {
                    copy[yy][xx] = 'x';
                    new_removals++;
                    accessible++;
                }
            }
        }
    }
    return accessible;
}

int main(int argc, char **argv) {
    
    // Check inputs
    if (argc != 2) {
        std::cout << "Input filename must be provided" << std::endl;
        return EXIT_FAILURE;
    }

    // Read the input file
    std::vector<std::string> map = read_input(argv[1]);

    // Process the inputs
    int accessible = count_accessible(map);
    int remove_all = count_all(map);

    // Output the results
    std::cout << "# Accessible: " << accessible << std::endl;
    std::cout << "# Removed: " << remove_all << std::endl;

    return EXIT_SUCCESS;
}
