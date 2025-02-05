#pragma once

#include <Arduino.h>

struct FirebaseCredential
{
    // The API key can be obtained from Firebase console > Project Overview > Project settings.
    String apiKey = "";
    String projectId = "";
    String realtimeDbUrl = "";

    // User Email and password that already registered or added in your project. (Anonymous enabled)
    String userEmail = "";
    String userPassword = "";

    bool isEmpty()
    {
        return apiKey.isEmpty() || projectId.isEmpty() || realtimeDbUrl.isEmpty() || userEmail.isEmpty() || userPassword.isEmpty();
    }
};

struct WifiCredential
{
    String ssid = "";
    String password = "";

    bool isEmpty()
    {
        return ssid.isEmpty() || password.isEmpty();
    }
};