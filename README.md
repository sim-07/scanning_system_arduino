## Hardware Connection Schema

### **1. Stepper Motor (Targeting System)**
*Controlled via ULN2003 Driver*

* **IN1** ➔ Arduino **Pin 4**
* **IN2** ➔ Arduino **Pin 5**
* **IN3** ➔ Arduino **Pin 6**
* **IN4** ➔ Arduino **Pin 7**
* **VCC/GND** ➔ **External Power Supply** (Breadboard Rails)

---

### **2. Servo Motor (Scanning System)**
*Controlled via 16-bit Hardware Timer*

* **Signal (Orange)** ➔ Arduino **Pin 9**
* **VCC (Red)** ➔ **Breadboard (+)**
* **GND (Brown)** ➔ **Breadboard (-)**

---

### **3. Ultrasonic Sensor (HC-SR04)**

* **Trig** ➔ Arduino **Pin 2**
* **Echo** ➔ Arduino **Pin 3**
* **VCC** ➔ **Breadboard (+)**
* **GND** ➔ **Breadboard (-)**

---

### **4. Power Management**

* **Common Ground:** Arduino **GND** ➔ Bridged to **Breadboard GND (-)**
* **Power Source:** External Breadboard Power Supply Module
* **Arduino Board:** Powered via **USB** (Logic) and shared **GND** with Power Supply Module
