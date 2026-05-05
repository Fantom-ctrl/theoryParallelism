import os 

os.environ["OPENCV_LOG_LEVEL"] = "SILENT"
os.environ["OPENCV_VIDEOIO_PRIORITY_FFMPEG"] = "0"
os.environ["OPENCV_VIDEOIO_DEBUG"] = "0"

import cv2
import time
import threading
import argparse
import logging
from queue import Queue, Empty


os.makedirs("log", exist_ok=True)

logging.basicConfig(
    filename="log/app.log",
    level=logging.INFO,
    format="%(asctime)s %(message)s"
)


class Sensor:
    def get(self):
        raise NotImplementedError()


class SensorX(Sensor):
    def __init__(self, delay):
        self.delay = delay
        self.data = 0

    def get(self):
        time.sleep(self.delay)
        self.data += 1
        return self.data


class SensorCam:
    def __init__(self, cam, res):
        self.cap = cv2.VideoCapture(cam)
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, res[0])
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, res[1])

        if not self.cap.isOpened():
            logging.error("Camera init failed")
            raise RuntimeError("Camera not found")

    def get(self):
        ret, frame = self.cap.read()
        return frame if ret else None

    def __del__(self):
        self.cap.release()


class WindowImage:
    def __init__(self, freq):
        self.delay = 1 / freq
        cv2.namedWindow("img")

    def show(self, img):
        cv2.imshow("img", img)
        cv2.waitKey(1)

    def __del__(self):
        cv2.destroyAllWindows()


def cam_worker(cam, q, stop):
    while not stop.is_set():
        frame = cam.get()
        if frame is None:
            stop.set()
            print("Read frame error")
            break
        q.put(frame)


def sensor_worker(sensor, q, stop):
    while not stop.is_set():
        q.put(sensor.get())


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cam", default="0")
    parser.add_argument("--res", default="640x480")
    parser.add_argument("--freq", type=float, default=30)
    args = parser.parse_args()

    w, h = map(int, args.res.split("x"))

    stop = threading.Event()

    cam_q = Queue(maxsize=1)
    s1_q = Queue(maxsize=1)
    s2_q = Queue(maxsize=1)
    s3_q = Queue(maxsize=1)

    try:
        cam = SensorCam(int(args.cam), (w, h))
    except Exception:
        print("Camera not found")
        return
    
    s1 = SensorX(0.01)
    s2 = SensorX(0.1)
    s3 = SensorX(1.0)

    win = WindowImage(args.freq)

    threads = [
        threading.Thread(target=cam_worker, args=(cam, cam_q, stop)),
        threading.Thread(target=sensor_worker, args=(s1, s1_q, stop)),
        threading.Thread(target=sensor_worker, args=(s2, s2_q, stop)),
        threading.Thread(target=sensor_worker, args=(s3, s3_q, stop)),
    ]

    for t in threads:
        t.start()

    last1 = last2 = last3 = 0

    while not stop.is_set():
        try:
            frame = cam_q.get(timeout=1)
        except Empty:
            continue

        try:
            last1 = s1_q.get_nowait()
        except Empty: 
            pass

        try: 
            last2 = s2_q.get_nowait()
        except Empty: 
            pass

        try: 
            last3 = s3_q.get_nowait()
        except Empty: 
            pass

        cv2.putText(frame, f"S1: {last1}", (20, 30), 0, 0.7, (0, 255, 0), 2)
        cv2.putText(frame, f"S2: {last2}", (20, 60), 0, 0.7, (255, 0, 0), 2)
        cv2.putText(frame, f"S3: {last3}", (20, 90), 0, 0.7, (0, 0, 255), 2)

        win.show(frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            stop.set()
            break

    for t in threads:
        t.join(timeout=1)

    del cam
    del win


if __name__ == "__main__":
    main()