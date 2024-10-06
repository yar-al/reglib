#ifndef UTIL
#define UTIL

#include <vector>
#include <algorithm>

namespace reglib{
    class Util{
    public:
        static std::vector<int> merge(std::vector<int> v1, std::vector<int> v2){
            std::vector<int> v;
            for(const auto &i : v1){
                v.push_back(i);
            }
            bool flag = true;
            for(const auto &i : v2){
                flag = true;
                for(const auto &j : v){
                    if(i == j)
                        flag = false;
                }
                if(flag) v.push_back(i);
            }
            std::sort(v.begin(), v.end());
            return v;
        }

        template <typename T>
        static bool vector_s_c(std::vector<T>& v1, std::vector<T>& v2){
            std::sort(v1.begin(), v1.end());
            std::sort(v2.begin(), v2.end());
            return v1 == v2;
        }
    };
}

#endif