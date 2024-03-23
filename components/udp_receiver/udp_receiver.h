#pragma once

#include "esphome/components/socket/socket.h"
#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"

#ifdef USE_ARDUINO
class UDP;
#endif  // USE_ARDUINO

namespace esphome {
namespace udp_receiver {

class UdpReceiver : public text_sensor::TextSensor, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  void set_port(uint16_t port) { this->port_ = port; }
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

 protected:
  uint16_t port_{0};
  std::unique_ptr<socket::Socket> socket_;
  #ifdef USE_ARDUINO
  std::unique_ptr<UDP> udp_;
  #endif  // USE_ARDUINO
};


}  // namespace udp_receiver
}  // namespace esphome
