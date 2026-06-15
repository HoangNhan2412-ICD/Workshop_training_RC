# Wokwi ESP32 RC Car Lab

Project mô phỏng Lab 1: ESP32 điều khiển L298N + DC motor + servo.

Trong Wokwi, L298N/DC motor được mô phỏng bằng:

- LED IN1 trên GPIO26.
- LED IN2 trên GPIO27.
- LED ENA_PWM và Logic Analyzer trên GPIO25.
- Servo `wokwi-servo` trên GPIO18.

## Cách chạy trên Wokwi

1. Tạo project Arduino ESP32 trên Wokwi.
2. Copy 4 file vào project:
   - `sketch.ino`
   - `diagram.json`
   - `libraries.txt`
   - `README.md`
3. Bấm Start Simulation.
4. Mặc định `AUTO_TEST = 1`, chương trình tự chạy bài lab.
5. Nếu muốn test tay, đổi `#define AUTO_TEST 0`, sau đó dùng Serial Monitor:
   - `F`: tiến.
   - `B`: lùi.
   - `L`: rẽ trái.
   - `R`: rẽ phải.
   - `S`: dừng.
   - `180 90`: chạy speed 180, servo 90.

## Expected behavior

- Tiến: IN1 sáng, IN2 tắt, ENA có PWM.
- Lùi: IN1 tắt, IN2 sáng, ENA có PWM.
- Dừng: IN1 tắt, IN2 tắt, ENA = 0.
- Servo đổi góc 90, 135, 90 theo auto test.
- Logic Analyzer:
  - D0 = ENA_PWM GPIO25.
  - D1 = IN1 GPIO26.
  - D2 = IN2 GPIO27.
  - D3 = SERVO GPIO18.

## CHECK SAI / CẦN SỬA CHI TIẾT

### 1. Sai pin IN1/IN2/ENA/SERVO

1. Sai cái gì: cần kiểm tra cả `sketch.ino` và `diagram.json`. Pin bắt buộc là `IN1 = GPIO26`, `IN2 = GPIO27`, `ENA_PWM = GPIO25`, `SERVO_PIN = GPIO18`.
2. Ảnh hưởng gì: nếu sai pin, LED IN1/IN2 không phản ánh đúng chiều motor, PWM ENA không hiện trên LED/Logic Analyzer, hoặc servo không quay đúng tín hiệu.
3. Đã sửa gì: trong `sketch.ino` đã đặt `IN1 = 26`, `IN2 = 27`, `ENA_PWM = 25`, `SERVO_PIN = 18`. Trong `diagram.json` đã nối `esp:26` tới LED IN1 và Logic Analyzer D1, `esp:27` tới LED IN2 và D2, `esp:25` tới LED ENA_PWM và D0, `esp:18` tới servo PWM và D3.
4. Trạng thái sau sửa: pin code và pin sơ đồ đã khớp với yêu cầu lab.

### 2. Servo thiếu V+ hoặc GND

1. Sai cái gì: servo `wokwi-servo` cần đủ 3 dây `PWM`, `V+`, `GND`. Lỗi thường nằm trong `diagram.json` nếu chỉ nối PWM mà quên nguồn hoặc mass.
2. Ảnh hưởng gì: servo có thể không hoạt động hoặc không đổi góc trong mô phỏng, dù code đã gọi `steeringServo.write(angle)`.
3. Đã sửa gì: trong `diagram.json`, `servo1:PWM` đã nối tới `esp:18`, `servo1:V+` đã nối tới `esp:5V`, và `servo1:GND` đã nối tới `esp:GND.2`.
4. Trạng thái sau sửa: servo đã có đủ tín hiệu điều khiển, nguồn và GND.

### 3. PWM không thấy vì chưa attach đúng GPIO25/channel

