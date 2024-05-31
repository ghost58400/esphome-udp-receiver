```yaml
# example configuration:

external_components:
  - source: github://ghost58400/esphome-udp-receiver
    components: [ udp_receiver ]

text_sensor:
  - platform: udp_receiver
    port: 1337
    on_value:
      then:
        - lambda: ....
```
