import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]

multical402_ns = cg.esphome_ns.namespace("multical402")
Multical402 = multical402_ns.class_(
    "Multical402", cg.PollingComponent, uart.UARTDevice
)

CONF_ENERGY = "energy"
CONF_POWER = "power"
CONF_TEMP_IN = "temp_in"
CONF_TEMP_OUT = "temp_out"
CONF_TEMP_DIFF = "temp_diff"
CONF_FLOW = "flow"
CONF_VOLUME = "volume"
CONF_ENERGY_HIRES_403 = "energy_hires_403"
CONF_ENERGY_HIGH = "energy_high"

SENSOR_KEYS = [
    (CONF_ENERGY, "set_sensor_energy"),
    (CONF_POWER, "set_sensor_power"),
    (CONF_TEMP_IN, "set_sensor_tempin"),
    (CONF_TEMP_OUT, "set_sensor_tempout"),
    (CONF_TEMP_DIFF, "set_sensor_tempdiff"),
    (CONF_FLOW, "set_sensor_flow"),
    (CONF_VOLUME, "set_sensor_volume"),
    (CONF_ENERGY_HIRES_403, "set_sensor_energy_hires_403"),
    (CONF_ENERGY_HIGH, "set_sensor_energy_high"),
]

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Multical402),
            **{cv.Required(k): cv.use_id(sensor.Sensor) for k, _ in SENSOR_KEYS},
        }
    )
    .extend(cv.polling_component_schema("5min"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    for key, setter in SENSOR_KEYS:
        sens = await cg.get_variable(config[key])
        cg.add(getattr(var, setter)(sens))
