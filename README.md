# Public Documentation of my Crypto Trading System
## Overview
This repository is to outline my efforts for designing a crypto HFT system written predominantly in C++. While the majority of the code is kept in a private repository, I am including code-samples here to outline specific challenges I faced while building the system. I will keep this document up to date as I implement new components.

Ultimately, my goal is to have a functioning crypto trading system that works with a variety of exchanges and can trade spot currencies with reasonable latencies.

Outline:
* [Current Progress](#current-progress) - where I am at in this endeavor
* [System Design](#system-design) - the system design workflow and high-level implementation details
* [Finance](#finance) - implementation of finance-specific aspects (e.g. local Limit Order Book)
* [ETL](#etl) - ETL description(s); data recording, coin updates, etc.

## Current Progress
2022-05-22 - Implemented OrderBook finance object to manage local LOB representation 
2022-05-20 - Wrote FTX specific parser for trades, top\_of\_book, and order book updates
2022-05-10 - Added additional exchange support and re-wrote general websocket connector
2022-05-05 - Expanded exchanges to record (as of writing this includes: Binance, Binance-US, FTX, FTX-US, Mango, and DYDX)
2022-04-27 - First implementation of websocket connector and a file writer class; began exchange data message recordings

## System Design
![SystemDiagram1](https://user-images.githubusercontent.com/61852120/166719907-06c56249-222e-4eda-9e9e-b58a29e668eb.PNG)

### WebsocketCnx
In order to connect to exchanges' price feeds, I wrote a wrapper class utilizing [Websocketpp](https://github.com/zaphoyd/websocketpp) to create a generalizeable websocket connection. In order to handle the messages as they come in, the class has a private message callback lambda function that can be specified for different use cases. For example, the linux services I have implemented use lambda functions that write each message to a flat file specified. An example of this is:
```C++
exchanges::general::cnx ws(uri);
utils::files::syncronized_writer writer(path); // can also include subsciption messages in alternate constructor to subscribe to specific channels
ws.set_message_cb([&writer](std::string &msg) {
    writer.write_msg(msg); // in the production recorder I also add information regarding when I received the message separated by a '|'
});
```

### Parser
For each exchange, I leverage [RapidJSON](https://github.com/Tencent/rapidjson) to parse each different message type. The parsers are meant to pass messages through a lambda callback to push the parsed updates to the proper endpoint (e.g. order book updates go to an OrderBook object). 


## Finance
### Limit Order Book Representation
My implementation of the limit order book lives in [finance/order\_book.hpp](finance/order_book.hpp) and [order\_book.cc](finance/order_book.cc). For each side of the book (bids/offers), I utilize a doubly linked-list and an unordered_map to manage pointers to price nodes in the linked-list. The idea is that for each update to the book, either the price level has been pre-cached in the map or it creates a new one and inserts it in the linked list. 
In theory, this implementation is "self-balancing" in that the pointer, head, node is the current best bid or offer. By moving in the "next" direction the price improves and moving the "prev" direction the price gets worse. Therefore, by starting any search at the top of book the average cost hopefully is n/2 (rather than the worst case of O(n). 
If a price gets zeroed out or the top of book changes, the tree will rebalance itself to adjust the pointer to the new top of book node.

For bids, a sample impmlementation may look like:
![order_book_bids](https://user-images.githubusercontent.com/61852120/169717978-a4364769-73c5-4e28-b769-ad7242e67318.PNG)..
Each of the bid nodes (27999, 28000, and 28001) is mapped from the price level to a node pointer in an unordered map specific to the bids. Additionally, there is a pointer to the 28,000 node which is the current best bid. The next pointer always points to the price in the better direction (e.g. 28,001, or higher, if it gets an order will become next bid)

