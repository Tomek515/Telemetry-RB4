#include "mqtt_communication.hpp"
#include <iostream>
#include <thread>
#include <chrono>

const std::string CLIENT_ID = "RaspberryPiClient";

ServerCommunication server_communication_instance;

ServerCommunication::ServerCommunication() {
    mosquitto_lib_init();
}

ServerCommunication::~ServerCommunication() {
    if (mqtt_client) {
        mosquitto_disconnect(mqtt_client);
        mosquitto_destroy(mqtt_client);
    }
    mosquitto_lib_cleanup();
}

void ServerCommunication::begin() {
    mqtt_client = mosquitto_new(CLIENT_ID.c_str(), true, nullptr);
    if (!mqtt_client) {
        std::cerr << "Failed to create MQTT client instance." << std::endl;
        return;
    }

    mosquitto_username_pw_set(mqtt_client, mqtt_username.c_str(), mqtt_password.c_str());

    int ret = mosquitto_tls_set(mqtt_client, ca_cert_path.c_str(), nullptr, nullptr, nullptr, nullptr);
    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to set TLS: " << mosquitto_strerror(ret) << std::endl;
        return;
    }

    ret = mosquitto_connect(mqtt_client, mqtt_server.c_str(), mqtt_port, 60);
    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect to MQTT broker: " << mosquitto_strerror(ret) << std::endl;
        return;
    }

    std::cout << "MQTT connected" << std::endl;

    mosquitto_loop_start(mqtt_client);
}

void ServerCommunication::mqttReconnect() {
    if (!mqtt_client) return;

    int ret = mosquitto_reconnect(mqtt_client);
    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "Reconnect failed: " << mosquitto_strerror(ret) << std::endl;
    }
}



void ServerCommunication::publishMqttMessage(const std::string& payload) {
    if (!mqtt_client) return;

    int ret = mosquitto_publish(mqtt_client, nullptr,
                                mqtt_publish_topic.c_str(),
                                payload.size(),
                                payload.c_str(),
                                1, false);

    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to publish message: " << mosquitto_strerror(ret) << std::endl;
    }
}
