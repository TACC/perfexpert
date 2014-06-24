#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

template<typename T>
vector<T> getIntersection(vector<T>& v1,vector<T>& v2){
        vector<T> target(v1.size() + v2.size());
        sort(v1.begin(),v1.end());
        sort(v2.begin(),v2.end());
        auto it = std::set_intersection(v1.begin(),v1.end(),v2.begin(),v2.end(),target.begin());
        target.resize(it - target.begin());
        return target;	
}
