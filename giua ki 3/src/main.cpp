#include <WiFi.h>
#include <PubSubClient.h>

// Thông tin Wi-Fi
const char* ssid = "Phong 302";          // Điền tên Wi-Fi của bạn
const char* password = "33332222";  // Điền mật khẩu Wi-Fi của bạn

// Thông tin MQTT
const char* mqtt_server = "69f695fc1c884f55872b72f864e38612.s1.eu.hivemq.cloud";   // Địa chỉ broker MQTT 
const int mqtt_port = 8883;                   // Cổng MQTT 
const char* mqtt_user = "PhamLong";      // Tên người dùng MQTT 
const char* mqtt_pass = "LongPham2003";  // Mật khẩu MQTT 
const char* mqtt_topic = "soil_moisture";     // Chủ đề MQTT để gửi giá trị độ ẩm đất

WiFiClient espClient;
PubSubClient client(espClient);

// Chân Relay
const int relayPin = 17; // GPIO17

// Chân cảm biến độ ẩm đất
const int soilMoisturePin = A0;  // A0 (chân analog)

void setup() {
  // Khởi tạo chân relay là OUTPUT
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // Bật relay (tùy thuộc vào module relay)

  // Khởi tạo serial monitor
  Serial.begin(115200);

  // Kết nối MQTT
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // Đọc giá trị từ cảm biến độ ẩm đất
  int sensor_value = analogRead(soilMoisturePin);

  // Áp dụng công thức tính độ ẩm đất
  float do_am_dat = (100 - ((sensor_value / 4095.0) * 100));

  // In ra giá trị độ ẩm đất qua serial monitor
  Serial.print("Sensor value: ");
  Serial.print(sensor_value);
  Serial.print(" -> Soil Moisture: ");
  Serial.print(do_am_dat);
  Serial.println("%");

  // Gửi giá trị độ ẩm đất qua MQTT
  char msg[50];
  snprintf(msg, 50, "Soil Moisture: %.2f%%", do_am_dat);
  client.publish(mqtt_topic, msg);

  // Điều khiển relay dựa trên độ ẩm đất
  if (do_am_dat < 50) {
    // Nếu độ ẩm đất < 50%, bật relay
    digitalWrite(relayPin, HIGH);  // Tùy thuộc vào loại relay, HIGH có thể là bật
    Serial.println("Relay ON");
  } else {
    // Nếu độ ẩm đất >= 50%, tắt relay
    digitalWrite(relayPin, LOW);   // Tùy thuộc vào loại relay, LOW có thể là tắt
    Serial.println("Relay OFF");
  }

  // Đợi một chút trước khi đo lại
  delay(5000);  // 5 giây
}

void connectWiFi() {
  // Kết nối Wi-Fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  // Kết nối lại với MQTT broker
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}