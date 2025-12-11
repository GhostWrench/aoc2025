#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::vector<std::array<size_t, 2>> read_ranges(const std::string& fname) {
    // Read the file
    std::vector<std::array<size_t, 2>> ranges;
    std::string line;
    std::ifstream ifile(fname);
    while (std::getline(ifile, line)) {
        if (line.empty()) break;
        int mid = line.find('-');
        size_t begin = std::stoull(line.substr(0,mid));
        size_t end = std::stoull(line.substr(mid+1));
        ranges.push_back({begin, end});
    }
    ifile.close();
    // Sort the ranges
    std::sort(ranges.begin(), ranges.end(), [](const std::array<size_t,2>& a, const std::array<size_t,2>& b) {
        return a[0] < b[0];
    });
    // Compress the ranges
    size_t idx = 0;
    while (idx < (ranges.size()-1)) {
        if ((ranges[idx][1]+1) >= ranges[idx+1][0]) {
            if (ranges[idx][1] < ranges[idx+1][1]) {
                ranges[idx][1] = ranges[idx+1][1];
            }
            ranges.erase(ranges.begin() + (idx+1));
        }
        else
        {
            idx++;
        }
    }
    return ranges;
}

std::vector<size_t> read_ingredients(const std::string& fname) {
    std::string line;
    std::ifstream ifile(fname);
    bool ingredient_section = false;
    std::vector<size_t> ingredients;
    while (std::getline(ifile, line)) {
        if (!ingredient_section) {
            if (line.empty()) ingredient_section = true;
            continue;
        }
        if (line.empty()) break;
        ingredients.push_back(std::stoull(line));
    }
    ifile.close();
    return ingredients;
}

size_t fresh_ingredients(const std::vector<std::array<size_t,2>>& ranges, const std::vector<size_t>& ingredients) {
    size_t fresh_count = 0;
    for (const size_t& ingredient : ingredients) {
        auto idx = std::lower_bound(
            ranges.begin(), 
            ranges.end(), 
            ingredient, 
            [](const std::array<size_t,2>& range, const size_t& value){
                return range[0] <= value;
            }
        );
        size_t range_idx = static_cast<size_t> (idx - ranges.begin());
        if (range_idx == 0) continue;
        range_idx -= 1;
        if (ranges[range_idx][0] <= ingredient && ranges[range_idx][1] >= ingredient) {
            fresh_count++;
        }
    }
    return fresh_count;
}

size_t fresh_ids(const std::vector<std::array<size_t,2>>& ranges) {
    size_t total = 0;
    for (const std::array<size_t,2>& range: ranges) {
        total += (range[1] - range[0] + 1);
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
    std::vector<std::array<size_t, 2>> ranges = read_ranges(argv[1]);
    std::vector<size_t> ingredients = read_ingredients(argv[1]);

    // Process the inputs
    size_t num_fresh_ingredients = fresh_ingredients(ranges, ingredients);
    size_t num_fresh_ids = fresh_ids(ranges);

    // Output the results
    std::cout << "Fresh Ingredients: " << num_fresh_ingredients << std::endl;
    std::cout << "Fresh IDs: " << num_fresh_ids << std::endl;
    
    return EXIT_SUCCESS;
}
