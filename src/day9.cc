#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <set>
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

struct RectInfo {
    int64_t area;
    std::array<size_t, 2> pt_idx;
};

int64_t find_largest_area(
    const std::vector<Point2D>& tiles,
    std::vector<RectInfo>* rect_list = nullptr
) {
    int64_t largest_area = 0;
    for (size_t ii=0; ii<tiles.size()-1; ii++) {
        for (size_t jj=(ii+1); jj<tiles.size(); jj++) {
            int64_t w = std::abs(tiles[ii][0] - tiles[jj][0]) + 1;
            int64_t h = std::abs(tiles[ii][1] - tiles[jj][1]) + 1;
            int64_t area = w * h;
            if (area > largest_area) {
                largest_area = area;
            }
            if (rect_list != nullptr) {
                RectInfo ri = {
                    .area = area,
                    .pt_idx = {ii, jj},
                };
                rect_list->push_back(ri);
            }
        }
    }
    if (rect_list != nullptr) {
        std::sort(
            rect_list->begin(),
            rect_list->end(),
            [] (const RectInfo& a, const RectInfo& b) {
                return a.area > b.area;
            }
        );
    }
    return largest_area;
}

struct VSegment {
    int64_t length;
    Point2D coord;
};

bool trace_ray(
    int64_t test_point,
    int64_t rect_min,
    int64_t rect_max,
    const std::vector<VSegment>& vsegments
) {
    bool inside_poly = false;
    int on_edge = 0;
    bool poly_transition = false;
    int edge_transition = 0;
    bool inside_rect = false;
    int64_t prev_x = vsegments[0].coord[0];
    for (const VSegment& vs : vsegments) {
        int64_t next_x = vs.coord[0];
        if (poly_transition) {
            inside_poly = !inside_poly;
            poly_transition = false;
        }
        on_edge = edge_transition;
        // Check if the current ray span has element inside the rectangle
        if (prev_x > rect_max) {
            inside_rect = false;
        }
        else if (next_x > rect_min) {
            inside_rect = true;
        }
        else {
            inside_rect = false;
        }
        // Check for failure and return
        if (inside_rect && !inside_poly) {
            return false;
        }
        
        // Check if the next segement transistions into a polygon and how
        if (test_point < vs.coord[1]) {
            // Does not connect
            prev_x = next_x;
            continue;
        }
        if (test_point == vs.coord[1]) {
            if (on_edge >= 0) {
                poly_transition = true;
                edge_transition = 1;
            }
            else {
                edge_transition = 0;
            }
        }
        else if (test_point < vs.coord[1] + vs.length) {
            poly_transition = true;
        }
        else if (test_point == vs.coord[1] + vs.length) {
            if (on_edge <= 0) {
                poly_transition = true;
                edge_transition = -1;
            }
            else {
                edge_transition = 0;
            }
        }
        else {
            poly_transition = false;
        }
        prev_x = next_x;
    }
    return true;
}

