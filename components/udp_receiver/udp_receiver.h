#pragma once

#include "esphome/components/socket/socket.h"
#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace udp_receiver {

class UdpReceiver : public text_sensor::TextSensor, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

 protected:
  uint16_t port_{0};
  std::unique_ptr<socket::Socket> socket_;
};


}  // namespace udp_receiver
}  // namespace esphome