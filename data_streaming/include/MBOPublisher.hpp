#pragma once
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include "MBOParsed.hpp"
#include <string>


class MBOPublisher {
public:
    MBOPublisher(const std::string& topic_name = "MBOTopic");
    ~MBOPublisher();

    bool init();
    void publish(const MBOParsed& record);

private:
    eprosima::fastdds::dds::DomainParticipant* participant;
    eprosima::fastdds::dds::Publisher* publisher;
    eprosima::fastdds::dds::Topic* topic;
    eprosima::fastdds::dds::DataWriter* writer;
    eprosima::fastdds::dds::TypeSupport type;
};