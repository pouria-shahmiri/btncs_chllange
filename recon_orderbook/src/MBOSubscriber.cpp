#include "MBOSubscriber.hpp"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>

// Simple string type for FastDDS (same as publisher)
class StringType : public eprosima::fastdds::dds::TopicDataType {
public:
    StringType() {
        setName("StringType");
        m_typeSize = 4096;
        m_isGetKeyDefined = false;
    }

    bool serialize(void* data, eprosima::fastrtps::rtps::SerializedPayload_t* payload) override {
        std::string* str = static_cast<std::string*>(data);
        payload->length = static_cast<uint32_t>(str->size());
        memcpy(payload->data, str->data(), str->size());
        return true;
    }

    bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t* payload, void* data) override {
        std::string* str = static_cast<std::string*>(data);
        str->assign(reinterpret_cast<char*>(payload->data), payload->length);
        return true;
    }

    std::function<uint32_t()> getSerializedSizeProvider(void* data) override {
        return [data]() -> uint32_t {
            return static_cast<uint32_t>(static_cast<std::string*>(data)->size());
        };
    }

    void* createData() override {
        return new std::string();
    }

    void deleteData(void* data) override {
        delete static_cast<std::string*>(data);
    }

    bool getKey(void*, eprosima::fastrtps::rtps::InstanceHandle_t*, bool) override {
        return false;
    }
};

MBOSubscriber::MBOSubscriber()
    : participant(nullptr), subscriber(nullptr), topic(nullptr), reader(nullptr),
      matched_publishers(0), samples_received(0) 
{
    type.reset(new StringType());
}

MBOSubscriber::~MBOSubscriber() {
    if (reader) subscriber->delete_datareader(reader);
    if (topic) participant->delete_topic(topic);
    if (subscriber) participant->delete_subscriber(subscriber);
    if (participant) {
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);
    }
}

bool MBOSubscriber::init() {
    using namespace eprosima::fastdds::dds;

    DomainParticipantQos pqos;
    pqos.name("MBOSubscriber_Participant");
    
    participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (!participant) {
        std::cerr << "Failed to create participant" << std::endl;
        return false;
    }

    // Register the type
    type.register_type(participant);

    subscriber = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (!subscriber) {
        std::cerr << "Failed to create subscriber" << std::endl;
        return false;
    }

    TopicQos tqos = TOPIC_QOS_DEFAULT;
    topic = participant->create_topic("MBOTopic", type.get_type_name(), tqos);
    if (!topic) {
        std::cerr << "Failed to create topic" << std::endl;
        return false;
    }

    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    
    reader = subscriber->create_datareader(topic, rqos, this);
    if (!reader) {
        std::cerr << "Failed to create datareader" << std::endl;
        return false;
    }

    std::cout << "DDS Subscriber initialized successfully" << std::endl;
    return true;
}

void MBOSubscriber::on_subscription_matched(
    eprosima::fastdds::dds::DataReader* reader,
    const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) 
{
    if (info.current_count_change == 1) {
        matched_publishers++;
        std::cout << "Publisher matched. Total publishers: " << matched_publishers << std::endl;
    } else if (info.current_count_change == -1) {
        matched_publishers--;
        std::cout << "Publisher unmatched. Total publishers: " << matched_publishers << std::endl;
    }
}

void MBOSubscriber::on_data_available(eprosima::fastdds::dds::DataReader* reader) {
    eprosima::fastdds::dds::SampleInfo info;
    std::string message;
    
    while (reader->take_next_sample(&message, &info) == ReturnCode_t::RETCODE_OK) {
        if (info.valid_data) {
            samples_received++;
            
            // Parse the CSV string back to MBOParsed
            MBOParsed record = parseCSVString(message);
            
            // Print the record
            printRecord(record);
            
            std::cout << "[Sample #" << samples_received << "]" << std::endl;
        }
    }
}

MBOParsed MBOSubscriber::parseCSVString(const std::string& csv_line) {
    MBOParsed record{};
    std::stringstream ss(csv_line);
    std::string field;
    std::vector<std::string> fields;
    
    while (std::getline(ss, field, ',')) {
        fields.push_back(field);
    }
    
    if (fields.size() >= 15) {
        record.ts_event_str = fields[0];
        record.rtype = static_cast<uint8_t>(std::stoi(fields[1]));
        record.publisher_id = static_cast<uint16_t>(std::stoi(fields[2]));
        record.instrument_id = static_cast<uint32_t>(std::stoul(fields[3]));
        record.action = fields[4][0];
        record.side = fields[5][0];
        record.price = std::stod(fields[6]);
        record.size = static_cast<uint32_t>(std::stoul(fields[7]));
        record.channel_id = static_cast<uint8_t>(std::stoi(fields[8]));
        record.order_id = std::stoull(fields[9]);
        record.flags = static_cast<uint8_t>(std::stoi(fields[10]));
        record.ts_in_delta = std::stoi(fields[11]);
        record.sequence = static_cast<uint32_t>(std::stoul(fields[12]));
        record.symbol = fields[13];
        record.datetime = fields[14];
    }
    
    return record;
}

void MBOSubscriber::printRecord(const MBOParsed& r) {
    std::cout << std::fixed << std::setprecision(2)
              << "ts_event=" << r.ts_event_str
              << " rtype=" << int(r.rtype)
              << " publisher_id=" << r.publisher_id
              << " instrument_id=" << r.instrument_id
              << " action=" << r.action
              << " side=" << r.side
              << " price=" << r.price
              << " size=" << r.size
              << " channel_id=" << int(r.channel_id)
              << " order_id=" << r.order_id
              << " flags=" << int(r.flags)
              << " ts_in_delta=" << r.ts_in_delta
              << " sequence=" << r.sequence
              << " symbol=" << r.symbol
              << " datetime=" << r.datetime
              << std::endl;
}

void MBOSubscriber::run() {
    std::cout << "Waiting for samples... Press Ctrl+C to exit." << std::endl;
    
    // Keep running until user stops
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}