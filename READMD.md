## ESP32 Firebase Example

This project uses [FirebaseClient](https://github.com/mobizt/FirebaseClient) library for Arduino. The example has two modes:

- `Firebase Firestore`
- `Firebase Realtime Database`

Change the `DB_TYPE` macro in main to use Realtime Datebase or Firestore.

### Configure Credentials

To config credentials using the files in `data` folder. These will be read at setup.

1. Rename `firebase_config.json.example` to `firebase_config.json`
2. Rename `wifi_config.json.example` to `wifi_config.json` 
3. Add your WiFi ssid and password to `wifi_config.json`
4. Configure a [Firebase](https://firebase.google.com/) project with Anonymous Authentication or Email Authentication, Firestore and Realtime Database.
5. Add the projects credentials into `firebase_config.json`