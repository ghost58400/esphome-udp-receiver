import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor

AUTO_LOAD = ["socket"]

udp_receiver_ns = cg.esphome_ns.namespace('udp_receiver')
UdpReceiver = udp_receiver_ns.class_('UdpReceiver', text_sensor.TextSensor, cg.Component)

CONFIG_SCHEMA = text_sensor.text_sensor_schema().extend({
    cv.GenerateID(): cv.declare_id(UdpReceiver)
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = await text_sensor.new_text_sensor(config)
    await cg.register_component(var, config)