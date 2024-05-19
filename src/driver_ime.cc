#include "Meter.h"
#include <sstream>
#include <iomanip>

class MeterIME : public Meter {
public:
    MeterIME() {
        di.addDetection(MANUFACTURER_IME, 0x08, 0x55); // Adjust as per your meter's details
    }

    void processContent(const std::vector<uint8_t> &data, const struct wmbusframe &frame) override {
        if(data.size() < 11) return;

        // Decode and print the secondary address
        std::string secondaryAddress = decodeSecondaryAddress(data);
        std::cout << "Secondary Address: " << secondaryAddress << std::endl;

        for(size_t i = 0; i < data.size(); i += 11) {
            std::vector<uint8_t> field(data.begin() + i, data.begin() + i + 11);

            if (fieldMatch(field, {0x84, 0x90, 0x10, 0xFF, 0x80, 0x84, 0x3B})) {
                addValue(field, 0.01, "Positive Three-phase Active Energy (Total)", "kWh");
            } else if (fieldMatch(field, {0x84, 0x90, 0x10, 0xFF, 0x80, 0x84, 0x3C})) {
                addValue(field, 0.01, "Negative Three-phase Active Energy (Total)", "kWh");
            } else if (fieldMatch(field, {0x84, 0x90, 0x10, 0xFF, 0x81, 0x84, 0x3B})) {
                addValue(field, 0.01, "Positive Three-phase Reactive Energy (Total)", "kvarh");
            } else if (fieldMatch(field, {0x84, 0x90, 0x10, 0xFF, 0x81, 0x84, 0x3C})) {
                addValue(field, 0.01, "Negative Three-phase Reactive Energy (Total)", "kvarh");
            } else if (fieldMatch(field, {0x84, 0xB0, 0x10, 0xFF, 0x84, 0x2B})) {
                addValue(field, 1, "Three-phase Total Active Power", "W");
            } else if (fieldMatch(field, {0x84, 0x80, 0x20, 0xFF, 0x84, 0x2B})) {
                addValue(field, 1, "Active Power L1", "W");
            } else if (fieldMatch(field, {0x84, 0x90, 0x20, 0xFF, 0x84, 0x2B})) {
                addValue(field, 1, "Active Power L2", "W");
            } else if (fieldMatch(field, {0x84, 0xA0, 0x20, 0xFF, 0x84, 0x2B})) {
                addValue(field, 1, "Active Power L3", "W");
            } else if (fieldMatch(field, {0x84, 0x80, 0x20, 0xFF, 0x87, 0x48})) {
                addValue(field, 0.1, "1-N Voltage", "V");
            } else if (fieldMatch(field, {0x84, 0x90, 0x20, 0xFF, 0x87, 0x48})) {
                addValue(field, 0.1, "2-N Voltage", "V");
            } else if (fieldMatch(field, {0x84, 0xA0, 0x20, 0xFF, 0x87, 0x48})) {
                addValue(field, 0.1, "3-N Voltage", "V");
            } else if (fieldMatch(field, {0x84, 0x80, 0x20, 0xFF, 0x88, 0x48})) {
                addValue(field, 0.1, "1-2 Voltage", "V");
            } else if (fieldMatch(field, {0x84, 0x90, 0x20, 0xFF, 0x88, 0x48})) {
                addValue(field, 0.1, "2-3 Voltage", "V");
            } else if (fieldMatch(field, {0x84, 0xA0, 0x20, 0xFF, 0x88, 0x48})) {
                addValue(field, 0.1, "3-1 Voltage", "V");
            } else if (fieldMatch(field, {0x84, 0x80, 0x20, 0xFF, 0x89, 0x59})) {
                addValue(field, 0.001, "Phase 1 Current Value", "A");
            } else if (fieldMatch(field, {0x84, 0x90, 0x20, 0xFF, 0x89, 0x59})) {
                addValue(field, 0.001, "Phase 2 Current Value", "A");
            } else if (fieldMatch(field, {0x84, 0xA0, 0x20, 0xFF, 0x89, 0x59})) {
                addValue(field, 0.001, "Phase 3 Current Value", "A");
            } else if (fieldMatch(field, {0x02, 0xFF, 0x8A, 0x48})) {
                addValue(field, 0.1, "Frequency", "Hz");
            } else if (fieldMatch(field, {0x82, 0xB0, 0x10, 0xFF, 0x8B, 0x28})) {
                addValue(field, 0.001, "Three-phase Power Factor (PF)", "");
            } else if (fieldMatch(field, {0x84, 0x10, 0xFF, 0x8D, 0x2B})) {
                addValue(field, 1, "Total Active Power Requirement (MD)", "W");
            } else if (fieldMatch(field, {0x84, 0x10, 0xFF, 0x8E, 0x2B})) {
                addValue(field, 1, "Maximum Total Active Power Requirement Tariff 1 (PMD T1)", "W");
            } else if (fieldMatch(field, {0x84, 0x20, 0xFF, 0x8E, 0x2B})) {
                addValue(field, 1, "Maximum Total Active Power Requirement Tariff 2 (PMD T2)", "W");
            } else if (fieldMatch(field, {0x84, 0xB0, 0x10, 0xFF, 0x8F, 0x21})) {
                addValue(field, 1, "Run hour meter (TOT)", "min");
            } else if (fieldMatch(field, {0x84, 0x10, 0xFF, 0x8F, 0x21})) {
                addValue(field, 1, "Run hour meter (Tariff 1)", "min");
            } else if (fieldMatch(field, {0x84, 0x20, 0xFF, 0x8F, 0x21})) {
                addValue(field, 1, "Run hour meter (Tariff 2)", "min");
            }
        }
    }

private:
    bool fieldMatch(const std::vector<uint8_t>& field, const std::initializer_list<uint8_t>& pattern) {
        return std::equal(pattern.begin(), pattern.end(), field.begin());
    }

    void addValue(const std::vector<uint8_t>& field, double scale, const std::string& description, const std::string& unit) {
        int value = (field[7] << 0) | (field[8] << 8) | (field[9] << 16) | (field[10] << 24);
        double scaled_value = value * scale;
        std::cout << description << ": " << scaled_value << " " << unit << std::endl;
    }

    std::string decodeSecondaryAddress(const std::vector<uint8_t>& data) {
        std::ostringstream secondaryAddress;
        secondaryAddress << std::hex << std::uppercase;
        for (int i = 3; i >= 0; --i) {
            secondaryAddress << ((data[i] & 0xF0) >> 4);
            secondaryAddress << (data[i] & 0x0F);
        }
        return secondaryAddress.str();
    }
};

METER_REGISTER(MeterIME, "ime_driver", MANUFACTURER_IME, 0x08, 0x55);
