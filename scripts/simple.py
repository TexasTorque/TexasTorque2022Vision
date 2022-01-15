from cscore import CameraServer
import cv2
import numpy as np

CameraServer.enableLogging()

width = 320
height = 240


print(1)

cs = CameraServer()
camera = cs.startAutomaticCapture()
camera.setResolution(width, height)

print(2)

input_img = np.zeros((width, height, 3), dtype=np.uint8)

print(3)

#output = camera.putVideo("Name", width, height)

print(4)

cvSink = camera.enumerateSinks()[0]

print(4.5)

time, input_img = cvSink.grabFrame(input_img)

print(6)

while True:
    time, input_img = cvSink.grabFrame(input_img)

    print(7)

    if time == 0: # There is an error
        print(7.5)

        output.notifyError(cvSink.getError())
        continue

    print(8)

    hsv_img = cv2.cvtColor(input_img, cv2.COLOR_BGR2HSV)

    print(9)

    output.putFrame(hsv_img)