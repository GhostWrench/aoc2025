#include <array>
#include <fstream>
#include <iostream>
#include <vector>

std::vector<std::vector<size_t>> read_input(const std::string& fname) {
    std::string line;
    std::ifstream ifile(fname);
    std::vector<std::vector<size_t>> grid;
    while (std::getline(ifile, line)) {
        if (line.empty()) break;
        std::vector<size_t> row;
        for (const char& c: line) {
            row.push_back(static_cast<size_t> (c - '0'));
        }
        grid.push_back(row);
    }
    ifile.close();
    return grid;
}

size_t sum_largest_two_digits(const std::vector<std::vector<size_t>>& battery_banks) {
    size_t sum = 0;
    for (const std::vector<size_t>& bank : battery_banks) {
        // Find the first digit
        size_t first_value = 0;
        size_t first_index = 0;
        for (size_t ii=0; ii<(bank.size()-1); ii++) {
            if (bank[ii] > first_value) {
                first_value = bank[ii];
                first_index = ii;
            }
            if (first_value == 9) break;
        }
        // Find the second digit
        size_t second_value = 0;
        for (size_t ii=(first_index+1); ii<bank.size(); ii++) {
            if (bank[ii] > second_value) {
                second_value = bank[ii];
            }
            if (second_value == 9) break;
        }
        sum += (first_value * 10) + second_value;
    }
    return sum;
}

size_t sum_largest_12_digits(const std::vector<std::vector<size_t>>& battery_banks) {
    size_t sum = 0;
    for (const std::vector<size_t>& bank : battery_banks) {
        std::array<size_t, 12> values = {0};
        std::array<size_t, 12> indices = {0};
        for (size_t digit=0; digit<12; digit++) {
            for (size_t ii=indices[digit]; ii<(bank.size()-11+digit); ii++) {
                if (bank[ii] > values[digit]) {
                    values[digit] = bank[ii];
                    if (digit < (12-1)) {
                        indices[digit+1] = ii+1;
                    }
                }
                if (values[digit] == 9) break;
            }
        }
        size_t scale = 1;
        for (int ii=11; ii>=0; ii--) {
            sum += scale * values[ii];
            scale *= 10;
        }
    }
    return sum;
}

int main(int argc, char **argv) {
    
    // Check inputs
    if (argc != 2) {
        std::cout << "Input filename must be provided" << std::endl;
        return EXIT_FAILURE;
    }

    // Read the input file
    std::vector<std::vector<size_t>> battery_banks = read_input(argv[1]);

    // Process the inputs
    size_t joltage1 = sum_largest_two_digits(battery_banks);
    size_t joltage2 = sum_largest_12_digits(battery_banks);

    // Output the results
    std::cout << "Joltage: " << joltage1 << std::endl;
    std::cout << "Big Joltage: " << joltage2 << std::endl;

    return EXIT_SUCCESS;
}
