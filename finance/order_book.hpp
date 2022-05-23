#ifndef __order_book_h
#define __order_book_h

#include <unordered_map>
#include <vector>

namespace finance {

/* I probably don't want to manage a full order object for each thing that comes in
* there fore for any changes to hte book I'll want to make certain assumptions 
* 
*
*--------------
*        an sn
*         ...
*        a2 s2
*        a1 s1
*     ----
* s1 b1
* s2 b3
*  ...
* sn bn
*--------------
*
* At each price level (bn, an), we want to know:
*   1) How many orders are at this level (e.g. how many increases/decreases in value)
*   2) What is the size at this level
*
*
* Should I use a vector to represent the price levels or something else .... aaaaaah
*/

typedef struct PriceNode {
    PriceNode(double price, double size);
    PriceNode(double price);

    int n_orders;
    double price;
    double size;
    PriceNode* prev = nullptr;
    PriceNode* next = nullptr;
} PriceNode;

class OrderBook {
public:
    OrderBook(double tick_size, double min, double max);
    OrderBook(double tick_size);
    //~OrderBook();

    double get_mid(); // get mid price (bid + offer) / 2;
    double get_weighted_mid(); // return (bid * offer_size + offer *bid_size) / (offer_size + bid_size);

    double get_best_bid(); // get best bid price
    double get_best_offer(); // get best offer price
    double get_best_bid_size(); //  get best bid size
    double get_best_offer_size(); // get best offer size

    PriceNode* get_best_bid_node();
    PriceNode* get_best_offer_node();
    PriceNode* get_bid_node_at_level(double level);
    PriceNode* get_offer_node_at_level(double level);

    void update_bid(double price, double size);
    void update_offer(double price, double size);
    void clear_book();

    std::vector<std::vector<double>> get_bids(int levels); // -1 = all levels, 0 = just top of book
    std::vector<std::vector<double>> get_offers(int levels); // -1 = all levels, 0 = just top of book
    
private:
    PriceNode* m_best_bid_node = nullptr; // node for top of book
    PriceNode* m_best_offer_node = nullptr; // node for top of book
    PriceNode* m_worst_bid_node = nullptr;
    PriceNode* m_worst_offer_node = nullptr;
    std::unordered_map<double, PriceNode*> m_bids; // {px : PriceNode* } if it exists
    std::unordered_map<double, PriceNode*> m_offers; 

    double m_tick_size;

    void get_levels(int &levels, std::vector<std::vector<double>>&, PriceNode*);
    void insert_bid_node(PriceNode* bid);
    void insert_offer_node(PriceNode* offer);

};

}

#endif
