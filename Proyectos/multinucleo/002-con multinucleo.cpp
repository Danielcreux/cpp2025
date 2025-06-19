#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <immintrin.h>  // For SSE/AVX intrinsics

constexpr size_t CACHE_LINE_SIZE = 64;  // AMD cache line size

// Thread-safe accumulators with cache alignment
struct alignas(CACHE_LINE_SIZE) ThreadData {
    double double_sum = 0.0;
    long long ll_sum = 0;
};

void partial_sum(uint64_t start, uint64_t end, ThreadData& data) {
    double local_double = 0.0;
    long long local_ll = 0;
    
    // Process 2 elements per iteration (optimized for dual-core)
    for (uint64_t i = start; i <= end; i += 2) {
        // Double calculations
        double d1 = static_cast<double>(i);
        double d2 = static_cast<double>(i+1);
        local_double += d1*d1 + d2*d2;
        
        // Long long calculations
        long long ll1 = static_cast<long long>(i);
        long long ll2 = static_cast<long long>(i+1);
        local_ll += ll1*ll1 + ll2*ll2;
    }
    
    data.double_sum = local_double;
    data.ll_sum = local_ll;
}

int main() {
    const uint64_t N = 100'000'000;  // 100 million
    
    // Aligned thread data to prevent false sharing
    alignas(CACHE_LINE_SIZE) ThreadData data1;
    alignas(CACHE_LINE_SIZE) ThreadData data2;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Split work between two cores
    std::thread t1(partial_sum, 1, N/2, std::ref(data1));
    std::thread t2(partial_sum, N/2 + 1, N, std::ref(data2));
    
    t1.join();
    t2.join();
    
    // Combine results
    double double_result = data1.double_sum + data2.double_sum;
    long long ll_result = data1.ll_sum + data2.ll_sum;
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    
    std::cout << "Dual-threaded Results:\n";
    std::cout << "Double sum: " << double_result << "\n";
    std::cout << "Long long sum: " << ll_result << "\n";
    std::cout << "Time: " << duration.count() << " seconds\n";
    
    return 0;
}