1. Sai cái gì: trong `sketch.ino`, nếu PWM attach sai pin hoặc trùng channel với servo, tín hiệu ENA_PWM trên GPIO25 có thể không xuất hiện đúng.
2. Ảnh hưởng gì: LED ENA_PWM không sáng theo độ rộng xung, Logic Analyzer D0 không thấy PWM, nên không quan sát được mức speed 30%, 60%, 100%.
3. Đã sửa gì: trong `sketch.ino`, PWM đã attach đúng GPIO25. Với Arduino-ESP32 core mới dùng `ledcAttach(ENA_PWM, PWM_FREQ, PWM_RESOLUTION)`. Với core cũ dùng `ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION)` và `ledcAttachPin(ENA_PWM, PWM_CHANNEL)`. Channel đã đặt là `PWM_CHANNEL = 8` để giảm khả năng đụng với servo.
4. Trạng thái sau sửa: ENA_PWM đã xuất ra GPIO25 và được nối tới LED ENA_PWM cùng Logic Analyzer D0.

### 4. Watchdog làm xe dừng quá sớm trong AUTO_TEST

1. Sai cái gì: watchdog yêu cầu nếu quá 500ms không có lệnh mới thì `motorStop()`. Trong `AUTO_TEST`, nếu không cập nhật `lastCmdTime` trong lúc delay 1-2 giây, motor sẽ bị dừng giữa bước test.
2. Ảnh hưởng gì: auto test không giữ được tiến/lùi đủ thời gian, LED IN1/IN2 và PWM ENA bị tắt sớm hơn mô tả bài lab.
3. Đã sửa gì: trong `sketch.ino`, hàm `autoStep()` gọi `markCommandTime()` lặp lại mỗi 20ms trong thời gian chờ, đồng thời vẫn gọi `checkWatchdog()`.
4. Trạng thái sau sửa: `AUTO_TEST` chạy đủ từng bước 2 giây hoặc 1 giây mà không bị watchdog dừng sớm.

### 5. README nói có lệnh Serial `180 90` nhưng code chưa parse lệnh này

1. Sai cái gì: nếu README hướng dẫn nhập `180 90` nhưng `sketch.ino` chỉ xử lý ký tự `F/B/L/R/S`, người dùng không test được speed và angle tùy ý.
2. Ảnh hưởng gì: Serial Monitor báo không hiểu lệnh hoặc xe không chạy theo giá trị nhập tay.
3. Đã sửa gì: trong `sketch.ino`, hàm `handleSerialCommand()` đã parse chuỗi dạng `speed angle`, ví dụ `180 90`, bằng cách tách theo dấu cách và gọi `rcDrive(speed, angle)`. Code cũng thay dấu phẩy bằng dấu cách, nên `180,90` vẫn có thể dùng.
4. Trạng thái sau sửa: README và code đã khớp nhau; lệnh `180 90` chạy được khi `AUTO_TEST = 0`.

### 6. WebSocket không nên đưa vào project Wokwi Lab 1

1. Sai cái gì: WebSocket không thuộc mục tiêu Lab 1 mô phỏng L298N + DC motor + servo. Nếu đưa WebSocket vào project Wokwi này, cấu hình sẽ phức tạp hơn vì cần gateway hoặc port forwarding.
2. Ảnh hưởng gì: người học dễ nhầm trọng tâm, mô phỏng có thể không chạy như lab cơ bản, và lỗi mạng sẽ che mất lỗi điều khiển motor/servo.
3. Đã sửa gì: project này không thêm WebSocket. Điều khiển tự động dùng `AUTO_TEST`, điều khiển tay dùng Serial Monitor.
4. Trạng thái sau sửa: project giữ đúng phạm vi Lab 1; WebSocket nên tách thành project hoặc mode riêng cho phần hardware thật.


# Wokwi ESP32 RC Car Lab 2

## 1. Mục tiêu

Mô phỏng LAB 2: ESP32 tạo WiFi AP, chạy web server và WebSocket server để nhận lệnh điều khiển xe RC.
Lệnh gồm `speed` và `angle`, sau đó ESP32 điều khiển L298N logic, ENA PWM và servo.

## 2. Đối chiếu với workshop

