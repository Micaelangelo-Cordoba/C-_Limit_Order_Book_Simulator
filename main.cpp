#include <iostream>
#include <random>
#include <vector>
#include "profiler.h"
#include "DataFields.h"
#include "performance_test.h"

std::vector<NetworkPacket> generate_mock_traffic(size_t num_packets) { 
    using enum Type;
    std::vector<NetworkPacket> traffic;
    traffic.reserve(num_packets);

    std::mt19937_64 rng(35); //hardcoded a seed so performance is deterministic

    //will be casted back to uint64_t. Curve centered at $150.00 (15000 ticks) with a deviation of $0.50 (50 ticks)
    std::normal_distribution<double> price_dist (15000.0, 50.0);

    //between 100 and 1000 shares
    std::uniform_int_distribution<uint32_t> size_dist (1, 10);

    //distributes the types of messages 0-99%
    std::uniform_int_distribution<int> type_dist(0, 99);

    std::vector<uint64_t> active_ids;
    active_ids.reserve(num_packets);

    uint64_t current_timestamp = 1000000;
    uint64_t next_order_id = 1;

    for (size_t i = 0; i < num_packets; ++i) {
        NetworkPacket netpacket;
        netpacket.n_timestamp = current_timestamp++; //imagine time moving in nanoseconds;

        int rand_percent = type_dist(rng); //let the hardcoded seed generator choose a percentage
        //this will allow us to distribute the type of orders.
        //Market MIX:
        //40% add Buy //40% add sell // 15% cancel // 5% market orders

        if (rand_percent < 40) {  //add buys
            netpacket.N_TYPE = ADD_BUY;
            netpacket.n_id = next_order_id++; //move ids forward
            netpacket.n_price = static_cast<uint64_t>(price_dist(rng)); //random price
            netpacket.n_size = size_dist(rng) * 100;
            active_ids.push_back(netpacket.n_id);

        } else if (rand_percent < 80) { //add sell
            netpacket.N_TYPE = ADD_SELL;
            netpacket.n_id = next_order_id++;
            netpacket.n_price = static_cast<uint64_t>(price_dist(rng));
            netpacket.n_size = size_dist(rng) * 100;
            active_ids.push_back(netpacket.n_id);
        } else if (rand_percent < 95) {  //cancelling, there must be logic incase this gets called first
            netpacket.N_TYPE = CANCEL; 
            
            if (!active_ids.empty()) {
                std::uniform_int_distribution<size_t> idx_dist(0, active_ids.size() -1); //choose random order to cancel
                size_t random_idx = idx_dist(rng);
                netpacket.n_id = active_ids[random_idx];
                
                active_ids[random_idx] = active_ids.back(); //move to the back and pop it
                active_ids.pop_back();
            }
            else { //no orders may have been inserted yet, default to adding
                netpacket.N_TYPE = ADD_BUY;
                netpacket.n_id = next_order_id++;
                netpacket.n_price = static_cast<uint64_t>(price_dist(rng));
                netpacket.n_size = size_dist(rng) * 100;

                active_ids.push_back(netpacket.n_id);
            }
        } else { //last 5% -> market orders

            netpacket.N_TYPE = (rand_percent % 2 == 0) ? MKT_BUY : MKT_SELL;
            netpacket.n_id = next_order_id++;
            netpacket.n_size = size_dist(rng) * 100;
        }
        traffic.push_back(netpacket);
    }

    return traffic;

}



int main() {
    std::cout << "Generating 5 million mock messages!\n";
    std::vector<NetworkPacket> mock_data = generate_mock_traffic(5'000'000);

    TradingEngine engine;

    std::cout << "Starting engine execution\n";
    profiler timer {};

    for (const auto& packet: mock_data) {
        engine.packet_received(packet);
    }

    double OfficialTime = timer.timeElapsed();


    std::cout << "Processed 5 million packets in: " << OfficialTime << " microseconds\n";
    std::cout << "Throughput: " << (5'000'000 / (OfficialTime / 1'000'000)) << "matches/sec\n";



    return 0; 
}