#include "wled_light_effect.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace wled {

// Description of protocols:
// https://github.com/Aircoookie/WLED/wiki/UDP-Realtime-Control
enum Protocol { WLED_NOTIFIER = 0, WARLS = 1, DRGB = 2, DRGBW = 3, DNRGB = 4 };

const int DEFAULT_BLANK_TIME = 1000;

static const char *const TAG = "wled_light_effect";

WLEDLightEffect::WLEDLightEffect(const std::string &name) : AddressableLightEffect(name) {}

void WLEDLightEffect::start() {
  AddressableLightEffect::start();

  blank_at_ = 0;
}

void WLEDLightEffect::stop() {
  AddressableLightEffect::stop();

  if (this->socket_) {
    this->socket_->close();
    this->socket_.reset();
  }
}

void WLEDLightEffect::blank_all_leds_(light::AddressableLight &it) {
  for (int led = it.size(); led-- > 0;) {
    it[led].set(Color::BLACK);
  }
  it.schedule_show();
}

void WLEDLightEffect::apply(light::AddressableLight &it, const Color &current_color) {
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

  std::vector<uint8_t> payload;
  uint8_t buf[1460];

  ssize_t len = this->socket_->read(buf, sizeof(buf));
  if (len == -1) {
    return;
  }
  payload.resize(len);
  memmove(&payload[0], buf, len);

  if (!this->parse_frame_(it, &payload[0], payload.size())) {
    ESP_LOGD(TAG, "Frame: Invalid (size=%zu, first=0x%02X).", payload.size(), payload[0]);
  }

  // FIXME: Use roll-over safe arithmetic
  if (blank_at_ < millis()) {
    blank_all_leds_(it);
    blank_at_ = millis() + DEFAULT_BLANK_TIME;
  }
}

bool WLEDLightEffect::parse_frame_(light::AddressableLight &it, const uint8_t *payload, uint16_t size) {
  // At minimum frame needs to have:
  // 1b - protocol
  // 1b - timeout
  if (size < 2) {
    return false;
  }

  uint8_t protocol = payload[0];
  uint8_t timeout = payload[1];

  payload += 2;
  size -= 2;

  switch (protocol) {
    case WLED_NOTIFIER:
      // Hyperion Port
      if (port_ == 19446) {
        if (!parse_drgb_frame_(it, payload, size))
          return false;
      } else {
        if (!parse_notifier_frame_(it, payload, size))
          return false;
      }
      break;

    case WARLS:
      if (!parse_warls_frame_(it, payload, size))
        return false;
      break;

    case DRGB:
      if (!parse_drgb_frame_(it, payload, size))
        return false;
      break;

    case DRGBW:
      if (!parse_drgbw_frame_(it, payload, size))
        return false;
      break;

    case DNRGB:
      if (!parse_dnrgb_frame_(it, payload, size))
        return false;
      break;

    default:
      return false;
  }

  if (timeout == UINT8_MAX) {
    blank_at_ = UINT32_MAX;
  } else if (timeout > 0) {
    blank_at_ = millis() + timeout * 1000;
  } else {
    blank_at_ = millis() + DEFAULT_BLANK_TIME;
  }

  it.schedule_show();
  return true;
}

bool WLEDLightEffect::parse_notifier_frame_(light::AddressableLight &it, const uint8_t *payload, uint16_t size) {
  // Packet needs to be empty
  return size == 0;
}

bool WLEDLightEffect::parse_warls_frame_(light::AddressableLight &it, const uint8_t *payload, uint16_t size) {
  // packet: index, r, g, b
  if ((size % 4) != 0) {
    return false;
  }

  auto count = size / 4;
  auto max_leds = it.size();

  for (; count > 0; count--, payload += 4) {
    uint8_t led = payload[0];
    uint8_t r = payload[1];
    uint8_t g = payload[2];
    uint8_t b = payload[3];

    if (led < max_leds) {
      it[led].set(Color(r, g, b));
    }
  }

  return true;
}

bool WLEDLightEffect::parse_drgb_frame_(light::AddressableLight &it, const uint8_t *payload, uint16_t size) {
  // packet: r, g, b
  if ((size % 3) != 0) {
    return false;
  }

  auto count = size / 3;
  auto max_leds = it.size();

  for (uint16_t led = 0; led < count; ++led, payload += 3) {
    uint8_t r = payload[0];
    uint8_t g = payload[1];
    uint8_t b = payload[2];

    if (led < max_leds) {
      it[led].set(Color(r, g, b));
    }
  }

  return true;
}

bool WLEDLightEffect::parse_drgbw_frame_(light::AddressableLight &it, const uint8_t *payload, uint16_t size) {
  // packet: r, g, b, w
  if ((size % 4) != 0) {
    return false;
  }

  auto count = size / 4;
  auto max_leds = it.size();

  for (uint16_t led = 0; led < count; ++led, payload += 4) {
    uint8_t r = payload[0];
    uint8_t g = payload[1];
    uint8_t b = payload[2];
    uint8_t w = payload[3];

    if (led < max_leds) {
      it[led].set(Color(r, g, b, w));
    }
  }

  return true;
}

bool WLEDLightEffect::parse_dnrgb_frame_(light::AddressableLight &it, const uint8_t *payload, uint16_t size) {
  // offset: high, low
  if (size < 2) {
    return false;
  }

  uint16_t led = (uint16_t(payload[0]) << 8) + payload[1];
  payload += 2;
  size -= 2;

  // packet: r, g, b
  if ((size % 3) != 0) {
    return false;
  }

  auto count = size / 3;
  auto max_leds = it.size();

  for (; count > 0; count--, payload += 3, led++) {
    uint8_t r = payload[0];
    uint8_t g = payload[1];
    uint8_t b = payload[2];

    if (led < max_leds) {
      it[led].set(Color(r, g, b));
    }
  }

  return true;
}

}  // namespace wled
}  // namespace esphome