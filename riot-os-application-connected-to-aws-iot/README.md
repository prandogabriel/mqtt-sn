# Configuração

- 1º entrar no broker e iniciá-lo 
  - ``cd broker-mqtt/``
  - ``mosquitto -p 1884 -v -c ./mosquitto_bridge.conf``
- 2º entrar no MQTT_SN gateway e iniciar
  - ``cd paho.mqtt-sn.embedded-c/MQTTSNGateway/bin/``
  - ``./MQTT-SNGateway``
- 3º RIOT
  - ``cd RIOT``
  - configurar as interfaces de rede
    - ``sudo ./dist/tools/tapsetup/tapsetup``
    - ``sudo ip a a fec0:affe::1/64 dev tapbr0``
  - ``cd examples/custom_mqttsn/``
  - ``make``
  - ``make term``