#include <terminus/finance/order_book.hpp>
#include <iostream>

namespace finance {

PriceNode::PriceNode(double price, double size) : price(price), size(size), n_orders(1) { };
PriceNode::PriceNode(double price) : price(price), size(0), n_orders(0) { };

OrderBook::OrderBook(double tick_size, double min, double max) : m_tick_size(tick_size) {
    // constructor to pre-queue a bunch of nodes
    PriceNode* prev_bid = nullptr; // price is lower
    PriceNode* next_offer = nullptr; // for this the price will be higher
    PriceNode* bid;
    PriceNode* offer;

    while (min <= max) {
        bid = new PriceNode(min, 0); 
        offer = new PriceNode(min, 0);

        // connects bids
        if(prev_bid) prev_bid->next = bid;
        bid->prev = prev_bid;

        // connects offers
        if(next_offer) next_offer->prev = offer;
        offer->next = next_offer;

        m_bids.emplace(min, bid);
        m_offers.emplace(min, offer);
        min += tick_size;

        // update
        prev_bid = bid;
        next_offer = offer;
    }

};
OrderBook::OrderBook(double tick_size) : m_tick_size(tick_size) {};

double OrderBook::get_mid() {
    if(m_best_bid_node != nullptr && m_best_offer_node != nullptr) {
        return (m_best_bid_node->price + m_best_offer_node->price) / 2;
    }
    return -1;
}

double OrderBook::get_weighted_mid() {
    if(m_best_bid_node != nullptr && m_best_offer_node != nullptr) {
        return (m_best_bid_node->price * m_best_offer_node->size + m_best_offer_node->price * m_best_bid_node->size) / (m_best_bid_node->size + m_best_offer_node->size);
    }
    return -1;
}

double OrderBook::get_best_bid() {
    if (m_best_bid_node) {
        return m_best_bid_node->price;
    }
    return -1;
}
double OrderBook::get_best_bid_size() {
    if (m_best_bid_node) {
        return m_best_bid_node->size;
    }
    return -1;
}
double OrderBook::get_best_offer() {
    if (m_best_offer_node) {
        return m_best_offer_node->price;
    }
    return -1;
}
double OrderBook::get_best_offer_size() {
    if (m_best_offer_node) {
        return m_best_offer_node->size;
    }
    return -1;
}

PriceNode* OrderBook::get_best_bid_node() {
    return m_best_bid_node;
}
PriceNode* OrderBook::get_best_offer_node() {
    return m_best_offer_node;
}

std::vector<std::vector<double>> OrderBook::get_bids(int levels) {
    std::vector<std::vector<double>> bids;

    get_levels(levels, bids, m_best_bid_node);

    return bids;
}
std::vector<std::vector<double>> OrderBook::get_offers(int levels) {
    std::vector<std::vector<double>> offers;

    get_levels(levels, offers, m_best_offer_node);

    return offers;
}

void OrderBook::get_levels(int &levels, std::vector<std::vector<double>> &v, PriceNode* ptr) {
    std::vector<double> temp(2,0);
    if (levels != -1) {
        int i = 0;
        while(i <= levels && ptr) {
            temp[0] = ptr->price; 
            temp[1] = ptr->size;
            v.push_back(temp);
            ptr = ptr->prev;
            i++;
        }
    } else {
        while(ptr) {
            temp[0] = ptr->price; 
            temp[1] = ptr->size;
            v.push_back(temp);
            ptr = ptr->prev;
        }
    }
}


void OrderBook::update_bid(double price, double size) {
    std::unordered_map<double, PriceNode*>::const_iterator n = m_bids.find(price);
    PriceNode* node;
    if( n == m_bids.end() ) {
        // node price is not found
        node = new PriceNode(price, size);
        insert_bid_node(node);
    } else {
        node = n->second;
        if (size == 0) { 
            node->n_orders = 0;
            // need to update the bid next
            if (node == m_best_bid_node) {
                PriceNode* temp = node->prev;
                while(temp) {
                    if (temp->size > 0) {
                        m_best_bid_node = temp;
                        break;
                    } else {
                        temp = temp->prev;
                    }
                }
            }
        } else {
            if (size > node->size) {
                node->n_orders++;
            } else {
                node->n_orders--;
            }
            if(m_best_bid_node) {
                if (node->price > m_best_bid_node->price) m_best_bid_node = node;
            } else {
                m_best_bid_node = node;
            }
        }
        node->size = size;
    }
}
void OrderBook::update_offer(double price, double size) {
    std::unordered_map<double, PriceNode*>::const_iterator n = m_offers.find(price);
    PriceNode* node;
    if( n == m_bids.end() ) {
        // node price is not found
        node = new PriceNode(price, size);
        insert_offer_node(node);
    } else {
        node = n->second;
        if (size == 0) { 
            node->n_orders = 0;
            // need to update the offer next
            if (node == m_best_offer_node) {
                PriceNode* temp = node->prev;
                while(temp) {
                    if (temp->size > 0) {
                        m_best_offer_node = temp;
                        break;
                    } else {
                        temp = temp->prev;
                    }
                }
            }
        } else {
            if (size > node->size) {
                node->n_orders++;
            } else {
                node->n_orders--;
            }
            if(m_best_offer_node) {
                if (node->price < m_best_offer_node->price) m_best_offer_node = node;
            } else {
                m_best_offer_node = node;
            }
        }
        node->size = size;
    }
}

void OrderBook::clear_book() {
    m_bids.clear();
    m_offers.clear();
    m_best_bid_node = nullptr;
    m_best_offer_node = nullptr;
}

void OrderBook::insert_bid_node(PriceNode* node) {
    if(m_best_bid_node != nullptr) {
        PriceNode* prev;
        PriceNode* next;
        if(node->price > m_best_bid_node->price) {// is better bid {
            // walk the next prices until you find the spot
            prev = m_best_bid_node;
            next = m_best_bid_node->next;   
            while(next && next->price < node->price) {
                prev = next;
                next = next->next;
            }
        } else {
            prev = m_best_bid_node->prev;
            next = m_best_bid_node;
            while(prev && prev->price > node->price) {
                next = prev;
                prev = prev->prev;
            }
        }

        if(prev) prev->next = node;
        node->prev = prev;
        if(next) next->prev = node;
        node->next = next;
        if(node->price > m_best_bid_node->price) m_best_bid_node = node;
    } else {
        m_best_bid_node = node;
    }
    m_bids.emplace(node->price, node);
}

void OrderBook::insert_offer_node(PriceNode* node) {
    if(m_best_offer_node != nullptr) {
        PriceNode* prev;
        PriceNode* next;
        if(node->price < m_best_offer_node->price) {
            prev = m_best_offer_node;
            next = m_best_offer_node->next;
            while(next && next->price > node->price) {
                prev = next;
                next = next->next;
            }
        } else {
            prev = m_best_offer_node->prev;
            next = m_best_offer_node;
            while(prev && prev->price < node->price) {
                next = prev;
                prev = prev->prev;
            }
        }
        if(prev) prev->next = node;
        node->prev = prev;
        if(next) next->prev = node;
        node->next = next;

        if(node->price < m_best_offer_node->price) m_best_offer_node = node;
        
    } else {
        m_best_offer_node = node;
    }
    m_offers.emplace(node->price, node);
}


}