| Hạng mục trong workshop | Trên Wokwi | Trạng thái | Ghi chú ngắn |
|---|---|---|---|
| ESP32 DevKit V1 | Dùng `wokwi-esp32-devkit-v1` | Đúng | Bám theo board workshop. |
| WiFi AP `RC_CAR_ESP32` | `WiFi.softAP("RC_CAR_ESP32", "12345678")` | Đúng | AP chạy trong mô phỏng ESP32. |
| Password `12345678` | Đặt đúng trong code | Đúng | Dùng cho phần cứng thật. |
| WebSocket server | `WebSocketsServer` port 81 | Đúng | Dễ chạy trên Wokwi hơn `/ws` cùng port 80. |
| Web UI điều khiển | Trang web có nút F/B/L/R/S và slider `maxSpeed` | Đúng | Gửi JSON qua WebSocket. |
| L298N | Thay bằng LED/probe/Logic Analyzer | Thay thế | Quan sát IN1, IN2, ENA. |
| DC motor | Thay bằng tín hiệu IN1/IN2/ENA | Thay thế | Không mô phỏng tải motor thật. |
| Servo | Dùng `wokwi-servo` | Đúng | PWM GPIO18. |
| IN1 GPIO26 | LED IN1 + Logic Analyzer D1 | Đúng | Code và sơ đồ cùng GPIO26. |
| IN2 GPIO27 | LED IN2 + Logic Analyzer D2 | Đúng | Code và sơ đồ cùng GPIO27. |
| ENA GPIO25 | Logic Analyzer D0 + LED ENA | Đúng | Xem PWM tốc độ. |
| Servo GPIO18 | Servo + Logic Analyzer D3 | Đúng | Có VIN và GND. |
| Điều khiển bằng điện thoại thật | Cần test trên phần cứng thật | Không làm được | Wokwi không cho điện thoại thật kết nối AP mô phỏng như ESP32 thật. |

## 3. Cách chạy trên Wokwi

1. Mở project.
2. Bấm Start Simulation.
3. Mở Serial Monitor.
4. Test bằng `F`, `B`, `L`, `R`, `S`, hoặc `180 90`.

## 4. Cách chạy trên phần cứng thật

1. Nạp code vào ESP32.
2. Điện thoại kết nối WiFi `RC_CAR_ESP32`, password `12345678`.
3. Mở browser `http://192.168.4.1/`.
4. Dùng nút web để điều khiển xe.

## 5. Expected behavior

- Tiến: IN1 = HIGH, IN2 = LOW, ENA có PWM.
- Lùi: IN1 = LOW, IN2 = HIGH, ENA có PWM.
- Dừng: IN1 = LOW, IN2 = LOW, ENA = 0.
- Rẽ trái: servo 45 độ.
- Đi thẳng: servo 90 độ.
- Rẽ phải: servo 135 độ.
- D0 = ENA_PWM GPIO25.
- D1 = IN1 GPIO26.
- D2 = IN2 GPIO27.
- D3 = SERVO GPIO18.

## 6. Sai / khác so với workshop và cách xử lý

### Vấn đề 1: Điện thoại thật không kết nối trực tiếp vào AP mô phỏng

- Khác/sai ở đâu: Workshop dùng điện thoại thật kết nối WiFi AP của ESP32.
- Vì sao: AP trong Wokwi là mô phỏng, không phát WiFi vật lý cho điện thoại thật kết nối như phần cứng.
- Đã sửa/thay bằng: Thêm Serial fallback để test `F/B/L/R/S`, `180 90`, `-180 90` trong Wokwi; web UI giữ lại cho phần cứng thật.
- Ảnh hưởng: Wokwi vẫn kiểm tra được logic điều khiển; trải nghiệm điện thoại thật cần test trên ESP32 thật.

### Vấn đề 2: L298N thật được thay bằng LED/probe/Logic Analyzer

- Khác/sai ở đâu: Workshop dùng module L298N thật để kéo motor.
- Vì sao: Wokwi không mô phỏng đầy đủ L298N, dòng tải và sụt áp như mạch thật.
- Đã sửa/thay bằng: IN1 GPIO26, IN2 GPIO27 và ENA GPIO25 được đưa ra LED/probe/Logic Analyzer.
- Ảnh hưởng: Không kiểm tra được công suất L298N; vẫn kiểm tra đúng chiều quay và PWM.

### Vấn đề 3: DC motor thật không được mô phỏng đầy đủ

