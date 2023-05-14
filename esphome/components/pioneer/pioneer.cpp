#include <vector>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <math.h>
#include <map>
#include "pioneer_climate.h


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

double toC(double fahrenheit) {
    return (fahrenheit - 32.0) * (5.0 / 9.0);
}

double toF(double celsius) {
    return (celsius * (9.0 / 5.0)) + 32.0;
}

double toNearestQuarter(double num) {
    return floor(num * 4) / 4;
}

std::vector<unsigned char> tempToPioneerHex(double celsius) {
    celsius = toNearestQuarter(celsius);
    unsigned char first_nibble = static_cast<unsigned char>(31 - static_cast<int>(celsius));
    std::unordered_map<double, unsigned char> final_nibble_options = {
        {0.0, 0x0}, {0.25, 0x4}, {0.5, 0x8}, {0.75, 0xc}
    };
    unsigned char final_nibble = final_nibble_options[celsius - floor(celsius)];
    return {first_nibble, final_nibble};
}


unsigned char nibbleToHexInt(unsigned char nibble) {
    return static_cast<unsigned char>(nibble);
}


double fromPioneerHex(unsigned char first_nibble, unsigned char last_nibble) {
    std::unordered_map<unsigned char, double> final_nibble_options = {
        {0x0, 0.0}, {0x4, 0.25}, {0x8, 0.5}, {0xc, 0.75}
    };
    return (31.0 - static_cast<double>(first_nibble)) + final_nibble_options[last_nibble];
}

unsigned char calc_xor_checksum(const std::vector<unsigned char>& my_bytes) {
    unsigned char result = 0;
    for (const auto& byte : my_bytes) {
        result ^= byte;
    }
    return result;
}

bool check_xor_checksum(std::vector<unsigned char> my_bytes) {
    unsigned char current_checksum = my_bytes.back();
    my_bytes.pop_back();
    unsigned char result = 0;
    for (const auto& byte : my_bytes) {
        result ^= byte;
    }
    return result == current_checksum;
}

std::vector<unsigned char> get_unknown_message(int num) {
    std::vector<std::vector<unsigned char>> messages = {
        {0xbb, 0x00, 0x01, 0x04, 0x02, 0x01, 0x00, 0xbd},
        {0xbb, 0x00, 0x01, 0x0a, 0x03, 0x05, 0x00, 0x00, 0xb6},
        {0xbb, 0x00, 0x01, 0x09, 0x02, 0x05, 0x00, 0xb4},
        {0xbb, 0x00, 0x01, 0x0a, 0x03, 0x05, 0x00, 0x08, 0xbe}
    };
    return messages[num - 1];
}

std::vector<unsigned char> generate_message(Mode mode, double temp_celsius, WindSpeed wind_speed = WindSpeed::AUTO, FanUpDown up_down_mode = FanUpDown::AUTO, FanLeftRight left_right_mode = FanLeftRight::AUTO, SleepMode sleep_mode = SleepMode::OFF, bool is_on = true, bool is_display_on = true, bool is_buzzer_on = true, bool is_eco = false, bool is_8_deg_heater = false, bool is_health_on = false)
{
        if (temp_celsius > 31 || temp_celsius < 16) {
        std::cout << "Temperature must be between 16 and 31 degrees celsius\n";
        return std::vector<unsigned char>();
    }

    std::vector<unsigned char> message = {0xbb, 0x00, 0x01, 0x03};
    std::vector<unsigned char> command = {0x00, 0x00, 0x00, 0x00, 0x5c, 0x00, 0x04, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x99};

    if (is_on) {
        command[3] = command[3] | 0x04;
    }
    
    if (is_display_on) {
        command[3] = command[3] | 0x40;
    }

    if (is_buzzer_on) {
        command[3] = command[3] | 0x20;
    }

    if (is_eco) {
        command[3] = command[3] | 0x80;
    }

    if (is_8_deg_heater) {
        command[6] = command[6] | 0x80;
    }

    if (is_health_on) {
        command[4] = command[4] | 0x10;
    }

    command[15] = command[15] | static_cast<unsigned char>(sleep_mode);

    if (wind_speed == WindSpeed::AUTO) {
    } else if (wind_speed == WindSpeed::MUTE) {
        command[4] = command[4] | 0x80;
        command[6] = command[6] | 0x02;
    } else if (wind_speed == WindSpeed::SIX) {
        command[4] = command[4] | 0x40;
        command[6] = command[6] | 0x05;
    } else {
        command[4] = command[4] & ~0xc0;
        std::map<int, unsigned char> speeds = {{1, 0x2}, {2, 0x06}, {3, 0x03}, {4, 0x07}, {5, 0x05}};
        command[6] = command[6] | speeds[static_cast<int>(wind_speed)];
    }

    command[4] = command[4] | static_cast<unsigned char>(mode);

    std::vector<unsigned char> temp_bytes = tempToPioneerHex(temp_celsius);
    command[9] = command[9] | temp_bytes[0];
    command[11] = command[11] | temp_bytes[1];

    if (up_down_mode == FanUpDown::UP_DOWN_FLOW || up_down_mode == FanUpDown::UP_FLOW || up_down_mode == FanUpDown::DOWN_FLOW) {
        command[6] = command[6] | 0x38;
    }

    command[28] = command[28] | static_cast<unsigned char>(up_down_mode);

    if (left_right_mode == FanLeftRight::LEFT_RIGHT_FLOW || left_right_mode == FanLeftRight::LEFT_FLOW || left_right_mode == FanLeftRight::MIDDLE_FLOW || left_right_mode == FanLeftRight::RIGHT_FLOW) {
        command[7] = command[7] | 0x8;
    }

    command[29] = command[29] | static_cast<unsigned char>(left_right_mode);

    for (auto &c : command) {
        message.push_back(c);
    }

    message.push_back(calc_xor_checksum(message));

    return message;
}

int main() {
    // Generate a message with the provided command syntax
    std::vector<unsigned char> message = generate_message(Mode::HEAT, 20);
    for (const auto& byte : message) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return 0;
}
