#pragma once

#include "coap/coap_msg.h"

constexpr uint8_t EXCHANGE_ORIGIN_REMOTE = 0;
constexpr uint8_t EXCHANGE_ORIGIN_LOCAL  = 1;

class BaseInteraction;

namespace ja_iot {
namespace stack {
class Exchange
{
  public:
    Exchange( ja_iot::network::CoapMsg *in_message, uint8_t origin = EXCHANGE_ORIGIN_REMOTE );
    ~Exchange ();

    void acknowledge();
    void reject();
    void resend_reply();
    void reply_piggyback( ja_iot::network::CoapMsg *response );
    void deliver();

    bool             is_origin_remote() { return ( _origin == EXCHANGE_ORIGIN_REMOTE ); }
    bool             is_origin_local() { return ( _origin == EXCHANGE_ORIGIN_LOCAL ); }
    bool             is_msg_matched( ja_iot::network::CoapMsg & rcz_coap_msg );
    BaseInteraction* get_interaction() { return ( _interaction ); }
    void             set_interaction( BaseInteraction *interaction ) { _interaction = interaction; }

    bool     is_expired();
    void     initiate_retransmission();
    void     retransmit();
    uint32_t get_remaining_lifetime();
    bool     is_timedout() { return ( _retransmit_count > 4 ); }
    bool     is_con_outgoing() { return ( is_origin_local() && out_msg->is_confirmable() ); }

    ja_iot::network::CoapMsg* get_originator_msg();

  private:
    uint8_t                    _retransmit_count = 0;
    uint32_t                   _timeout_us       = 0;
    uint64_t                   _expiry_time_us   = 0;
    uint8_t                    _origin           = EXCHANGE_ORIGIN_LOCAL;
    ja_iot::network::CoapMsg * in_msg            = nullptr;
    ja_iot::network::CoapMsg * out_msg           = nullptr;
    BaseInteraction *          _interaction      = nullptr;
};
}
}