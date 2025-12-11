#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

struct Instruction {
    std::vector<size_t> numbers;
    char operation;
};

std::vector<Instruction> read_input(const std::string& fname) {
    // Read the file
    std::ifstream ifile(fname);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(ifile, line)) {
        if (line.empty()) break;
        lines.push_back(line);
    }
    ifile.close();

    // Get each operator and the width of each input
    size_t num_lines = lines.size();
    std::vector<char> operations;
    for (const char& c : lines[num_lines-1]) {
        if (c == '+' || c == '*') {
            operations.push_back(c);
        }
    }
    
    // Push back all the values
    std::vector<Instruction> output;
    for (size_t opidx=0; opidx<operations.size(); opidx++) {
        Instruction inst = {
            .numbers = {},
            .operation = operations[opidx],
        };
        output.push_back(inst);
    }
    for (size_t lnidx=0; lnidx<num_lines-1; lnidx++) {
        size_t op_idx = 0;
        std::stringstream ss(lines[lnidx]);
        size_t value = 0;
        while (ss >> value) {
            output[op_idx].numbers.push_back(value);
            op_idx++;
        }
    }
    return output;
}

std::vector<Instruction> read_cephalopod(const std::string& fname) {
    // Read the file
    std::ifstream ifile(fname);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(ifile, line)) {
        if (line.empty()) break;
        lines.push_back(line);
    }
    ifile.close();

    // Get each operator and the width of each input
    size_t num_lines = lines.size();
    std::vector<char> operations;
    std::vector<size_t> widths;
    int current_width = 0;
    for (const char& c : lines[num_lines-1]) {
        if (c == '+' || c == '*') {
            if (operations.size() > 0) {
                widths.push_back(current_width-1);
            }
            operations.push_back(c);
            current_width = 0;
        }
        current_width++;
    }
    widths.push_back(current_width);
    
    // Push back all the values
    std::vector<Instruction> output;
    size_t str_idx = 0;
    for (size_t opidx=0; opidx<operations.size(); opidx++) {
        Instruction inst = {
            .numbers = {},
            .operation = operations[opidx],
        };
        for (size_t didx=0; didx<widths[opidx]; didx++) {
            size_t number = 0;
            size_t scale = 1;
            for (size_t pidx=num_lines-1; pidx>0; pidx--) {
                char digit = lines[pidx-1][str_idx];
                if (digit != ' ') {
                    number += static_cast<size_t>(digit - '0') * scale;
                    scale *= 10;
                }
            }
            inst.numbers.push_back(number);
            str_idx++;
        }
        output.push_back(inst);
        str_idx++;
    }

    return output;
}

size_t do_homework(const std::vector<Instruction>& instructions) {
    size_t total = 0;
    for (const Instruction& inst : instructions) {
        if (inst.operation == '+') {
            for (const size_t& num : inst.numbers) {
                total += num;
            }
        }
        else if (inst.operation == '*') {
            size_t sub_total = inst.numbers[0];
            for (size_t ni=1; ni<inst.numbers.size(); ni++) {
                sub_total *= inst.numbers[ni];
            }
            total += sub_total;
        }
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
    std::vector<Instruction> instructions  = read_input(argv[1]);
    std::vector<Instruction> cephalopod_instructions = read_cephalopod(argv[1]);

    // Process the inputs
    size_t answer = do_homework(instructions);
    size_t cephalopod_answer = do_homework(cephalopod_instructions);

    // Output the results
    std::cout << "Answer: " << answer << std::endl;
    std::cout << "Cephalopod Answer: " << cephalopod_answer << std::endl;

    return EXIT_SUCCESS;
}
