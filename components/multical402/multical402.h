#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "kmp.h"

namespace esphome {
namespace multical402 {

class Multical402 : public PollingComponent, public uart::UARTDevice {
 public:
  void set_sensor_energy(sensor::Sensor *s) { sensor_energy_ = s; }
  void set_sensor_power(sensor::Sensor *s) { sensor_power_ = s; }
  void set_sensor_tempin(sensor::Sensor *s) { sensor_tempin_ = s; }
  void set_sensor_tempout(sensor::Sensor *s) { sensor_tempout_ = s; }
  void set_sensor_tempdiff(sensor::Sensor *s) { sensor_tempdiff_ = s; }
  void set_sensor_flow(sensor::Sensor *s) { sensor_flow_ = s; }
  void set_sensor_volume(sensor::Sensor *s) { sensor_volume_ = s; }
  void set_sensor_energy_hires_403(sensor::Sensor *s) { sensor_energy_hires_403_ = s; }
  void set_sensor_energy_high(sensor::Sensor *s) { sensor_energy_high_ = s; }

  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

  void setup() override {}

  void update() override {
    ESP_LOGD(KMP_TAG, "Start update");

    float energy = kmp_.HeatEnergy();
    if (energy != -1)
      sensor_energy_->publish_state(energy);

    float volume = kmp_.Volume();
    if (volume != -1)
      sensor_volume_->publish_state(volume);

    float tempin = kmp_.CurrentForwardTemperature();
    if (tempin != -1)
      sensor_tempin_->publish_state(tempin);

    float tempout = kmp_.CurrentReturnTemperature();
    if (tempout != -1)
      sensor_tempout_->publish_state(tempout);

    float tempdiff = kmp_.CurrentDifferentialTemperature();
    if (tempdiff != -1)
      sensor_tempdiff_->publish_state(tempdiff);

    float power = kmp_.CurrentPower();
    if (power != -1)
      sensor_power_->publish_state(power);

    float flow = kmp_.CurrentWaterFlow();
    if (flow != -1)
      sensor_flow_->publish_state(flow);

    float energy_hires_403 = kmp_.HeatEnergy_hires_403();
    if (energy_hires_403 != -1)
      sensor_energy_hires_403_->publish_state(energy_hires_403);

    float energy_high = kmp_.HeatEnergy_high();
    if (energy_high != -1)
      sensor_energy_high_->publish_state(energy_high);

    ESP_LOGD(KMP_TAG, "End update");
  }

 private:
  KMP kmp_{this};
  sensor::Sensor *sensor_energy_{nullptr};
  sensor::Sensor *sensor_power_{nullptr};
  sensor::Sensor *sensor_tempin_{nullptr};
  sensor::Sensor *sensor_tempout_{nullptr};
  sensor::Sensor *sensor_tempdiff_{nullptr};
  sensor::Sensor *sensor_flow_{nullptr};
  sensor::Sensor *sensor_volume_{nullptr};
  sensor::Sensor *sensor_energy_hires_403_{nullptr};
  sensor::Sensor *sensor_energy_high_{nullptr};
};

}  // namespace multical402
}  // namespace esphome
