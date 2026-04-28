import serial
import numpy as np
import cv2
import struct
import time

DEVICE = '/dev/ttyUSB0' 
BAUD_RATE = 1000000
W, H = 160, 120

ser = serial.Serial(DEVICE, BAUD_RATE, timeout=1)

last = time.time()

while True:
    if ser.read(1) == b'\xde':
        if ser.read(3) == b'\xad\xbe\xef':

            interval = struct.unpack('<I', ser.read(4))[0]
            fps      = struct.unpack('<f', ser.read(4))[0]
            labels   = struct.unpack('<B', ser.read(1))[0]
            size     = struct.unpack('<I', ser.read(4))[0]
            
            img = ser.read(size)

            print(f"Interval: {interval}, FPS: {fps:.2f}, labels: {labels}")
            
            if len(img) == size:
                img = np.frombuffer(img, dtype=np.uint8).reshape((H, W))
                preview = cv2.resize(img, (W*4, H*4), interpolation=cv2.INTER_NEAREST)

                time_diff = time.time() - last
                img_fps = 1.0 / time_diff

                cv2.putText(preview, f"FPS: {img_fps:.2f}", (10,20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255), 1)
                cv2.imshow("Podglad", preview)

                last = time.time()

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

ser.close()
cv2.destroyAllWindows()