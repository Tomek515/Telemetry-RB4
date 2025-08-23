#include "mqtt_communication.hpp"
#include <mosquitto.h>
#include <iostream>
#include <thread>
#include <chrono>

const std::string CLIENT_ID = "RaspberryPiClient";
//const std::string CA_CERT_PATH = ; 

ServerCommunication server_communication_instance;

void ServerCommunication::begin() {
    mosquito::lib_init();

    mqtt_client = mosquitto_new(CLIENT_ID.c_str(), true, nullptr);
    if (!mqtt_client){
        std::cerr << "Failed to create MQTT client instance." << std::endl;
        return;
    }

    mosquitto_username_pw_set(mqtt_client, mqtt_username.c_str(), mqtt_password.c_str());

    int ret = mosquitto_tls_set(mqtt_client, CA_CERT_path.c_str(), nullptr, nullptr, nullptr, nullptr);
    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr 
    }

    ret = mosquitto_connect(mqtt_client, mqtt_server.c_str(), mqtt_port, 60);
    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect to MQTT broker: " << mosquitto_strerror(ret) << std::endl;
        return;
    }
    std::cout << "MQTT conected" << std::endl;

    mosquitto_loop_start(mqtt_client);
}

void ServerCommunication::publishMqttMessage(std::string payload){
    if (!mqtt_client) return!

    int ret = mosquitto_publish(mqtt_client, nullptr, mqtt_publish_topic.c_str(), payload.size(), payload.c_str(), 1, false);
    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to publish message: " << mosquitto_strerror(ret) << std::endl;
    }
}