- Khác/sai ở đâu: Motor JGA25 thật có quán tính, tải, dòng khởi động và tốc độ vật lý.
- Vì sao: Mô phỏng này chỉ tập trung vào tín hiệu điều khiển từ ESP32.
- Đã sửa/thay bằng: Dùng IN1/IN2 để xem chiều và ENA PWM để xem tốc độ mong muốn.
- Ảnh hưởng: Không thấy motor quay thật; vẫn đủ để kiểm tra code Lab 2 trước khi nối motor thật.

### Vấn đề 4: Nguồn motor 7-12V không kiểm tra được trên Wokwi

- Khác/sai ở đâu: Workshop cần nguồn 7-12V cấp cho L298N/motor.
- Vì sao: Wokwi không kiểm tra nguồn công suất, sụt áp, nhiễu motor hoặc common GND như mạch thật.
- Đã sửa/thay bằng: Servo dùng VIN/GND trong Wokwi, motor driver được thay bằng tín hiệu logic.
- Ảnh hưởng: Phần nguồn motor bắt buộc test trên phần cứng thật.

### Vấn đề 5: WebSocket dùng port 81 thay vì `/ws` chung port 80

- Khác/sai ở đâu: Workshop mô tả WebSocket endpoint `/ws` trên web server.
- Vì sao: `WebServer` + `WebSocketsServer` trên Arduino/Wokwi chạy ổn định hơn khi tách web port 80 và WebSocket port 81.
- Đã sửa/thay bằng: Web server dùng port 80, WebSocket dùng port 81, web page kết nối `ws://<host>:81/`.
- Ảnh hưởng: Chức năng gửi JSON `speed/angle` không đổi; chỉ khác endpoint.

### Vấn đề 6: Không dùng ESPAsyncWebServer

- Khác/sai ở đâu: Workshop có thể dùng hướng AsyncWebServer cho WebSocket.
- Vì sao: `ESPAsyncWebServer` dễ lỗi thư viện/phụ thuộc trên Wokwi hơn.
- Đã sửa/thay bằng: Dùng `WebServer.h` và `WebSocketsServer.h`.
- Ảnh hưởng: Đủ cho Lab 2; code dễ compile hơn trong Wokwi.

### Vấn đề 7: Thêm Serial fallback để test logic

- Khác/sai ở đâu: Workshop Lab 2 điều khiển chính bằng web app.
- Vì sao: Wokwi không test điện thoại thật kết nối AP mô phỏng đầy đủ.
- Đã sửa/thay bằng: Thêm Serial Monitor nhận `F`, `B`, `L`, `R`, `S`, `180 90`, `-180 90`.
- Ảnh hưởng: Không thay đổi logic chính; giúp kiểm tra motor, PWM và servo trong Wokwi.

## 7. Kết luận

Project chạy được trên Wokwi để kiểm tra code, pin map, PWM, servo và logic nhận lệnh.
Phần WebSocket qua điện thoại thật, L298N thật, motor thật và nguồn 7-12V cần test trên phần cứng thật.

# Wokwi STM32 RC Car Lab

## 1. Mục tiêu

Project này mô phỏng BUỔI 2 của workshop RC car: STM32 điều khiển chiều quay motor DC qua IN1/IN2, tốc độ qua ENA PWM, servo lái qua PWM và nhận lệnh RF dạng frame 5 byte. Trên Wokwi, phần công suất L298N/motor thật được thay bằng LED, probe và Logic Analyzer để kiểm tra logic điều khiển trước khi làm mạch thật.

## 2. Đối chiếu với workshop

| Hạng mục trong workshop | Đã làm trên Wokwi | Trạng thái | Ghi chú ngắn |
|---|---|---|---|
| STM32F4 Discovery hoặc Nucleo-F411RE | Dùng `board-stm32-bluepill` | Thay thế | Wokwi hỗ trợ Blue Pill ổn định hơn cho Arduino-style STM32 |
| CubeMX/HAL | Dùng Arduino-style STM32 | Thay thế | Dễ chạy trực tiếp trong Wokwi, vẫn giữ logic điều khiển |
| IN1 điều khiển chiều motor | PA0, LED IN1, Logic Analyzer D1 | Đúng | `IN1 = PA0` |
| IN2 điều khiển chiều motor | PA1, LED IN2, Logic Analyzer D2 | Đúng | `IN2 = PA1` |
| ENA PWM điều khiển tốc độ | PA6, LED PWM, Logic Analyzer D0 | Đúng | PA6 là chân PWM hợp lệ trên Blue Pill |
| Servo PWM | PB6, servo Wokwi, Logic Analyzer D3 | Thay thế | PB6 tương ứng hướng TIM4_CH1 trên STM32F103 |
| USART2 TX/RX cho HC-12 | PA2 TX, PA3 RX nối Serial Monitor | Thay thế | Không có module HC-12 thật trong mô phỏng |
| Frame RF 5 byte | Parser `0xAA speed angle flags checksum` | Đúng | Checksum XOR byte 0 đến 3 |
| Watchdog 500 ms | `checkWatchdog()` | Đúng | Mất lệnh thì motor dừng |
| PID preview | Chỉ giữ cấu trúc sẵn sàng mở rộng | Không làm được | Chưa có encoder/tải thật để đóng vòng PID |

