import cv2
import mediapipe as mp
from picamera2 import Picamera2
import time
from adafruit_servokit import ServoKit

# --- Constants & Configuration ---
FRAME_WIDTH = 640
FRAME_HEIGHT = 480
FRAME_CENTER_X = FRAME_WIDTH // 2
FRAME_CENTER_Y = FRAME_HEIGHT // 2

# --- ServoKit (PCA9685) Configuration ---
try:
    kit = ServoKit(channels=16)
    PCA9685_AVAILABLE = True
    print("Adafruit ServoKit (PCA9685) initialized.")
except Exception as e:
    print(f"Error initializing ServoKit (PCA9685): {e}")
    PCA9685_AVAILABLE = False

PAN_SERVO_CHANNEL = 0
TILT_SERVO_CHANNEL = 1

PAN_MIN_ANGLE = 10
PAN_MAX_ANGLE = 110
INITIAL_PAN_ANGLE = 90

TILT_MIN_ANGLE = 10
TILT_MAX_ANGLE = 170
INITIAL_TILT_ANGLE = 85

current_pan_angle = INITIAL_PAN_ANGLE
current_tilt_angle = INITIAL_TILT_ANGLE

# Control Parameters
KP_PAN = 0.04
KP_TILT = 0.04
DEAD_ZONE_X = 25
DEAD_ZONE_Y = 25
MAX_ADJUSTMENT_SPEED = 1.25

# --- Picamera2 Initialization ---
picam2 = Picamera2()
config = picam2.create_preview_configuration(main={"size": (FRAME_WIDTH, FRAME_HEIGHT), "format": "RGB888"})
picam2.configure(config)
picam2.start()
time.sleep(1)

# --- Mediapipe Face Detection Initialization ---
mp_face_detection = mp.solutions.face_detection
mp_drawing = mp.solutions.drawing_utils
detector = mp_face_detection.FaceDetection(model_selection=0, min_detection_confidence=0.5)

# --- Helper Function for Servo Control ---
def set_servo_angle_pca9685(servo_channel, angle, current_angle_ref, min_angle_limit, max_angle_limit):
    if not PCA9685_AVAILABLE:
        return current_angle_ref
    angle = max(min_angle_limit, min(max_angle_limit, angle))
    try:
        kit.servo[servo_channel].angle = angle
    except Exception as e:
        print(f"Error setting servo CH{servo_channel} angle: {e}")
    return angle

# --- Initial Servo Position ---
if PCA9685_AVAILABLE:
    print(f"Setting initial pan: {INITIAL_PAN_ANGLE}°, tilt: {INITIAL_TILT_ANGLE}°")
    current_pan_angle = set_servo_angle_pca9685(PAN_SERVO_CHANNEL, INITIAL_PAN_ANGLE, current_pan_angle, PAN_MIN_ANGLE, PAN_MAX_ANGLE)
    current_tilt_angle = set_servo_angle_pca9685(TILT_SERVO_CHANNEL, INITIAL_TILT_ANGLE, current_tilt_angle, TILT_MIN_ANGLE, TILT_MAX_ANGLE)
    time.sleep(0.5)

print("Starting face tracking... Press Ctrl+C to exit.")

# --- Main Loop ---
try:
    while True:
        frame_raw = picam2.capture_array()
        image_rgb = frame_raw  # Already RGB888 format

        results = detector.process(image_rgb)

        target_found = False
        target_center_x = 0
        target_center_y = 0
        largest_area = 0

        if results.detections:
            for detection in results.detections:
                box = detection.location_data.relative_bounding_box
                ih, iw, _ = image_rgb.shape
                x, y, w, h = int(box.xmin * iw), int(box.ymin * ih), \
                             int(box.width * iw), int(box.height * ih)
                area = w * h

                if area > largest_area:
                    largest_area = area
                    target_center_x = x + w // 2
                    target_center_y = y + h // 2
                    target_found = True

                mp_drawing.draw_detection(frame_raw, detection)

        if target_found:
            cv2.circle(frame_raw, (target_center_x, target_center_y), 7, (0, 255, 0), -1)
            cv2.putText(frame_raw, "FACE", (target_center_x + 10, target_center_y),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

            if PCA9685_AVAILABLE:
                # --- Pan Control (Reversed direction) ---
                error_pan = FRAME_CENTER_X - target_center_x
                if abs(error_pan) > DEAD_ZONE_X:
                    pan_adjustment = KP_PAN * error_pan
                    pan_adjustment = max(-MAX_ADJUSTMENT_SPEED, min(MAX_ADJUSTMENT_SPEED, pan_adjustment))
                    current_pan_angle += pan_adjustment  # Inverted direction here
                    current_pan_angle = set_servo_angle_pca9685(PAN_SERVO_CHANNEL, current_pan_angle, current_pan_angle, PAN_MIN_ANGLE, PAN_MAX_ANGLE)

                # --- Tilt Control ---
                error_tilt = FRAME_CENTER_Y - target_center_y
                if abs(error_tilt) > DEAD_ZONE_Y:
                    tilt_adjustment = KP_TILT * error_tilt
                    tilt_adjustment = max(-MAX_ADJUSTMENT_SPEED, min(MAX_ADJUSTMENT_SPEED, tilt_adjustment))
                    current_tilt_angle -= tilt_adjustment
                    current_tilt_angle = set_servo_angle_pca9685(TILT_SERVO_CHANNEL, current_tilt_angle, current_tilt_angle, TILT_MIN_ANGLE, TILT_MAX_ANGLE)

        # Draw center crosshairs
        cv2.line(frame_raw, (FRAME_CENTER_X - 10, FRAME_CENTER_Y), (FRAME_CENTER_X + 10, FRAME_CENTER_Y), (0, 0, 255), 1)
        cv2.line(frame_raw, (FRAME_CENTER_X, FRAME_CENTER_Y - 10), (FRAME_CENTER_X, FRAME_CENTER_Y + 10), (0, 0, 255), 1)

        cv2.imshow('RPi Face Tracker', frame_raw)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

except KeyboardInterrupt:
    print("\nExiting program.")
finally:
    print("Cleaning up...")
    if 'detector' in locals() and detector:
        detector.close()
    if 'picam2' in locals() and picam2.started:
        picam2.stop()
    cv2.destroyAllWindows()
    print("Done.")
