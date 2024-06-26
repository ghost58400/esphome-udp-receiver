#include "esphome/core/log.h"
#include "udp_receiver.h"

#ifdef USE_ARDUINO
#ifdef USE_ESP32
#include <WiFi.h>
#endif

#ifdef USE_ESP8266
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#endif

#ifdef USE_BK72XX
#include <WiFiUdp.h>
#endif
#endif  // USE_ARDUINO

namespace esphome {
namespace udp_receiver {

static const char *TAG = "udp_receiver";

void UdpReceiver::setup() {

  #ifdef USE_ARDUINO
  if (!udp_) {
    udp_ = make_unique<WiFiUDP>();

    if (!udp_->begin(this->port_)) {
      ESP_LOGW(TAG, "Cannot bind to %d.", this->port_);
      return;
    }
  }
  #endif  // USE_ARDUINO

  #ifndef USE_ARDUINO
  // Init UDP lazily
  this->socket_ = socket::socket_ip(SOCK_DGRAM, IPPROTO_IP);
  if (socket_ == nullptr) {
    ESP_LOGW(TAG, "Could not create socket");
    this->mark_failed();
    return;
  }

  int enable = 1;
  int err = this->socket_->setsockopt(SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
  if (err != 0) {
    ESP_LOGW(TAG, "Socket unable to set reuseaddr: errno %d", err);
    // we can still continue
  }
  err = this->socket_->setblocking(false);
  if (err != 0) {
    ESP_LOGW(TAG, "Socket unable to set nonblocking mode: errno %d", err);
    this->mark_failed();
    return;
  }

  struct sockaddr_storage server;

  socklen_t sl = socket::set_sockaddr_any((struct sockaddr *) &server, sizeof(server), this->port_);
  if (sl == 0) {
    ESP_LOGW(TAG, "Socket unable to set sockaddr: errno %d", errno);
    this->mark_failed();
    return;
  }
  server.ss_family = AF_INET;

  err = this->socket_->bind((struct sockaddr *) &server, sizeof(server));
  if (err != 0) {
    ESP_LOGW(TAG, "Socket unable to bind: errno %d", errno);
    this->mark_failed();
    return;
  }
  #endif  // not USE_ARDUINO
}

void UdpReceiver::loop() {
  uint8_t buf[1024];

  #ifdef USE_ARDUINO
  this->udp_->parsePacket();
  ssize_t len = this->udp_->read(buf, sizeof(buf));
  #endif  // USE_ARDUINO
  #ifndef USE_ARDUINO
  ssize_t len = this->socket_->read(buf, sizeof(buf));
  #endif  // not USE_ARDUINO
  
  if (len == -1) {
    return;
  }
  if (len > 0) {
    ESP_LOGV(TAG, "UDP frame received, converting to str...");
    // Assuming received data is null-terminated string
    std::string receivedString(reinterpret_cast<char*>(buf), len);

    // Process the received string as needed
    ESP_LOGI(TAG, "UDP frame received :  %s", receivedString.c_str());
    this->publish_state(receivedString);
  }
}

void UdpReceiver::dump_config(){
    LOG_TEXT_SENSOR("", "UDP Receiver Text Sensor", this);
    ESP_LOGCONFIG(TAG, "  Port: %u", this->port_);
}


}  // namespace udp_receiver
}  // namespace esphome
