#pragma once

#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/uart/uart.h"
#include <vector>
#include <bitset>
#include <unordered_map>

namespace esphome {
namespace pioneer {

enum class SleepMode : unsigned char {
    OFF = 0x0,
    STANDARD = 0x1,
    THE_AGED = 0x2,
    CHILD = 0x3
};

enum class WindSpeed : unsigned char {
    AUTO = 0,
    ONE = 1,
    TWO = 2,
    THREE = 3,
    FOUR = 4,
    FIVE = 5,
    SIX = 6,
    MUTE = 7
};

enum class Mode : unsigned char {
    HEAT = 0x1,
    DEHUMIDIFY = 0x2,
    COOL = 0x3,
    FAN = 0x7,
    AUTO = 0x8
};

enum class FanUpDown : unsigned char {
    AUTO = 0x0,
    UP_DOWN_FLOW = 0x18,
    UP_FLOW = 0x10,
    DOWN_FLOW = 0x08,
    TOP_FIX = 0x01,
    UPPER_FIX = 0x02,
    MIDDLE_FIX = 0x03,
    ABOVE_DOWN_FIX = 0x04,
    BOTTOM_FIX = 0x05
};

enum class FanLeftRight : unsigned char {
    AUTO = 0x0,
    LEFT_RIGHT_FLOW = 0x08,
    LEFT_FLOW = 0x10,
    MIDDLE_FLOW = 0x18,
    RIGHT_FLOW = 0x20,
    LEFT_FIX = 0x01,
    LEFT_MIDDLE_FIX = 0x02,
    MIDDLE_FIX = 0x3,
    RIGHT_MIDDLE_FIX = 0x4,
    RIGHT_FIX = 0x5
};


class pioneerClimate : public climate::Climate, public uart::UARTDevice, public PollingComponent {
 public:
  void loop() override;
  void update() override;
  void dump_config() override;
  void control(const climate::ClimateCall &call) override;
  void set_supported_swing_modes(const std::set<climate::ClimateSwingMode> &modes) {
    this->supported_swing_modes_ = modes;
  }

  double toC(double fahrenheit);
  double toF(double celsius);
  double toNearestQuarter(double num);
  std::vector<unsigned char> tempToPioneerHex(double celsius);
  unsigned char nibbleToHexInt(unsigned char nibble);
  double fromPioneerHex(unsigned char first_nibble, unsigned char last_nibble);
  unsigned char calc_xor_checksum(const std::vector<unsigned char>& my_bytes);
  bool check_xor_checksum(std::vector<unsigned char> my_bytes);
  std::vector<unsigned char> get_unknown_message(int num);
  std::vector<unsigned char> generate_message(Mode mode, double temp_celsius, WindSpeed wind_speed, FanUpDown up_down_mode, FanLeftRight left_right_mode, SleepMode sleep_mode, bool is_on, bool is_display_on, bool is_buzzer_on, bool is_eco, bool is_8_deg_heater, bool is_health_on);

 protected:
  climate::ClimateTraits traits() override;
  void read_state_(const uint8_t *data, uint8_t size);
  void send_data_(const uint8_t *message, uint8_t size);
  void dump_message_(const char *title, const uint8_t *message, uint8_t size);
  uint8_t get_checksum_(const uint8_t *message, size_t size);

 private:
  uint8_t data_[37];
  std::set<climate::ClimateSwingMode> supported_swing_modes_{};
};

}  // namespace pioneer
}  // namespace esphome
