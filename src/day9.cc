#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using Point2D = std::array<int64_t, 2>;
std::vector<Point2D> read_input(const std::string& fname) {
    // Read the file
    std::ifstream ifile(fname);
    std::string line;
    std::vector<std::array<int64_t, 2>> output;
    while (std::getline(ifile, line)) {
        if (line.empty()) break;
        std::stringstream ss(line);
        output.push_back({0, 0});
        for (size_t ii=0; ii<2; ii++){
            std::string cell;
            std::getline(ss, cell, ',');
            output[output.size()-1][ii] = std::stoll(cell);
        }
    }
    ifile.close();
    return output;
}

int64_t p1_brute_force(const std::vector<Point2D>& tiles) {
    int64_t largest_area = 0;
    for (size_t ii=0; ii<tiles.size(); ii++) {
        for (size_t jj=0; jj<tiles.size(); jj++) {
            int64_t w = std::abs(tiles[ii][0] - tiles[jj][0]) + 1;
            int64_t h = std::abs(tiles[ii][1] - tiles[jj][1]) + 1;
            int64_t area = w * h;
            if (area > largest_area) {
                largest_area = area;
            }
        }
    }
    return largest_area;
}

int main(int argc, char **argv) {

    // Check inputs
    if (argc != 2) {
        std::cout << "Input filename must be provided" << std::endl;
        return EXIT_FAILURE;
    }

    // Read the input file
    std::vector<Point2D> tiles = read_input(argv[1]);

    // Process the inputs
    int64_t largest_area = p1_brute_force(tiles);

    // Output the results
    std::cout << "Largest Area: " << largest_area << std::endl;

    return EXIT_SUCCESS;
}
