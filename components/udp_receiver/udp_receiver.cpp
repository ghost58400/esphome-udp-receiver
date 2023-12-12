#include "esphome/core/log.h"
#include "udp_receiver.h"

namespace esphome {
namespace udp_receiver {

static const char *TAG = "udp_receiver.component";

void UdpReceiver::setup() {
  // Init UDP lazily
  if (this->socket_ == nullptr) {
    this->socket_ = socket::socket_ip(SOCK_DGRAM, IPPROTO_IP);

    int enable = 1;
    int err = this->socket_->setsockopt(SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (err != 0) {
      ESP_LOGW(TAG, "Socket unable to set reuseaddr: errno %d", err);
      // we can still continue
    }
    err = this->socket_->setblocking(false);
    if (err != 0) {
      ESP_LOGW(TAG, "Socket unable to set nonblocking mode: errno %d", err);
      return;
    }

    struct sockaddr_storage server;

    socklen_t sl = socket::set_sockaddr_any((struct sockaddr *) &server, sizeof(server), this->port_);
    if (sl == 0) {
      ESP_LOGW(TAG, "Socket unable to set sockaddr: errno %d", errno);
      return;
    }
    server.ss_family = AF_INET;

    err = this->socket_->bind((struct sockaddr *) &server, sizeof(server));
    if (err != 0) {
      ESP_LOGW(TAG, "Socket unable to bind: errno %d", errno);
      return;
    }
  }
}

void UdpReceiver::loop() {
  uint8_t buf[1460];

  ssize_t len = this->socket_->read(buf, sizeof(buf));
  if (len == -1) {
    return;
  }
  if (len > 0) {
    // Assuming received data is null-terminated string
    std::string receivedString(reinterpret_cast<char*>(buf), len);

    // Process the received string as needed
    ESP_LOGW(TAG, "UDP frame received :  %s", receivedString.c_str());
  }

}

void UdpReceiver::dump_config(){
    ESP_LOGCONFIG(TAG, "Empty component");
}


}  // namespace udp_receiver
}  // namespace esphome