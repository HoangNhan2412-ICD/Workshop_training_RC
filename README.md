Wokwi ESP32 RC Car Lab
Project mô phỏng Lab 1: ESP32 điều khiển L298N + DC motor + servo.
Trong Wokwi, L298N/DC motor được mô phỏng bằng:
LED IN1 trên GPIO26.
LED IN2 trên GPIO27.
LED ENA_PWM và Logic Analyzer trên GPIO25.
Servo wokwi-servo trên GPIO18.
Cách chạy trên Wokwi
Tạo project Arduino ESP32 trên Wokwi.
Copy 4 file vào project:sketch.ino
diagram.json
libraries.txt
README.md

Bấm Start Simulation.
Mặc định AUTO_TEST = 1, chương trình tự chạy bài lab.
Nếu muốn test tay, đổi #define AUTO_TEST 0, sau đó dùng Serial Monitor:F: tiến.
B: lùi.
L: rẽ trái.
R: rẽ phải.
S: dừng.
180 90: chạy speed 180, servo 90.

Expected behavior
Tiến: IN1 sáng, IN2 tắt, ENA có PWM.
Lùi: IN1 tắt, IN2 sáng, ENA có PWM.
Dừng: IN1 tắt, IN2 tắt, ENA = 0.
Servo đổi góc 90, 135, 90 theo auto test.
Logic Analyzer:D0 = ENA_PWM GPIO25.
D1 = IN1 GPIO26.
D2 = IN2 GPIO27.
D3 = SERVO GPIO18.

CHECK SAI / CẦN SỬA CHI TIẾT
1. Sai pin IN1/IN2/ENA/SERVO
Sai cái gì: cần kiểm tra cả sketch.ino và diagram.json. Pin bắt buộc là IN1 = GPIO26, IN2 = GPIO27, ENA_PWM = GPIO25, SERVO_PIN = GPIO18.
Ảnh hưởng gì: nếu sai pin, LED IN1/IN2 không phản ánh đúng chiều motor, PWM ENA không hiện trên LED/Logic Analyzer, hoặc servo không quay đúng tín hiệu.
Đã sửa gì: trong sketch.ino đã đặt IN1 = 26, IN2 = 27, ENA_PWM = 25, SERVO_PIN = 18. Trong diagram.json đã nối esp:26 tới LED IN1 và Logic Analyzer D1, esp:27 tới LED IN2 và D2, esp:25 tới LED ENA_PWM và D0, esp:18 tới servo PWM và D3.
Trạng thái sau sửa: pin code và pin sơ đồ đã khớp với yêu cầu lab.
2. Servo thiếu V+ hoặc GND
Sai cái gì: servo wokwi-servo cần đủ 3 dây PWM, V+, GND. Lỗi thường nằm trong diagram.json nếu chỉ nối PWM mà quên nguồn hoặc mass.
Ảnh hưởng gì: servo có thể không hoạt động hoặc không đổi góc trong mô phỏng, dù code đã gọi steeringServo.write(angle).
Đã sửa gì: trong diagram.json, servo1:PWM đã nối tới esp:18, servo1:V+ đã nối tới esp:5V, và servo1:GND đã nối tới esp:GND.2.
Trạng thái sau sửa: servo đã có đủ tín hiệu điều khiển, nguồn và GND.
3. PWM không thấy vì chưa attach đúng GPIO25/channel
Sai cái gì: trong sketch.ino, nếu PWM attach sai pin hoặc trùng channel với servo, tín hiệu ENA_PWM trên GPIO25 có thể không xuất hiện đúng.
Ảnh hưởng gì: LED ENA_PWM không sáng theo độ rộng xung, Logic Analyzer D0 không thấy PWM, nên không quan sát được mức speed 30%, 60%, 100%.
Đã sửa gì: trong sketch.ino, PWM đã attach đúng GPIO25. Với Arduino-ESP32 core mới dùng ledcAttach(ENA_PWM, PWM_FREQ, PWM_RESOLUTION). Với core cũ dùng ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION) và ledcAttachPin(ENA_PWM, PWM_CHANNEL). Channel đã đặt là PWM_CHANNEL = 8 để giảm khả năng đụng với servo.
Trạng thái sau sửa: ENA_PWM đã xuất ra GPIO25 và được nối tới LED ENA_PWM cùng Logic Analyzer D0.
4. Watchdog làm xe dừng quá sớm trong AUTO_TEST
Sai cái gì: watchdog yêu cầu nếu quá 500ms không có lệnh mới thì motorStop(). Trong AUTO_TEST, nếu không cập nhật lastCmdTime trong lúc delay 1-2 giây, motor sẽ bị dừng giữa bước test.
Ảnh hưởng gì: auto test không giữ được tiến/lùi đủ thời gian, LED IN1/IN2 và PWM ENA bị tắt sớm hơn mô tả bài lab.
Đã sửa gì: trong sketch.ino, hàm autoStep() gọi markCommandTime() lặp lại mỗi 20ms trong thời gian chờ, đồng thời vẫn gọi checkWatchdog().
Trạng thái sau sửa: AUTO_TEST chạy đủ từng bước 2 giây hoặc 1 giây mà không bị watchdog dừng sớm.
5. README nói có lệnh Serial 180 90 nhưng code chưa parse lệnh này
Sai cái gì: nếu README hướng dẫn nhập 180 90 nhưng sketch.ino chỉ xử lý ký tự F/B/L/R/S, người dùng không test được speed và angle tùy ý.
Ảnh hưởng gì: Serial Monitor báo không hiểu lệnh hoặc xe không chạy theo giá trị nhập tay.
Đã sửa gì: trong sketch.ino, hàm handleSerialCommand() đã parse chuỗi dạng speed angle, ví dụ 180 90, bằng cách tách theo dấu cách và gọi rcDrive(speed, angle). Code cũng thay dấu phẩy bằng dấu cách, nên 180,90 vẫn có thể dùng.
Trạng thái sau sửa: README và code đã khớp nhau; lệnh 180 90 chạy được khi AUTO_TEST = 0.
6. WebSocket không nên đưa vào project Wokwi Lab 1
Sai cái gì: WebSocket không thuộc mục tiêu Lab 1 mô phỏng L298N + DC motor + servo. Nếu đưa WebSocket vào project Wokwi này, cấu hình sẽ phức tạp hơn vì cần gateway hoặc port forwarding.
Ảnh hưởng gì: người học dễ nhầm trọng tâm, mô phỏng có thể không chạy như lab cơ bản, và lỗi mạng sẽ che mất lỗi điều khiển motor/servo.
Đã sửa gì: project này không thêm WebSocket. Điều khiển tự động dùng AUTO_TEST, điều khiển tay dùng Serial Monitor.
Trạng thái sau sửa: project giữ đúng phạm vi Lab 1; WebSocket nên tách thành project hoặc mode riêng cho phần hardware thật.
