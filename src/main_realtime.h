/**
 * ABOUT:
 *
 * The non-blocking (async) example to push the data to the database.
 *
 * This example also shows how to use the query to filter your data.
 *
 * This example uses the UserAuth class for authentication, and the DefaultNetwork class for network interface configuration.
 * See examples/App/AppInitialization and examples/App/NetworkInterfaces for more authentication and network examples.
 *
 * The complete usage guidelines, please read README.md or visit https://github.com/mobizt/FirebaseClient
 *
 * SYNTAX:
 *
 * 1.------------------------
 *
 * RealtimeDatabase::push<T>(<AsyncClient>, <path>, <value>, <AsyncResultCallback>, <uid>);
 *
 * T - The type of value to push.
 * <AsyncClient> - The async client.
 * <path> - The node path to push the value.
 * <value> - The value to push.
 * <AsyncResultCallback> - The async result callback (AsyncResultCallback).
 * <uid> - The user specified UID of async result (optional).
 */

#include <Arduino.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>

#include <Benchmark.h>
#include <SimpleTimer.h>

#include <CredentialsManager/CredentialsManager.h>

static const char *WIFI_SSID;
static const char *WIFI_PASSWORD;

// static const char *API_KEY;
// static const char *USER_EMAIL;
// static const char *USER_PASSWORD;
static const char *DATABASE_URL;

DefaultNetwork network; // initialize with boolean parameter to enable/disable network reconnection
FirebaseApp app;
WiFiClientSecure sslClient;
using AsyncClient = AsyncClientClass;

AsyncClient aClient(sslClient, getNetwork(network));
RealtimeDatabase Database;

FirebaseCredential firebaseCredential;
WifiCredential wifiCredential;

bool taskCompleted = false;

void asyncCB(AsyncResult &aResult);
void printResult(AsyncResult &aResult);
void printError(int code, const String &msg);
void timeStatusCB(uint32_t &ts);

void setup()
{
    Serial.begin(115200);
    delay(3000); // wait for the serial monitor to connect

    // Read configuration files
    if (!LittleFS.begin())
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

    CredentialsManager credentialsManager(LittleFS);
    firebaseCredential = credentialsManager.getFirebaseCredential();
    wifiCredential = credentialsManager.getWifiCredential();

    if (wifiCredential.isEmpty())
    {
        Serial.println("Failed to read configuration file");
        return;
    }
    WIFI_SSID = wifiCredential.ssid.c_str();
    WIFI_PASSWORD = wifiCredential.password.c_str();

    if (firebaseCredential.isEmpty())
    {
        Serial.println("Firebase configuration is empty");
        return;
    }
    // API_KEY = firebaseCredential.apiKey.c_str();
    // USER_EMAIL = firebaseCredential.userEmail.c_str();
    // USER_PASSWORD = firebaseCredential.userPassword.c_str();
    DATABASE_URL = firebaseCredential.realtimeDbUrl.c_str();

    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());

    // Setup Firebase
    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
    sslClient.setInsecure();

    Serial.println("Initializing the app...");
    // UserAuth userAuth(API_KEY, USER_EMAIL, USER_PASSWORD);
    NoAuth userAuth;
    initializeApp(aClient, app, getAuth(userAuth), asyncCB, "authTask");
    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);
    Serial.println("Initialized the app");

    // Set time using NTP server
    tm timeinfo;
    configTzTime("UTC0", "0.pool.ntp.org", "1.pool.ntp.org", "2.pool.ntp.org");
    getLocalTime(&timeinfo);
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void loop()
{
    app.loop();
    Database.loop();

    if (app.ready() && !taskCompleted)
    {
        taskCompleted = true;

        Serial.print("Pushing the JSON object... ");
        uint32_t ms = millis();
        float temperature = random(0, 1000) / 11.0;
        float humidity = random(0, 1000) / 11.0;
        String json = "{\"timestamp\": " + String(ms) + ", \"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + "}";
        object_t obj = object_t(json);
        BENCHMARK_MICROS_BEGIN(PUSH)
        Database.push<object_t>(aClient, "/test/json", object_t("{\"data\":123}"), asyncCB, "pushJsonTask1");
        BENCHMARK_MICROS_END(PUSH)

        // object_t json, obj1, obj2, obj3, obj4;
        // JsonWriter writer;
        // writer.create(obj1, "int/value", 9999);
        // writer.create(obj2, "string/value", string_t("hello"));
        // writer.create(obj3, "float/value", number_t(123.456, 2));
        // writer.join(obj4, 3 /* no. of object_t (s) to join */, obj1, obj2, obj3);
        // writer.create(json, "node/list", obj4);
        // Database.push<object_t>(aClient, "/test/json", json, asyncCB, "pushJsonTask2");
    }

    EVERY_N_MILLIS(10000)
    {
        taskCompleted = false;
    }
}

void asyncCB(AsyncResult &aResult) { printResult(aResult); }

void printResult(AsyncResult &aResult)
{
    if (aResult.isEvent())
    {
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
    }

    if (aResult.isDebug())
    {
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
    }

    if (aResult.isError())
    {
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
    }

    if (aResult.available())
    {
        if (aResult.to<RealtimeDatabaseResult>().name().length())
            Firebase.printf("task: %s, name: %s\n", aResult.uid().c_str(), aResult.to<RealtimeDatabaseResult>().name().c_str());
        Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
    }
}
void printError(int code, const String &msg)
{
    Firebase.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}

void timeStatusCB(uint32_t &ts)
{
    if (time(nullptr) < FIREBASE_DEFAULT_TS)
    {

        configTime(3 * 3600, 0, "pool.ntp.org");
        while (time(nullptr) < FIREBASE_DEFAULT_TS)
        {
            delay(100);
        }
    }
    ts = time(nullptr);
}