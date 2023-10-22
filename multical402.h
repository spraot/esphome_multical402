// 151023 htvekov
// Added sensor_energy_high - HiRes Energy register

#ifndef _M402_
#define _M402_

#include "esphome.h"
#include "esphome/core/component.h"

#include "kmp.h"

class Multical402 : public PollingComponent
{
public:
  Sensor *sensor_energy{nullptr};
  Sensor *sensor_power{nullptr};
  Sensor *sensor_tempin{nullptr};
  Sensor *sensor_tempout{nullptr};
  Sensor *sensor_tempdiff{nullptr};
  Sensor *sensor_flow{nullptr};
  Sensor *sensor_volume{nullptr};
  Sensor *sensor_energy_hires_403{nullptr};
  Sensor *sensor_energy_high{nullptr};

  // constructor
  Multical402(
      uint32_t update_interval,
      UARTComponent *uart_bus,
      Sensor *m__energy,
      Sensor *m__power,
      Sensor *m__tin,
      Sensor *m__tout,
      Sensor *m__tdiff,
      Sensor *m__flow,
      Sensor *m__volume,
      Sensor *m__energy_hires_403,
      Sensor *m__energy_high) : PollingComponent(update_interval),
                                sensor_energy(m__energy),
                                sensor_power(m__power),
                                sensor_tempin(m__tin),
                                sensor_tempout(m__tout),
                                sensor_tempdiff(m__tdiff),
                                sensor_flow(m__flow),
                                sensor_volume(m__volume),
                                sensor_energy_hires_403(m__energy_hires_403),
                                sensor_energy_high(m__energy_high)
  {
    _kmp = new KMP(uart_bus);
  }

  float get_setup_priority() const override { return esphome::setup_priority::AFTER_WIFI; }

  void setup() override {}

  void update() override
  {
    ESP_LOGD(TAG, "Start update");

    float energy = _kmp->HeatEnergy();
    if (energy != -1)
      sensor_energy->publish_state(energy);
    
    float volume = _kmp->Volume();
    if (volume != -1)
      sensor_volume->publish_state(volume);

    float tempin = _kmp->CurrentForwardTemperature();
    if (tempin != -1)
      sensor_tempin->publish_state(tempin);

    float tempout = _kmp->CurrentReturnTemperature();
    if (tempout != -1)
      sensor_tempout->publish_state(tempout);

    float tempdiff = _kmp->CurrentDifferentialTemperature();
    if (tempdiff != -1)
      sensor_tempdiff->publish_state(tempdiff);

    float power = _kmp->CurrentPower();
    if (power != -1)
      sensor_power->publish_state(power);

    float flow = _kmp->CurrentWaterFlow();
    if (flow != -1)
      sensor_flow->publish_state(flow);

    float energy_hires_403 = _kmp->HeatEnergy_hires_403();
    if (energy_hires_403 != -1)
      sensor_energy_hires_403->publish_state(energy_hires_403);

    float energy_high = _kmp->HeatEnergy_high();
    if (energy_high != -1)
      sensor_energy_high->publish_state(energy_high);

    ESP_LOGD(TAG, "End update");
  }

private:
  KMP *_kmp;
};

#endif