## 3. Cách chạy trên Wokwi

1. Mở project Wokwi với các file này.
2. Bấm Start Simulation.
3. Mặc định `AUTO_TEST = 1`: quan sát Serial Monitor, servo, LED và Logic Analyzer.
4. Muốn test tay hoặc frame RF, đổi `AUTO_TEST` thành `0`, rồi nhập lệnh trong Serial Monitor.

## 4. Expected behavior

* Tiến: IN1 = HIGH, IN2 = LOW, ENA có PWM.
* Lùi: IN1 = LOW, IN2 = HIGH, ENA có PWM.
* Dừng: IN1 = LOW, IN2 = LOW, ENA = 0.
* Servo đổi góc 90, 135, 90 theo bài test.
* Tốc độ 30%, 60%, 100% thể hiện bằng duty PWM trên ENA.
* Frame RF hợp lệ sẽ cập nhật tốc độ, góc lái và reset watchdog.

## 5. Sai / khác so với workshop và cách xử lý

### Vấn đề 1: Board STM32 không đúng 100% như workshop

* Khác/sai ở đâu: Workshop dùng STM32F4 Discovery hoặc Nucleo-F411RE, file `diagram.json` dùng `board-stm32-bluepill`.
* Vì sao: Wokwi hỗ trợ mô phỏng Blue Pill thuận tiện hơn cho project Arduino-style STM32.
* Đã sửa/thay bằng: Dùng STM32 Blue Pill, giữ đúng logic chính và giữ các chân bắt buộc PA0, PA1, PA2, PA3, PA6.
* Ảnh hưởng: Không kiểm tra đúng tài nguyên TIM của F411RE, nhưng vẫn kiểm tra được logic motor, PWM, UART frame và servo.

### Vấn đề 2: Không dùng CubeMX/HAL như workshop

* Khác/sai ở đâu: Workshop hướng HAL/CubeMX, file `sketch.ino` dùng Arduino C++.
* Vì sao: Arduino-style chạy nhanh và trực tiếp hơn trong Wokwi.
* Đã sửa/thay bằng: Viết đủ hàm điều khiển `motorSetSpeed()`, `servoSetAngle()`, `rcDrive()`, `rfParseByte()`, `processRfFrame()`, `checkWatchdog()` và `runAutoTest()`.
* Ảnh hưởng: Cần port lại sang HAL nếu nạp vào project CubeMX thật, nhưng thuật toán và pin map vẫn rõ ràng.

### Vấn đề 3: TIM3/TIM4 không cấu hình trực tiếp như phần cứng thật

* Khác/sai ở đâu: Workshop dùng timer PWM, còn `sketch.ino` dùng `analogWrite()` cho PA6 và thư viện Servo cho PB6.
* Vì sao: Wokwi Arduino-style ẩn phần cấu hình timer.
* Đã sửa/thay bằng: ENA PWM đặt tại PA6, servo đặt tại PB6 và đều đưa vào Logic Analyzer.
* Ảnh hưởng: Kiểm tra được duty PWM và servo PWM, nhưng chưa xác nhận thanh ghi timer như HAL thật.

### Vấn đề 4: HC-12 không mô phỏng đúng module RF thật

