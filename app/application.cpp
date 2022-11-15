#include <iostream>
#include "leetcode.h"

int main(int argc, char** argv){
    std::vector<int> nums1 = {1, 3};
    std::vector<int> nums2 = {2, 7};

    SolutionMedianOfTwoSortedArrays solution = SolutionMedianOfTwoSortedArrays();
    double result = solution.findMedianSortedArrays(nums1, nums2);
    return 0;
}