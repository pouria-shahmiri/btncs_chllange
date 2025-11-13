#include "MBOPublisher.hpp"
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <iostream>
#include <sstream>

// Simple string type for FastDDS
class StringType : public eprosima::fastdds::dds::TopicDataType {
public:
    StringType() {
        setName("StringType");
        m_typeSize = 4096;  // max string size
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

MBOPublisher::MBOPublisher(const std::string& topic_name)
    : participant(nullptr), publisher(nullptr), topic(nullptr), writer(nullptr) 
{
    type.reset(new StringType());
}

MBOPublisher::~MBOPublisher() {
    if (writer) publisher->delete_datawriter(writer);
    if (topic) participant->delete_topic(topic);
    if (publisher) participant->delete_publisher(publisher);
    if (participant) {
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);
    }
}

bool MBOPublisher::init() {
    using namespace eprosima::fastdds::dds;

    DomainParticipantQos pqos;
    pqos.name("MBOPublisher_Participant");
    
    participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (!participant) {
        std::cerr << "Failed to create participant" << std::endl;
        return false;
    }

    // Register the type
    type.register_type(participant);

    publisher = participant->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (!publisher) {
        std::cerr << "Failed to create publisher" << std::endl;
        return false;
    }

    TopicQos tqos = TOPIC_QOS_DEFAULT;
    topic = participant->create_topic("MBOTopic", type.get_type_name(), tqos);
    if (!topic) {
        std::cerr << "Failed to create topic" << std::endl;
        return false;
    }

    DataWriterQos wqos = DATAWRITER_QOS_DEFAULT;
    wqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    
    writer = publisher->create_datawriter(topic, wqos, nullptr);
    if (!writer) {
        std::cerr << "Failed to create datawriter" << std::endl;
        return false;
    }

    std::cout << "DDS Publisher initialized successfully" << std::endl;
    return true;
}

void MBOPublisher::publish(const MBOParsed& record) {
    // Convert MBOParsed to string (CSV format)
    std::ostringstream oss;
    oss << record.ts_event_str << "," 
        << int(record.rtype) << "," 
        << record.publisher_id << "," 
        << record.instrument_id << "," 
        << record.action << "," 
        << record.side << "," 
        << record.price << "," 
        << record.size << "," 
        << int(record.channel_id) << "," 
        << record.order_id << "," 
        << int(record.flags) << "," 
        << record.ts_in_delta << "," 
        << record.sequence << "," 
        << record.symbol << "," 
        << record.datetime;

    std::string message = oss.str();
    writer->write(&message);
}