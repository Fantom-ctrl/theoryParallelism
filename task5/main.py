import argparse
import time
import cv2
from ultralytics import YOLO
from multiprocessing import Process, Queue, cpu_count
from queue import Empty


class VideoCaptureRALL:
    def __init__(self, source):
        self.cap = cv2.VideoCapture(source)
        if not self.cap.isOpened():
            raise RuntimeError("Cannot open video source")

    def read(self):
        return self.cap.read()

    def release(self):
        self.cap.release()

    def __del__(self):
        self.release()


class VideoWriterRALL:
    def __init__(self, path, fps, size):
        fourcc = cv2.VideoWriter_fourcc(*"mp4v")
        self.writer = cv2.VideoWriter(path, fourcc, fps, size)

    def write(self, frame):
        self.writer.write(frame)

    def release(self):
        self.writer.release()

    def __del__(self):
        self.release()


class YOLOModel:
    def __init__(self):
        self.model = YOLO("yolov8s-pose.pt")

    def infer(self, frame):
        return self.model(frame, verbose=False)


def draw_results(results):
    return results[0].plot()


def worker(input_q, output_q):
    model = YOLO("yolov8s-pose.pt")

    while True:
        item = input_q.get()

        if item is None:
            break

        idx, frame = item

        results = model(frame, verbose=False)
        annotated = results[0].plot()

        output_q.put((idx, annotated))


def run_single(source, output_path):
    cap = VideoCaptureRALL(source)

    fps = cap.cap.get(cv2.CAP_PROP_FPS)
    if fps == 0:
        fps = 30

    w = int(cap.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    h = int(cap.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

    writer = VideoWriterRALL(output_path, fps, (w, h))
    model = YOLOModel()

    start = time.time()

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        results = model.infer(frame)
        frame = draw_results(results)

        if source == 0:
            cv2.imshow("frame", frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

        writer.write(frame)

    end = time.time()

    cap.release()
    writer.release()
    cv2.destroyAllWindows()

    print(f"{end - start:.4f}")


def run_multi(source, output_path, n_workers):
    cap = VideoCaptureRALL(source)

    fps = cap.cap.get(cv2.CAP_PROP_FPS)
    if fps == 0:
        fps = 60

    w = int(cap.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    h = int(cap.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

    writer = VideoWriterRALL(output_path, fps, (w, h))

    input_q = Queue(maxsize=n_workers * 2)
    output_q = Queue(maxsize=n_workers * 2)

    workers = []
    for _ in range(n_workers):
        p = Process(target=worker, args=(input_q, output_q))
        p.start()
        workers.append(p)

    start = time.time()

    frame_id = 0
    next_id = 0
    buffer = {}

    total_frames_sent = 0
    total_frames_received = 0

    stop = False

    try:
        while True:
            ret, frame = cap.read()

            if not ret:
                break

            input_q.put((frame_id, frame))

            total_frames_sent += 1
            frame_id += 1

            while True:
                try:
                    idx, out_frame = output_q.get_nowait()
                    buffer[idx] = out_frame
                    total_frames_received += 1
                except Empty:
                    break

            while next_id in buffer:
                frame_to_show = buffer.pop(next_id)

                if source == 0:
                    cv2.imshow("frame", frame_to_show)

                    if cv2.waitKey(1) & 0xFF == ord('q'):
                        stop = True
                        break

                writer.write(frame_to_show)

                next_id += 1

            if stop:
                break

    except KeyboardInterrupt:
        stop = True

    for _ in workers:
        input_q.put(None)

    while total_frames_received < total_frames_sent:
        idx, out_frame = output_q.get()

        buffer[idx] = out_frame
        total_frames_received += 1

        while next_id in buffer:
            frame_to_show = buffer.pop(next_id)

            if source == 0:
                cv2.imshow("frame", frame_to_show)

                if cv2.waitKey(1) & 0xFF == ord('q'):
                    stop = True

            writer.write(frame_to_show)

            next_id += 1

    for p in workers:
        p.join()

    cap.release()
    writer.release()

    input_q.close()
    output_q.close()

    cv2.destroyAllWindows()

    end = time.time()

    print(f"{end - start:.4f}")


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("--video_path", default=None)
    parser.add_argument("--output", default="output.mp4")
    parser.add_argument("--mode", choices=["s", "m"], default="s")
    parser.add_argument("--workers", type=int, default=cpu_count())
    parser.add_argument("--camera", action="store_true")

    args = parser.parse_args()

    if args.camera:
        source = 0
    else:
        if args.video_path is None:
            raise ValueError("Provide video_path or use --camera")
        source = args.video_path

    if args.mode == "s":
        run_single(source, args.output)
    else:
        run_multi(source, args.output, args.workers)


if __name__ == "__main__":
    main()