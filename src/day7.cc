#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#ifdef DEBUG
#include <iomanip>
#endif // DEBUG

std::vector<std::string> read_input(const std::string& fname) {
    // Read the file
    std::ifstream ifile(fname);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(ifile, line)) {
        if (line.empty()) break;
        lines.push_back(line);
    }
    ifile.close();
    return lines;
}

size_t count_splits(const std::vector<std::string>& data) {
    std::vector<std::string> local_data = data;
    size_t splits = 0;
    for (size_t row=0; row<local_data.size()-1; row++) {
        for (size_t col=0; col<local_data[row].length(); col++) {
            char& upper = local_data[row][col];
            char& lower = local_data[row+1][col];
            char& left = local_data[row+1][col-1];
            char& right = local_data[row+1][col+1];

            if (upper == 'S' || upper == '|') {
                if (lower == '.') {
                    lower = '|';
                }
                else if (lower == '^') {
                    splits++;
                    left = '|';
                    right = '|';
                }
            }
        }
    }
    return splits;
}

size_t count_timelines(const std::vector<std::string>& data) {

    // Convert data to a more useful form
    std::vector<std::vector<int64_t>> local_data;
    for (size_t row=0; row<data.size(); row++) {
        std::vector<int64_t> row_data;
        for (size_t col=0; col<data[row].length(); col++) {
            char value = data[row][col];
            if (value == 'S' || value == '|') {
                row_data.push_back(1);
            }
            else if (value == '^') {
                row_data.push_back(-1);
            }
            else
            {
                row_data.push_back(0);
            }
        }
        local_data.push_back(row_data);
    }

    // Split the beams
    for (size_t row=0; row<local_data.size()-1; row++) {
        for (size_t col=0; col<local_data[row].size(); col++) {
            int64_t& upper = local_data[row][col];
            int64_t& lower = local_data[row+1][col];
            int64_t& left = local_data[row+1][col-1];
            int64_t& right = local_data[row+1][col+1];

            if (upper > 0) {
                if (lower != -1) {
                    lower += upper;
                }
                else {
                    left += upper;
                    right += upper;
                }
            }
        }
    }

#ifdef DEBUG
    // Print the map
    for (size_t row=0; row<local_data.size()-1; row++) {
        for (size_t col=0; col<local_data[row].size(); col++) {
            std::cout << std::setw(20) << local_data[row][col];
        }
        std::cout << std::endl;
    }
#endif //DEBUG
    
    // Cound the timelines
    int64_t timelines = 0;
    size_t last_row = local_data.size()-1;
    for (size_t col=0; col<local_data[last_row].size(); col++) {
        int64_t value = local_data[last_row][col];
        if (value > 0) timelines += static_cast<size_t>(value);
    }
    return timelines;
}

int main(int argc, char **argv) {

    // Check inputs
    if (argc != 2) {
        std::cout << "Input filename must be provided" << std::endl;
        return EXIT_FAILURE;
    }

    // Read the input file
    std::vector<std::string> data = read_input(argv[1]);

    // Process the inputs
    size_t splits = count_splits(data);
    size_t result2 = count_timelines(data);

    // Output the results
    std::cout << "Splits: " << splits << std::endl;
    std::cout << "Timelines: " << result2 << std::endl;

    return EXIT_SUCCESS;
}