* Khác/sai ở đâu: Workshop dùng HC-12 qua USART2 PA2/PA3, Wokwi không mô phỏng đầy đủ kênh RF HC-12.
* Vì sao: HC-12 là module RF vật lý, Wokwi chủ yếu mô phỏng UART/Serial logic.
* Đã sửa/thay bằng: PA2/PA3 được nối với Serial Monitor; code vẫn tạo luồng `HC12Serial` và parser frame RF 5 byte.
* Ảnh hưởng: Kiểm tra được định dạng frame, checksum và watchdog, nhưng không kiểm tra được tầm RF, nhiễu RF hoặc baud thực của HC-12.

### Vấn đề 5: L298N và DC motor thật không mô phỏng đầy đủ

* Khác/sai ở đâu: Workshop dùng L298N và motor JGA25 hoặc tương đương, `diagram.json` dùng LED/probe/Logic Analyzer.
* Vì sao: Wokwi không phản ánh đúng tải motor, quán tính, dòng khởi động và sụt áp công suất.
* Đã sửa/thay bằng: IN1, IN2 hiển thị bằng LED; ENA PWM hiển thị bằng LED và Logic Analyzer.
* Ảnh hưởng: Đủ để kiểm tra chiều quay và PWM, nhưng tốc độ/quán tính/tải motor phải test trên phần cứng thật.

### Vấn đề 6: Nguồn motor 7-12V không được kiểm chứng

* Khác/sai ở đâu: Workshop cần nguồn motor 7-12V cho L298N, Wokwi chỉ mô phỏng tín hiệu logic.
* Vì sao: Mô phỏng này không có mạch nguồn công suất thật.
* Đã sửa/thay bằng: Chỉ kiểm tra tín hiệu điều khiển IN1/IN2/ENA và servo.
* Ảnh hưởng: Không đánh giá được nóng driver, dòng tải, sụt áp hoặc nhiễu nguồn.

### Vấn đề 7: Serial command phải khớp README

* Khác/sai ở đâu: Nếu README nói nhập `200 90` hoặc frame `AA C8 5A 00 38` mà code không parse thì test tay sẽ sai.
* Vì sao: Lệnh tay và frame RF là hai dạng nhập khác nhau.
* Đã sửa/thay bằng: `sketch.ino` parse được `F`, `B`, `L`, `R`, `S`, dạng `speed angle`, frame 4 byte tự tính checksum và frame 5 byte có checksum.
* Ảnh hưởng: Người học có thể test AUTO_TEST, lệnh tay và frame RF trong cùng một project.

### Vấn đề 8: Watchdog có thể làm AUTO_TEST dừng sai thời điểm

* Khác/sai ở đâu: Watchdog 500 ms sẽ gọi `motorStop()` nếu `lastCmdTime` không được cập nhật trong AUTO_TEST.
* Vì sao: AUTO_TEST có nhiều bước kéo dài 1-2 giây, dài hơn thời gian watchdog.
* Đã sửa/thay bằng: Mỗi bước AUTO_TEST gọi `markCommandTime()` trong lúc chờ.
* Ảnh hưởng: AUTO_TEST chạy đúng thời lượng, còn khi test tay thì mất lệnh quá 500 ms vẫn dừng motor như yêu cầu an toàn.

### Vấn đề 9: PID chỉ là phần chuẩn bị

* Khác/sai ở đâu: Workshop BUỔI 2 có hướng PID-ready, nhưng project chưa đóng vòng PID thật.
* Vì sao: Chưa có encoder, feedback tốc độ và tải motor trong Wokwi.
* Đã sửa/thay bằng: Code giữ hàm điều khiển theo tham số `speed` và `angle`, dễ thay `motorSetSpeed()` bằng PID sau này.
* Ảnh hưởng: Lab hiện tại kiểm tra điều khiển mở vòng; PID cần thêm encoder và phần cứng thật hoặc mô phỏng riêng.

## 6. Kết luận

Project đã mô phỏng được phần điều khiển logic của BUỔI 2: pin map, chiều quay, PWM tốc độ, servo, frame RF, checksum và watchdog. Những phần vật lý như L298N thật, motor thật, nguồn 7-12V, HC-12 RF và PID có feedback cần kiểm tra trên phần cứng thật. Wokwi dùng tốt để kiểm tra code, pin map, PWM, UART frame và servo trước khi triển khai mạch thật.
