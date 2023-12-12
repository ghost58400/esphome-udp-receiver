#include "esphome/core/log.h"
#include "udp_receiver.h"

namespace esphome {
namespace udp_receiver {

static const char *TAG = "udp_receiver.component";

void UdpReceiver::setup() {

}

void UdpReceiver::loop() {

}

void UdpReceiver::dump_config(){
    ESP_LOGCONFIG(TAG, "Empty component");
}


}  // namespace udp_receiver
}  // namespace esphome