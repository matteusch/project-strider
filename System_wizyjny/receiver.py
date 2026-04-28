import serial
import numpy as np
import cv2
import struct
import time

DEVICE = '/dev/ttyUSB0' 
BAUD_RATE = 1000000
W, H = 160, 120
SCALE = 4

ser = serial.Serial(DEVICE, BAUD_RATE, timeout=1)
label_info = list()

last = time.time()

while True:
    if ser.read(1) == b'\xde':
        if ser.read(3) == b'\xad\xbe\xef':

            label_info.clear()

            interval = struct.unpack('<I', ser.read(4))[0]
            fps      = struct.unpack('<f', ser.read(4))[0]
            labels   = struct.unpack('<B', ser.read(1))[0]

            for i in range(labels):
                label=dict()
                label["index"] = struct.unpack('<B', ser.read(1))[0]
                label["area"] = struct.unpack('<I', ser.read(4))[0]
                label["x"] = struct.unpack('<f', ser.read(4))[0]
                label["y"] = struct.unpack('<f', ser.read(4))[0]
                label["roundness"] = struct.unpack('<f', ser.read(4))[0]

                label_info.append(label)

            size     = struct.unpack('<I', ser.read(4))[0]
            
            img = ser.read(size)

            print(f"Interval: {interval}, FPS: {fps:.2f}, labels: {labels}")
            
            if len(img) == size:
                img = np.frombuffer(img, dtype=np.uint8).reshape((H, W))
                preview = cv2.resize(img, (W*SCALE, H*SCALE), interpolation=cv2.INTER_NEAREST)
                preview = cv2.cvtColor(preview, cv2.COLOR_GRAY2BGR)

                for i in range(labels):
                    cv2.putText(
                        preview, 
                        f"{label_info[i]["index"]}", 
                        (int(label_info[i]["x"])*SCALE,int(label_info[i]["y"])*SCALE), 
                        cv2.FONT_HERSHEY_SIMPLEX, 
                        0.75, 
                        (0,0,255), 
                        2,
                    )

                    cv2.putText(
                        preview, 
                        f"Sylwetka {i+1}: A={label_info[i]["area"]:.2f}, p={label_info[i]["roundness"]:.2f}, xs={label_info[i]["x"]:.2f}, ys={label_info[i]["y"]:.2f}", 
                        (10,20+20*(i+1)), 
                        cv2.FONT_HERSHEY_SIMPLEX, 
                        0.5, (255,255,255), 
                        1
                    )

                time_diff = time.time() - last
                img_fps = 1.0 / time_diff

                cv2.putText(preview, f"FPS: {img_fps:.2f}", (10,20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255,255,255), 1)
                cv2.imshow("Podglad", preview)

                last = time.time()

                ser.reset_input_buffer()

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

ser.close()
cv2.destroyAllWindows()