#pragma once

#include "esphome/core/component.h"

namespace esphome {
namespace udp_receiver {

class UdpReceiver : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
};


}  // namespace udp_receiver
}  // namespace esphome