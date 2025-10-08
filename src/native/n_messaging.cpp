#include "native/n_bmmpi.hpp"


bool construct_message(unsigned int dest_id, unsigned int source_id, unsigned int data, FIFOMessage* msg_out) {
    if (dest_id > FIFO_DEST_ID_MAX || source_id > FIFO_SOURCE_ID_MAX || data > FIFO_DATA_MAX) {
        return false; // Invalid parameters
    }
    msg_out->dest_id = dest_id;
    msg_out->source_id = source_id;
    msg_out->data = data;
    return true;
}


uint32_t message_to_uint32(FIFOMessage msg) {
    uint32_t raw = 0;
    raw |= msg.dest_id;
    raw |= (msg.source_id << FIFO_DEST_ID_SIZE);
    raw |= (msg.data << (FIFO_DEST_ID_SIZE + FIFO_SOURCE_ID_SIZE));

    return raw;
}


FIFOMessage message_from_uint32(uint32_t raw) {
    FIFOMessage msg;
    msg.dest_id = raw & FIFO_DEST_ID_MAX;
    msg.source_id = (raw >> FIFO_DEST_ID_SIZE) & FIFO_SOURCE_ID_MAX;
    msg.data = (raw >> (FIFO_DEST_ID_SIZE + FIFO_SOURCE_ID_SIZE)) & FIFO_DATA_MAX;

    return msg;
}


bool ReceiverRouter::register_handler(unsigned int id, ReceiverTask task) {
    if (id > FIFO_DEST_ID_MAX || handlers[id] != nullptr) {
        return false; // Invalid id or handler already registered
    }
    handlers[id] = task;
    this->registered_count++;
    return true;
}


bool ReceiverRouter::route_message(FIFOMessage msg) {
    if (msg.dest_id > FIFO_DEST_ID_MAX || handlers[msg.dest_id] == nullptr) {
        return false; // Invalid destination or no handler registered
    }
    handlers[msg.dest_id](msg);
    return true;
}


bool ReceiverRouter::is_full() {
    return this->registered_count >= (FIFO_DEST_ID_MAX + 1);
}


unsigned int ReceiverRouter::get_registered_count() {
    return this->registered_count;
}