int64_t find_largest_contained_rect(
    const std::vector<Point2D>& tiles,
    const std::vector<RectInfo>& rects
) {
    // Ray tracing solution with pre-sorting of vertical polygon segments to 
    // reduce the number of required tests

    // Collect all the vertical line segments and note the maximum distance
    int64_t vmax = 0;
    std::vector<VSegment> vsegments;
    std::set<int64_t> test_points;
    for (size_t ii=0; ii<tiles.size(); ii++) {
        Point2D p1 = tiles[ii];
        Point2D p2 = tiles[0];
        test_points.insert(p1[1]);
        test_points.insert(p2[1]);
        if (ii < tiles.size() - 1) {
            p2 = tiles[ii+1];
        }
        int64_t dist = 0;
        if (p1[0] == p2[0]) {
            // Vertical line
            VSegment vs;
            if (p2[1] >= p1[1]) {
                dist = p2[1] - p1[1];
                vs = {
                    .length = dist,
                    .coord = p1,
                };
            }
            else {
                dist = p1[1] - p2[1];
                vs = {
                    .length = dist,
                    .coord = p2,
                };
            }
            vsegments.push_back(vs);
            if (dist > vmax) vmax = dist;
        }
    }
    // Sort the line segments by the y coordinate
    std::sort(
        vsegments.begin(),
        vsegments.end(),
        [](const VSegment& a, const VSegment& b) {
            return a.coord[1] < b.coord[1];
        }
    );
    
    // Loop through each rectangle starting with the biggest
    for (const RectInfo& rect : rects) {
        // Calculate the minimum and maximum range for segments
        Point2D pt1 = tiles[rect.pt_idx[0]];
        Point2D pt2 = tiles[rect.pt_idx[1]];
       
        /*
        std::cout << "Rect: "
                  << "(" << pt1[0] << "," << pt1[1] << "), "
                  << "(" << pt2[0] << "," << pt2[1] << ")" << std::endl;
        */

        int64_t rect_min = std::min(pt1[1], pt2[1]);
        int64_t rect_max = std::max(pt1[1], pt2[1]);
        int64_t segment_min = rect_min - vmax;
        // Find the first and last segments to loop through
        std::vector<VSegment>::iterator vseg_begin = std::lower_bound(
            vsegments.begin(),
            vsegments.end(),
            segment_min,
            [] (const VSegment& vs, const int64_t& y) {
                return vs.coord[1] < y;
            }
        );
        std::vector<VSegment>::iterator vseg_end = std::upper_bound(
            vsegments.begin(),
            vsegments.end(),
            rect_max,
            [] (const int64_t& y, const VSegment& vs) {
                return y <= vs.coord[1];
            }
        );
        // Find the span of test coordinates
        std::set<int64_t>::iterator tp_begin = std::lower_bound(
            test_points.begin(),
            test_points.end(),
            rect_min
        );
        std::set<int64_t>::iterator tp_end = std::upper_bound(
            test_points.begin(),
            test_points.end(),
            rect_max
        );
        // Get a copy of the segment and then sort by the x value
        std::vector<VSegment> vseg_select(vseg_begin, vseg_end);
        std::sort(
            vseg_select.begin(),
            vseg_select.end(),
            [] (const VSegment& a, const VSegment& b) {
                return a.coord[0] < b.coord[0];
            }
        );

        /*
        std::cout << "Segments: " 
                  << (vseg_begin - vsegments.begin()) << ","
                  << (vseg_end - vsegments.begin()) << std::endl;

        std::cout << "Test Points: ";
        for (auto it=tp_begin; it!=tp_end; it++) {
            std::cout << *it << ",";
        }
        std::cout << std::endl;
        std::cout << std::endl;
        */


        bool trace_ok = true;
        for (auto tp=tp_begin; tp!=tp_end; tp++) {
            int64_t x_start = std::min(pt1[0], pt2[0]);
            int64_t x_end = std::max(pt1[0], pt2[0]);
            trace_ok = trace_ray(*tp, x_start, x_end, vseg_select);
            if (!trace_ok) break;
        }
        if (trace_ok) {
            return rect.area;
        }
    }
    return 0;
}

int main(int argc, char **argv) {

    // Check inputs
    if (argc != 2) {
        std::cout << "Input filename must be provided" << std::endl;
        return EXIT_FAILURE;
    }

    // Read the input file
    std::vector<Point2D> tiles = read_input(argv[1]);
    std::vector<RectInfo> rect_info;

    // Process the inputs
    int64_t largest_area = find_largest_area(tiles, &rect_info);
    int64_t largest_bounded_rect = find_largest_contained_rect(tiles, rect_info);

    // Output the results
    std::cout << "Largest Area: " << largest_area << std::endl;
    std::cout << "Largest Bounded Rect: " << largest_bounded_rect << std::endl;

    return EXIT_SUCCESS;
}
