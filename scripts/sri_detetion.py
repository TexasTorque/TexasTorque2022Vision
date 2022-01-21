#!/usr/bin/env python3

import cv2
import numpy as np 

red = True

cap = cv2.VideoCapture(0)
callibration = cv2.imread("/Users/nayukumbham/Pictures/rr_calibration.jpg")

def find_marker(callibration):
    hsv = cv2.cvtColor(callibration, cv2.COLOR_BGR2HSV)

    lower_blue = np.array([110,50,50])
    upper_blue = np.array([130,255,255])

    lower_red = np.array([0, 50, 20])
    upper_red = np.array([5, 255, 255])

    mask = cv2.inRange(hsv, lower_red, upper_red)

    contours, _ = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    c = max(contours, key = cv2.contourArea)

    return cv2.minAreaRect(c)


while(1):
    _, frame = cap.read()
    #frame = cv2.imread("imgs/powerup_8_ft.jpg")
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    
    lower_blue = np.array([110,50,50])
    upper_blue = np.array([130,255,255])

    lower_red = np.array([0, 50, 20])
    upper_red = np.array([5, 255, 255])

 
    mask = cv2.inRange(hsv, lower_red, upper_red)
    res = cv2.bitwise_and(frame, frame, mask = mask)

    contours, _ = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    
    maxContour = []

    for contour in contours:
        if len(contour) > len(maxContour):
            maxContour = contour

    x, y, w, h = 0, 0, 0, 0
    if len(maxContour) > 0:


        x, y, w, h = cv2.boundingRect(maxContour)
        
        
        

        cv2.rectangle(frame,(x,y),(x+w,y+h),(255,0,0),3)

        p1 = [x, y]
        p2 = [x, y + h]
        p3 = [x + w, y]
        p4 = [x + w, y + h]

        centerX = (p3[0] - p1[0]) / 2
        centerY = (p2[1] - p1[1]) / 2
        centerPoint = [p1[0] + centerX, p1[1] + centerY]

        imageCircle = cv2.circle(frame, center = (int(centerPoint[0]), int(centerPoint[1])), radius = 5, color = (56, 247, 35), thickness = 2)


        frameWidth  = cap.get(cv2.CAP_PROP_FRAME_WIDTH)
        frameHeight = cap.get(cv2.CAP_PROP_FRAME_HEIGHT)
        cpirto = [centerPoint[0] - (frameWidth / 2), -(centerPoint[1] - (frameHeight / 2))]

        c = max(maxContour, key = cv2.contourArea)
        marker = cv2.minAreaRect(c)

    cv2.imshow('frame', frame)
    #cv2.imshow('res', res)

    k = cv2.waitKey(5) & 0xFF
    if k == 27:
        break




cv2.imshow('image', frame)
cv2.waitKey()
#if the loop breaks; the instance breaks 
cv2.destroyAllWindows()
cap.release() 





    
    
    
    