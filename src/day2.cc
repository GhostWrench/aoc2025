#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <sstream>
#include <vector>

std::vector<std::array<std::string, 2>> read_input(const std::string& fname) {
    std::ifstream ifile(fname);
    std::string str_range;
    std::vector<std::array<std::string, 2>> ranges;
    while(std::getline(ifile, str_range, ',')) {
        size_t dash_idx = str_range.find('-');
        std::string s1 = str_range.substr(0,dash_idx);
        std::string s2 = str_range.substr(dash_idx+1);
        if (s2.back() == '\n') {
            s2.pop_back();
        }
        std::array<std::string, 2> range = {s1, s2};
        ranges.push_back(range);
    }
    ifile.close();
    return ranges;
}

u_int64_t sum_invalid_ids(const std::vector<std::array<std::string, 2>>& ranges) {
    u_int64_t grand_sum = 0;
    for (const std::array<std::string, 2>& range: ranges) {
        // Find the minimum possible value
        size_t lb_len = range[0].length();
        u_int64_t min = 1;
        // Odd number of digits
        if (lb_len & 0x01)
        {
            lb_len = (lb_len + 1) >> 1;
            for (size_t ii=1; ii<lb_len; ii++)
            {
                min *= 10;
            }
        }
        // Even number of digits
        else
        {
            u_int64_t mid = lb_len >> 1;
            min = std::stoull(range[0].substr(0, mid));
            u_int64_t v2 = std::stoull(range[0].substr(mid));
            if (v2 > min) {
                min++;
            }
        }

        // Find the maximum possible value
        size_t ub_len = range[1].length();
        if (ub_len < 2)
        {
            continue;
        }
        u_int64_t max = 10;
        // Odd number of digits
        if (ub_len & 0x01)
        {
            ub_len = (ub_len - 1) >> 1;
            for (size_t ii=1; ii<ub_len; ii++)
            {
                max *= 10;
            }
            max -= 1;
        }
        // Even number of digits
        else
        {
            u_int64_t mid = ub_len >> 1;
            max = std::stoull(range[1].substr(0, mid));
            u_int64_t v2 = std::stoull(range[1].substr(mid));
            if (v2 < max) {
                max--;
            }
        }

        // Nothing to search
        if (min > max) {
            continue;
        }

        // Sum up the values
        u_int64_t sum = 0;
        for (u_int64_t ii=min; ii<=max; ii++) {
            std::stringstream ss("");
            ss << ii << ii;
            sum += std::stoull(ss.str());
        }
        grand_sum += sum;
    }
    return grand_sum;
}

u_int64_t sum_invalid_ids_2(const std::vector<std::array<std::string, 2>>& ranges) {
    u_int64_t grand_sum = 0;
    for (const std::array<std::string, 2>& range: ranges) {

        // Get the full min and max values of the range
        u_int64_t full_min = std::stoull(range[0]);
        u_int64_t full_max = std::stoull(range[1]);

#ifdef DEBUG
        std::cout << full_min << "-" << full_max << std::endl;
#endif

        // Find the maximum possible value
        size_t ub_len = range[1].length();
        if (ub_len < 2)
        {
            continue;
        }
        u_int64_t max = 10;
        // Odd number of digits
        if (ub_len & 0x01)
        {
            ub_len = (ub_len - 1) >> 1;
            for (size_t ii=1; ii<ub_len; ii++)
            {
                max *= 10;
            }
            max -= 1;
        }
        // Even number of digits
        else
        {
            u_int64_t mid = ub_len >> 1;
            max = std::stoull(range[1].substr(0, mid));
            u_int64_t v2 = std::stoull(range[1].substr(mid));
            if (v2 < max) {
                max--;
            }
        }

        // Sum up the values
        u_int64_t sum = 0;
        std::set<u_int64_t> used_values;
        used_values.clear();
        for (u_int64_t ii=1; ii<=max; ii++) {
            std::stringstream ss("");
            while (true) {
                ss << ii;
                u_int64_t test_val = std::stoull(ss.str());
                if (test_val >= full_min && test_val <= full_max) {
                    if (!used_values.contains(test_val)) {
                        sum += test_val;
                        used_values.insert(test_val);
#ifdef DEBUG
                        std::cout << test_val << std::endl;
#endif
                    }
                }
                if (test_val > full_max) {
                    break;
                }
            }
        }
        grand_sum += sum;
    }
    return grand_sum;
}

int main(int argc, char **argv) {
    
    // Check inputs
    if (argc != 2) {
        std::cout << "Input filename must be provided" << std::endl;
        return EXIT_FAILURE;
    }

    // Read the input file
    std::vector<std::array<std::string, 2>> ranges = read_input(argv[1]);

    u_int64_t sum = sum_invalid_ids(ranges);
    u_int64_t sum2 = sum_invalid_ids_2(ranges);
    std::cout << "Sum: " << sum << std::endl;
    std::cout << "Sum 2: " << sum2 << std::endl;

    return EXIT_SUCCESS;
}
