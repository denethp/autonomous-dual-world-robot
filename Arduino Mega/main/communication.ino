void sendCommand(char cmd) {
  Serial2.write(cmd);

  Serial.print("Sent: ");
  Serial.println(cmd);
}

void waitForDone() {
  String msg = "";

  while (true) {
    while (Serial2.available()) {
      char c = Serial2.read();

      if (c == '\n') {
        msg.trim();

        Serial.print("Received: ");
        Serial.println(msg);

        if (msg == "DONE") {
          Serial.println("Step Completed\n");
          return;
        }

        msg = "";
      } else {
        msg += c;
      }
    }
  }
}