from cscore import CameraServer
import cv2
import numpy as np

CameraServer.enableLogging()

camera = CameraServer.startAutomaticCapture()
camera.setResolution(width, height)

sink = cs.getVideo()
time, input_img = cvSink.grabFrame(input_img)

while True:
    time, input_img = cvSink.grabFrame(input_img)

    if time == 0: # There is an error
        output.notifyError(sink.getError())
        continue

    hsv_img = cv2.cvtColor(input_img, cv2.COLOR_BGR2HSV)

    output.putFrame(hsv_img)