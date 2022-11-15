#include <gtest/gtest.h>
#include "mylib.h"
#include "leetcode.h"

TEST(MedianOfTwoSortedArrays, Example1) 
{
  std::vector<int> nums1 = {1, 3};
  std::vector<int> nums2 = {2};

  SolutionMedianOfTwoSortedArrays solution = SolutionMedianOfTwoSortedArrays();
  double result = solution.findMedianSortedArrays(nums1, nums2);
  EXPECT_DOUBLE_EQ(result, 2.0);
}


TEST(MedianOfTwoSortedArrays, Example2) 
{
  std::vector<int> nums1 = {1, 2};
  std::vector<int> nums2 = {3, 4};

  SolutionMedianOfTwoSortedArrays solution = SolutionMedianOfTwoSortedArrays();
  double result = solution.findMedianSortedArrays(nums1, nums2);
  EXPECT_DOUBLE_EQ(result, 2.5);
}


TEST(MedianOfTwoSortedArrays, Example3) 
{
  std::vector<int> nums1 = {0, 0};
  std::vector<int> nums2 = {0, 0};

  SolutionMedianOfTwoSortedArrays solution = SolutionMedianOfTwoSortedArrays();
  double result = solution.findMedianSortedArrays(nums1, nums2);
  EXPECT_DOUBLE_EQ(result, 0.0);
}


TEST(MedianOfTwoSortedArrays, Example4) 
{
  std::vector<int> nums1 = {};
  std::vector<int> nums2 = {1};

  SolutionMedianOfTwoSortedArrays solution = SolutionMedianOfTwoSortedArrays();
  double result = solution.findMedianSortedArrays(nums1, nums2);
  EXPECT_DOUBLE_EQ(result, 1.0);
}


TEST(MedianOfTwoSortedArrays, Example5) 
{
  std::vector<int> nums1 = {2};
  std::vector<int> nums2 = {};

  SolutionMedianOfTwoSortedArrays solution = SolutionMedianOfTwoSortedArrays();
  double result = solution.findMedianSortedArrays(nums1, nums2);
  EXPECT_DOUBLE_EQ(result, 2.0);
}

TEST(MedianOfTwoSortedArrays, Test1)
{
  std::vector<int> nums1 = {1, 3};
  std::vector<int> nums2 = {2, 7};

  SolutionMedianOfTwoSortedArrays solution = SolutionMedianOfTwoSortedArrays();
  double result = solution.findMedianSortedArrays(nums1, nums2);
  EXPECT_DOUBLE_EQ(result, 2.5);
}