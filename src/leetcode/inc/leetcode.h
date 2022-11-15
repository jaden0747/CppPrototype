#pragma once

#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

class SolutionMedianOfTwoSortedArrays {
public:
    int binarySearch(const std::vector<int>& arr, int low, int high, int key) {
        if (high >= low) {
            int mid = low + (high - low) / 2; 
            if (arr[mid] == key) return mid; 
            else if (arr[mid] > key) return binarySearch(arr, low, mid - 1, key); 
            else return binarySearch(arr, mid + 1, high, key); 
        }
        return -1; 
    }

    double findMedian(const vector<int>& v)
    {
        size_t n = v.size();

        if (n % 2 != 0) {
            return v[n / 2];
        }
        else {
            return (v[n/2 - 1] + v[n/2]) / 2.0;
        }
    }

    double findMedianSortedArraysBruthForce(vector<int>& nums1, vector<int>& nums2) 
    {
        size_t n1 = nums1.size();
        size_t n2 = nums2.size();

        if ((n1 + n2) == 0) {
            return 0.0; // 2 empty vector, mean is 0.0
        }
        else if (n1 == 0) {
            return findMedian(nums2);
        }
        else if (n2 == 0) {
            return findMedian(nums1);
        }
        else {
            std::vector<int> v;
            for (auto i : nums1) {
                v.push_back(i);
            }
            for (auto i : nums2) {
                v.push_back(i);
            }
            std::sort(v.begin(), v.end());
            return findMedian(v);
        }
        
        return 0.0;
    }

    double findMedianSortedArrays(vector<int>& nums1, vector<int>& nums2)
    {
        size_t n1 = nums1.size();
        size_t n2 = nums2.size();
        int i = 0;
        int j = 0;
        int lastindex = -1;

        vector<int> v(n1 + n2, 0);

        while(i < n1 && j < n2) {
            // add the smaller one by one so that no sort need to be done
            if (nums1[i] < nums2[j]) {
                v[++lastindex] = nums1[i++]; // i++ ensure that no skip in nums1
            }
            else {
                //! 2 cases can happen:
                //  nums2[j++] is bigger than nums1[i] => add
                //  nums2[j++] is equal to nums1[i] => add
                v[++lastindex] = nums2[j++]; // j++ ensure that no skip in nums2
            }
        }

        // j >= n2 and break above loop, add remaining to v
        while(i < n1) {
            v[++lastindex] = nums1[i++];
        }

        // i >= n1 and break above loop, add remaining to v
        while(j < n2) {
            v[++lastindex] = nums2[j++];
        }

        size_t n = n1 + n2;
        return n % 2 ? v[n/2] : (v[n/2 - 1] + v[n/2]) / 2.0;
    }
};