#pragma once

#include "esphome/components/socket/socket.h"
#include "esphome/core/component.h"

namespace esphome {
namespace udp_receiver {

class UdpReceiver : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

 protected:
  uint16_t port_{0};
  std::unique_ptr<socket::Socket> socket_;
};


}  // namespace udp_receiver
}  // namespace esphome