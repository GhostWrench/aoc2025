#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::vector<int> read_input(const std::string& fname) {
    std::vector<int> turns;
    std::ifstream ifile(fname);
    std::string line;
    while (ifile >> line)
    {
        int direction = 1;
        if (line.substr(0, 1) == "L") direction = -1;
        int distance = std::stoi(line.substr(1));
        turns.push_back(direction * distance);
    }
    return turns;
}

int get_code(const std::vector<int>& turns, int start) {
    int current = start;
    int zeros = 0;
    for (const int& turn: turns) {
        current += turn;
        while (current < 0) current += 100;
        while (current >= 100) current -= 100;
        if (current == 0) zeros++;
    }
    return zeros;
}

int get_code_0x434C49434B(const std::vector<int>& turns, int start) {
    //int prev = start;
    int current = start;
    int zeros = 0;
    //bool skip_cross_check = false;
#ifdef DEBUG
    int line = 0;
    std::ofstream ofile("day1.log");
#endif
    for (const int& turn: turns) {
        int full_turns = std::abs(turn / 100);
        int rem_clicks = turn % 100;
        zeros += full_turns;
        int prev = current;
        current += rem_clicks;
        if (current >= 100) {
            zeros++;
            current -= 100;
        }
        else if (current <= -100)
        {
            zeros++;
            current += 100;
        }
        else if (prev < 0 && current >= 0)
        {
            zeros++;
        }
        else if (prev > 0 && current <= 0)
        {
            zeros++;
        }
#ifdef DEBUG
        ofile << line << ": " << turn << " => (" << current << "," << zeros << ")" <<std::endl;
        line++;
#endif
    }
#ifdef DEBUG
    ofile.close();
#endif
    return zeros;
}

int main(int argc, char **argv) {

    // Read the input file
    std::vector turns = read_input("data/day1/input.dat");

    // Add up the turns
    int safe_code = get_code(turns, 50);
    int safe_code_0x434C49434B = get_code_0x434C49434B(turns, 50);

    // Print results
    std::cout << "Safe Code: " << safe_code << std::endl;
    std::cout << "Safe Code Updated: " << safe_code_0x434C49434B << std::endl;

    return EXIT_SUCCESS;

}
