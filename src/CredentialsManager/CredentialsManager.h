#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>

#include "Models.h"

#define FIREBASE_CONFIG_FILE "/firebase_config.json"
#define WIFI_CONFIG_FILE "/wifi_config.json"

class CredentialsManager
{
private:
    fs::FS &fileSystem;

public:
    CredentialsManager(fs::FS &fileSystem) : fileSystem(fileSystem) {};

    WifiCredential getWifiCredential()
    {
        Serial.printf("Reading file: %s\r\n", WIFI_CONFIG_FILE);
        File wifiConfigFile = fileSystem.open(WIFI_CONFIG_FILE, "r");
        if (!wifiConfigFile)
        {
            Serial.println("Failed to open wifi_config.json file");
            return WifiCredential();
        }
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, wifiConfigFile);
        if (error)
        {
            Serial.println("Failed to read file, using default configuration");
            wifiConfigFile.close();
            return WifiCredential();
        }
        String ssid = doc["ssid"];
        String password = doc["password"];
        wifiConfigFile.close();

        return WifiCredential{ssid, password};
    }

    FirebaseCredential getFirebaseCredential()
    {
        Serial.printf("Reading file: %s\r\n", FIREBASE_CONFIG_FILE);
        File firebaseConfigFile = fileSystem.open(FIREBASE_CONFIG_FILE, "r");
        if (!firebaseConfigFile)
        {
            Serial.println("Failed to open firebase_config.json file");
            return FirebaseCredential();
        }
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, firebaseConfigFile);
        if (error)
        {
            Serial.println("Failed to read file, using default configuration");
            firebaseConfigFile.close();
            return FirebaseCredential();
        }
        String apiKey = doc["apiKey"];
        String projectId = doc["projectId"];
        String realtimeDbUrl = doc["realtimeDbUrl"];
        String userEmail = doc["userEmail"];
        String userPassword = doc["userPassword"];
        firebaseConfigFile.close();

        return FirebaseCredential{apiKey, projectId, realtimeDbUrl, userEmail, userPassword};
    }
};