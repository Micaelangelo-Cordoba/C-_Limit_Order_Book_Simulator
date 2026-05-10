#ifndef PERFORMANCE_TEST_H
#define PERFORMANCE_TEST_H
#include <cstdint>
#include "DataFields.h"


enum class Type : char { //will be used by simulated network packets to indicate what type of order they are
    ADD_BUY = 'B', 
    ADD_SELL = 'S',
    CANCEL = 'C',
    MKT_BUY = 'N',
    MKT_SELL = 'M'
};

struct NetworkPacket { //represents a fully parsed packet that needs to be processed by the order book
   
    uint64_t n_id;
    uint64_t n_price;
    uint64_t n_timestamp;
    uint32_t n_size;
    Type N_TYPE;
};


class TradingEngine {

    private: 
        LimitOrderBook LOB {};


    public:
        void packet_received(const NetworkPacket& packet); //will process these packets
     
};


inline void TradingEngine::packet_received(const NetworkPacket& packet) {

    using enum Type;

    switch (packet.N_TYPE) { //enumerators used to determine what the LOB should do with the packets

        case (ADD_BUY): {
            LOB.add_buy_limit_order(packet.n_id, packet.n_price, packet.n_size, packet.n_timestamp);
            break;
        }
        case (ADD_SELL):  {
            LOB.add_sell_limit_order(packet.n_id, packet.n_price, packet.n_size, packet.n_timestamp);
            break;
        }
        case (CANCEL): {
            LOB.cancel_order(packet.n_id);
            break;
        }
        case (MKT_BUY): {
            LOB.execute_market_buy(packet.n_size);
            break;
        }
        case (MKT_SELL): {
            LOB.execute_market_sell(packet.n_size);
            break;
        }
        default: {
            std::cerr << "Error when processing packet\n";
            break;
        }
    }
}







#endif