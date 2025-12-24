#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const size_t NUM_SHAPES = 6;

struct Present {
    std::array<std::array<char, 3>, 3> shape;
    int volume;
};

struct Region {
    int width;
    int height;
    std::array<int, NUM_SHAPES> presents;
};

struct Input {
    std::array<Present, NUM_SHAPES> presents;
    std::vector<Region> regions;
};

Input read_input(const std::string& fname) {
    // Read the file
    std::ifstream ifile(fname);
    std::string line;
    Input input;
    for (size_t present_idx=0; present_idx<NUM_SHAPES; present_idx++) {
        std::getline(ifile, line);
        Present present;
        present.volume = 0;
        for (size_t row=0; row<3; row++) {
            std::getline(ifile, line);
            for (size_t col=0; col<3; col++) {
                if (line[col] == '#') {
                    present.volume++;
                }
                present.shape[row][col] = line[col];
            }
        }
        input.presents[present_idx] = present;
        std::getline(ifile, line);
    }

    std::vector<Region> regions;
    while (std::getline(ifile, line)) {
        Region region;
        sscanf(
            line.c_str(), "%dx%d: %d %d %d %d %d",
            &region.width,
            &region.height,
            &region.presents[0],
            &region.presents[1],
            &region.presents[2],
            &region.presents[3],
            &region.presents[4]
        );
        input.regions.push_back(region);
    }
    ifile.close();
    return input;
}

/*
bool presents_fit_in_region(const Input& input) {

    for ()

}
*/

int main(int argc, char **argv) {

    // Check inputs
    if (argc != 2) {
        std::cout << "Input filename must be provided" << std::endl;
        return EXIT_FAILURE;
    }

    // Read the input file
    Input input = read_input(argv[1]);

    // Process the inputs
    //int result1 = get_result1(data);
    //int result2 = get_result2(data);

    // Output the results
    std::cout << "Input Regions: " << input.regions.size() << std::endl;
    //std::cout << "Result 2: " << result2 << std::endl;

    return EXIT_SUCCESS;
}
