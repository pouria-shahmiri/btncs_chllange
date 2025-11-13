#pragma once

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <memory>
#include "MBOParsed.hpp"
#include "OrderBookManager.hpp"

class MBOSubscriber : public eprosima::fastdds::dds::DataReaderListener {
private:
    eprosima::fastdds::dds::DomainParticipant* participant;
    eprosima::fastdds::dds::Subscriber* subscriber;
    eprosima::fastdds::dds::Topic* topic;
    eprosima::fastdds::dds::DataReader* reader;
    eprosima::fastdds::dds::TypeSupport type;
    
    int matched_publishers;
    int samples_received;
    
    // OrderBook manager
    std::unique_ptr<OrderBookManager> orderbook_mgr_;

public:
    MBOSubscriber();
    ~MBOSubscriber();
    
    bool init();
    void run();
    
    // DataReaderListener callbacks
    void on_data_available(eprosima::fastdds::dds::DataReader* reader) override;
    void on_subscription_matched(
        eprosima::fastdds::dds::DataReader* reader,
        const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;
    
private:
    MBOParsed parseCSVString(const std::string& csv_line);
    void printRecord(const MBOParsed& r);
};