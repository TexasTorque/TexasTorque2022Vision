
import sys
from networktables import NetworkTables, NetworkTablesInstance
import threading

if '-l' not in sys.argv:
    import cv2
    from cscore import CameraServer
else:
    import opencv2
    
team = 1477
ip = "10.14.77.2"


def list_ports():
    """
    Test the ports and returns a tuple with the available ports and the ones that are working.
    """
    is_working = True
    dev_port = 0
    working_ports = []
    available_ports = []
    while is_working:
        camera = cv2.VideoCapture(dev_port)
        if not camera.isOpened():
            is_working = False
            print("Port %s is not working." %dev_port)
        else:
            is_reading, img = camera.read()
            w = camera.get(3)
            h = camera.get(4)
            if is_reading:
                print("Port %s is working and reads images (%s x %s)" %(dev_port,h,w))
                working_ports.append(dev_port)
            else:
                print("Port %s for camera ( %s x %s) is present but does not reads." %(dev_port,h,w))
                available_ports.append(dev_port)
        dev_port +=1
    return available_ports, working_ports

def main():
    notified = False
    condition = threading.Condition()
    # notify as soon as connection is made
    def connection_listener(connected, info):
        with condition:
            notified = True
            condition.notify()
    
    NetworkTables.initialize(server=ip)
    NetworkTables.addConnectionListener(connection_listener, immediateNotify=True)
    with condition:
        if not notified:
            condition.wait()

    ntinst = NetworkTablesInstance.getDefault()
    tb = ntinst.getTable("BallTable")
    tb.getEntry("ballentry").forceSetString("balls")
    
    
    
    _, working = list_ports()

    if len(working) <= 0:
        print("AHHHH no working motors")
        exit(-1)

    cs = CameraServer()
    output = cs.putVideo("Name", 480, 640)

    camera = cv2.VideoCapture(working[0])
    frameEntry = tb.getEntry("frameC")
    frameC = 0
    while True:
        _, frame = camera.read()
        frameC += 1
        frameEntry.forceSetNumber(frameC)
        frame = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        # print(frame.size)

        output.putFrame(frame)
    
if __name__ == "__main__":
    main()
  
    