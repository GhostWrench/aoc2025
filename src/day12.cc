#include <array>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const size_t NUM_SHAPES = 6;

struct Present {
    std::array<std::array<char, 3>, 3> shape;
    int area;
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
        present.area = 0;
        for (size_t row=0; row<3; row++) {
            std::getline(ifile, line);
            for (size_t col=0; col<3; col++) {
                if (line[col] == '#') {
                    present.area++;
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

int presents_fit_in_region(const Input& input) {
    int num_fit = 0;
    for (const Region& region : input.regions) {
        int region_area = region.width * region.height;
        int present_area = 0;
        int total_presents = 0;
        for (size_t pidx=0; pidx<input.presents.size(); pidx++) {
            present_area += region.presents[pidx] * input.presents[pidx].area;
            total_presents += region.presents[pidx];
        }
        //std::cout << "Presents: " << present_area << " Region: " << region_area << std::endl;
        if (present_area > region_area) {
            continue;
        }
        int min_presents = (region.width / 3) * (region.height / 3);
        //std::cout << "Min Fit: " << min_presents << " Num: " << total_presents << std::endl;
        if (total_presents <= min_presents) {
            num_fit++;
            continue;
        }
    }
    return num_fit;
}

int main(int argc, char **argv) {

    // Check inputs
    if (argc != 2) {
        std::cout << "Input filename must be provided" << std::endl;
        return EXIT_FAILURE;
    }

    // Read the input file
    Input input = read_input(argv[1]);

    // Process the inputs
    int presents_fit = presents_fit_in_region(input);

    // Output the results
    std::cout << "Presents that fit: " << presents_fit << std::endl;

    return EXIT_SUCCESS;
}
