#include <iostream>
#include <chrono>
#include <immintrin.h>  // For SSE/AVX intrinsics

constexpr size_t CACHE_LINE_SIZE = 64;  // AMD cache line size

void calculate_sums(uint64_t n, double& double_sum, long long& ll_sum) {
    double_sum = 0.0;
    ll_sum = 0;
    
    // Align memory for cache optimization
    alignas(CACHE_LINE_SIZE) double temp_double = 0.0;
    alignas(CACHE_LINE_SIZE) long long temp_ll = 0;
    
    // Loop unrolling with 4-way parallelism
    for (uint64_t i = 1; i <= n; i += 4) {
        // Process 4 elements at a time
        double d1 = static_cast<double>(i);
        double d2 = static_cast<double>(i+1);
        double d3 = static_cast<double>(i+2);
        double d4 = static_cast<double>(i+3);
        
        temp_double += d1*d1 + d2*d2 + d3*d3 + d4*d4;
        
        long long ll1 = static_cast<long long>(i);
        long long ll2 = static_cast<long long>(i+1);
        long long ll3 = static_cast<long long>(i+2);
        long long ll4 = static_cast<long long>(i+3);
        
        temp_ll += ll1*ll1 + ll2*ll2 + ll3*ll3 + ll4*ll4;
    }
    
    double_sum = temp_double;
    ll_sum = temp_ll;
}

int main() {
    const uint64_t N = 100'000'000;  // 100 million
    
    double double_result;
    long long ll_result;
    
    auto start = std::chrono::high_resolution_clock::now();
    calculate_sums(N, double_result, ll_result);
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> duration = end - start;
    
    std::cout << "Single-threaded Results:\n";
    std::cout << "Double sum: " << double_result << "\n";
    std::cout << "Long long sum: " << ll_result << "\n";
    std::cout << "Time: " << duration.count() << " seconds\n";
    
    return 0;
}