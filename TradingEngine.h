#pragma once
#ifndef TRADINGENGINE_H
#define TRADINGENGINE_H
#include "LimitOrderBook.h"
#include "Types.h"
#include "memory_pool.h"

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