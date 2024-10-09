#include "message.hpp"

Message::Message(const MessageMetadata& msg_metadata, const MessageText& msg_text) : 
    msg_metadata(msg_metadata), msg_text(msg_text) 
{} 

nlohmann::json Message::to_json() const {
    nlohmann::json j;
    j.update(msg_metadata.to_json());
    j.update(msg_text.to_json());

    return j;
}