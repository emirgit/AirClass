import cv2
import numpy as np
import mediapipe as mp
import pandas as pd
from tensorflow.keras.models import load_model
from sklearn.preprocessing import LabelEncoder
import pickle
import time
import math
import json
import os
import stat
import signal
import sys
import threading
from queue import Queue
from picamera2 import Picamera2
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
PAN_MAX_ANGLE = 140
INITIAL_PAN_ANGLE = 90

TILT_MIN_ANGLE = 10
TILT_MAX_ANGLE = 170
INITIAL_TILT_ANGLE = 85

current_pan_angle = INITIAL_PAN_ANGLE
current_tilt_angle = INITIAL_TILT_ANGLE

# Face Tracking Control Parameters
KP_PAN = 0.04
KP_TILT = 0.04
DEAD_ZONE_X = 75
DEAD_ZONE_Y = 75
MAX_ADJUSTMENT_SPEED = 1

# --- Gesture Recognition Configuration ---
DEBUG = True
gesture_threshold = 200
GESTURE_COOLDOWN = 1
TWO_UP_DISTANCE_THRESHOLD = 0.05
DRAWING_MODE = False
last_tracked_gesture = None

# Zoom mode configuration
ZOOM_CONFIG = {
    "toggle_gesture": "ok",
    "active": False
}

# Named pipe for interprocess communication
PIPE_PATH = "/tmp/gesture_pipe"
pipe_fd = None

# --- Thread Control Variables ---
running = True
frame_lock = threading.Lock()
current_frame = None
face_results_lock = threading.Lock()
gesture_results_lock = threading.Lock()

# Shared data between threads
face_data = {
    'target_found': False,
    'target_center_x': 0,
    'target_center_y': 0,
    'detection': None
}

gesture_data = {
    'gesture_text': 'IDLE',
    'landmarks': None,
    'is_right_hand': True,
    'hand_detected': False
}

# --- Picamera2 Initialization ---
picam2 = Picamera2()
config = picam2.create_preview_configuration(main={"size": (FRAME_WIDTH, FRAME_HEIGHT), "format": "RGB888"})
picam2.configure(config)
picam2.start()
time.sleep(1)
print("PiCamera2 initialized for combined face tracking and gesture recognition.")

# --- MediaPipe Initialization ---
# These will be initialized in their respective threads to avoid conflicts
mp_face_detection = mp.solutions.face_detection
mp_drawing = mp.solutions.drawing_utils
mp_hands = mp.solutions.hands

# --- Load Gesture Recognition Model ---
try:
    model = load_model('gesture_recognizer.keras')
    print("Gesture recognition model loaded.")
except Exception as e:
    print(f"Error loading gesture model: {e}")
    model = None

# Load label encoder
try:
    with open('label_encoder.pkl', 'rb') as f:
        le = pickle.load(f)
        print(f"Recognizable gestures: {', '.join(le.classes_)}")
except FileNotFoundError:
    print("Label encoder not found. Gesture recognition disabled.")
    le = None

# Load scaler
try:
    with open('scaler.pkl', 'rb') as f:
        scaler = pickle.load(f)
except FileNotFoundError:
    print("Scaler file not found! Using default MinMaxScaler.")
    from sklearn.preprocessing import MinMaxScaler
    scaler = MinMaxScaler()

# --- Helper Functions ---
def clear_websocket_file():
    """Clear the websocket.txt file at program startup"""
    if DEBUG:
        try:
            with open('websocket.txt', 'w') as f:
                pass
            print("Websocket debug file cleared")
        except Exception as e:
            print(f"Error clearing websocket file: {e}")

def setup_pipe():
    global pipe_fd
    pipe_fd = None
    
    try:
        if os.path.exists(PIPE_PATH):
            os.unlink(PIPE_PATH)
        
        os.mkfifo(PIPE_PATH, mode=0o666)
        print(f"Created named pipe: {PIPE_PATH}")
        
        signal.signal(signal.SIGINT, cleanup_handler)
        signal.signal(signal.SIGTERM, cleanup_handler)
        
    except Exception as e:
        print(f"Error setting up pipe: {e}")

def cleanup_handler(sig, frame):
    global running
    print("\nCleaning up and exiting...")
    running = False
    
    try:
        if pipe_fd is not None and pipe_fd > 0:
            os.close(pipe_fd)
        if os.path.exists(PIPE_PATH):
            os.unlink(PIPE_PATH)
    except Exception as e:
        print(f"Error during cleanup: {e}")
    finally:
        if 'picam2' in locals() and picam2.started:
            picam2.stop()
        cv2.destroyAllWindows()
        sys.exit(0)

def set_servo_angle_pca9685(servo_channel, angle, current_angle_ref, min_angle_limit, max_angle_limit):
    if not PCA9685_AVAILABLE:
        return current_angle_ref
    angle = max(min_angle_limit, min(max_angle_limit, angle))
    try:
        kit.servo[servo_channel].angle = angle
    except Exception as e:
        print(f"Error setting servo CH{servo_channel} angle: {e}")
    return angle

def send_websocket(command, position_data=None):
    """Send command to the C++ program via the named pipe"""
    global pipe_fd
    
    message = {
        "command": command,
    }
    
    if position_data:
        message["position"] = position_data
    
    json_str = json.dumps(message) + "\n"
    
    try:
        if pipe_fd is None:
            try:
                pipe_fd = os.open(PIPE_PATH, os.O_WRONLY | os.O_NONBLOCK)
                print("Pipe opened for writing")
            except OSError as e:
                if e.errno == 6:
                    if DEBUG:
                        print(f"Command: {command}")
                    return
                else:
                    raise
        
        try:
            os.write(pipe_fd, json_str.encode('utf-8'))
            if DEBUG:
                print(f"Sent: {command}")
        except BlockingIOError:
            if DEBUG:
                print("Pipe is full, skipping message")
        except BrokenPipeError:
            print("Reader disconnected, closing pipe")
            if pipe_fd is not None:
                os.close(pipe_fd)
                pipe_fd = None
            
    except Exception as e:
        print(f"Error sending command: {e}")
        if pipe_fd is not None:
            try:
                os.close(pipe_fd)
            except:
                pass
            pipe_fd = None

def extract_hand_features(hand_landmarks):
    features = {}
    for i, landmark in enumerate(hand_landmarks.landmark):
        features[f'x{i}'] = landmark.x
        features[f'y{i}'] = landmark.y
        features[f'z{i}'] = landmark.z
    return pd.DataFrame([features])

def calculate_landmark_distance(landmark1, landmark2):
    """Calculate Euclidean distance between two landmarks"""
    dx = landmark1.x - landmark2.x
    dy = landmark1.y - landmark2.y
    dz = landmark1.z - landmark2.z
    return math.sqrt(dx*dx + dy*dy + dz*dz)

def calculate_angle(p1, p2, p3):
    """Calculate angle between three points"""
    v1 = np.array([p1.x - p2.x, p1.y - p2.y])
    v2 = np.array([p3.x - p2.x, p3.y - p2.y])
    
    cosine = np.dot(v1, v2) / (np.linalg.norm(v1) * np.linalg.norm(v2))
    angle = np.arccos(np.clip(cosine, -1.0, 1.0))
    return np.degrees(angle)

def toggle_zoom_mode(direction=None):
    """Toggle between normal mode and zoom mode"""
    global ZOOM_CONFIG
    
    if direction == "on":
        send_websocket("zoom_in")
        if not ZOOM_CONFIG["active"]:
            ZOOM_CONFIG["active"] = True
            print("Mode switched to: ZOOM MODE")
        return True
    
    elif direction == "off":
        if ZOOM_CONFIG["active"]:
            ZOOM_CONFIG["active"] = False
            print("Mode switched to: NORMAL MODE")
            return False
        return ZOOM_CONFIG["active"]
    
    else:
        ZOOM_CONFIG["active"] = not ZOOM_CONFIG["active"]
        mode_name = "ZOOM" if ZOOM_CONFIG["active"] else "NORMAL"
        print(f"Mode switched to: {mode_name} MODE")
        
        if ZOOM_CONFIG["active"]:
            send_websocket("zoom_in")
        
        return ZOOM_CONFIG["active"]

# --- Face Tracking Thread ---
def face_tracking_thread():
    global running, current_pan_angle, current_tilt_angle, face_data
    
    # Initialize face detection for this thread
    face_detector = mp_face_detection.FaceDetection(model_selection=0, min_detection_confidence=0.5)
    
    print("Face tracking thread started")
    
    try:
        while running:
            # Get current frame
            with frame_lock:
                if current_frame is None:
                    time.sleep(0.01)
                    continue
                frame_copy = current_frame.copy()
            
            # Process face detection
            image_rgb = cv2.flip(frame_copy, 1)  # Flip back for processing
            face_results = face_detector.process(image_rgb)
            
            target_found = False
            target_center_x = 0
            target_center_y = 0
            largest_area = 0
            detection_for_display = None

            if face_results.detections:
                for detection in face_results.detections:
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
                        detection_for_display = detection

            # Update shared face data
            with face_results_lock:
                face_data['target_found'] = target_found
                face_data['target_center_x'] = target_center_x
                face_data['target_center_y'] = target_center_y
                face_data['detection'] = detection_for_display

            # Servo control for face tracking
            if target_found and PCA9685_AVAILABLE:
                # Pan Control
                error_pan = FRAME_CENTER_X - target_center_x
                if abs(error_pan) > DEAD_ZONE_X:
                    pan_adjustment = KP_PAN * error_pan
                    pan_adjustment = max(-MAX_ADJUSTMENT_SPEED, min(MAX_ADJUSTMENT_SPEED, pan_adjustment))
                    current_pan_angle += pan_adjustment
                    current_pan_angle = set_servo_angle_pca9685(PAN_SERVO_CHANNEL, current_pan_angle, current_pan_angle, PAN_MIN_ANGLE, PAN_MAX_ANGLE)

                # Tilt Control
                error_tilt = FRAME_CENTER_Y - target_center_y
                if abs(error_tilt) > DEAD_ZONE_Y:
                    tilt_adjustment = KP_TILT * error_tilt
                    tilt_adjustment = max(-MAX_ADJUSTMENT_SPEED, min(MAX_ADJUSTMENT_SPEED, tilt_adjustment))
                    current_tilt_angle -= tilt_adjustment
                    current_tilt_angle = set_servo_angle_pca9685(TILT_SERVO_CHANNEL, current_tilt_angle, current_tilt_angle, TILT_MIN_ANGLE, TILT_MAX_ANGLE)
            
            time.sleep(0.03)  # ~30 FPS for face tracking
            
    except Exception as e:
        print(f"Face tracking thread error: {e}")
    finally:
        face_detector.close()
        print("Face tracking thread stopped")

# --- Gesture Recognition Thread ---
def gesture_recognition_thread():
    global running, gesture_data, last_tracked_gesture, DRAWING_MODE
    
    # Initialize hand detection for this thread
    hands = mp_hands.Hands(
        static_image_mode=False,
        max_num_hands=1,
        min_detection_confidence=0.5,
        min_tracking_confidence=0.5
    )
    
    # Gesture handling variables
    last_gesture_time = 0
    last_output_time = time.time()
    current_gesture = "IDLE"
    gesture_start_time = None
    stable_duration = 0.1
    
    print("Gesture recognition thread started")
    
    try:
        while running:
            # Get current frame
            with frame_lock:
                if current_frame is None:
                    time.sleep(0.01)
                    continue
                frame_copy = current_frame.copy()
            
            # Process hand detection
            hand_results = hands.process(frame_copy)
            
            gesture_text = "IDLE"
            current_hand_landmarks = None
            is_right_hand = True
            hand_detected = False

            if hand_results.multi_hand_landmarks and model and le:
                primary_hand = hand_results.multi_hand_landmarks[0]
                hand_detected = True
                
                if hand_results.multi_handedness:
                    hand_label = hand_results.multi_handedness[0].classification[0].label
                    is_right_hand = hand_label == "Right"

                current_hand_landmarks = primary_hand.landmark
                
                # Extract hand features and predict gesture
                features_df = extract_hand_features(primary_hand)
                
                expected_columns = [f'{coord}{i}' for i in range(21) for coord in ['x', 'y', 'z']]
                for col in expected_columns:
                    if col not in features_df.columns:
                        features_df[col] = 0.0
                        
                features_df = features_df[expected_columns]
                features_array = scaler.transform(features_df)
                
                prediction = model.predict(features_array, verbose=0)
                gesture_index = np.argmax(prediction)
                gesture = le.classes_[gesture_index]
                confidence = prediction[0][gesture_index]

                if confidence > 0.70:
                    detected_gesture = gesture
                    
                    if detected_gesture != current_gesture:
                        current_gesture = detected_gesture
                        gesture_start_time = time.time()
                    
                    if time.time() - gesture_start_time >= stable_duration:
                        gesture_text = current_gesture
                    
                    if time.time() - last_output_time >= 1.0:
                        last_output_time = time.time()
                else:
                    if DEBUG:
                        gesture_text = f"Uncertain ({confidence:.2f})"
                    current_gesture = "IDLE"
            else:
                gesture_text = "IDLE"
                current_gesture = "IDLE"
            
            # Process gesture commands
            if current_hand_landmarks:
                current_time = time.time()
                frame_height = FRAME_HEIGHT
                frame_width = FRAME_WIDTH

                # Handle gesture logic (same as original but in thread)
                if gesture_text == "ok" and current_time - last_gesture_time >= GESTURE_COOLDOWN:
                    toggle_zoom_mode(direction="on")
                    last_gesture_time = current_time

                elif gesture_text == "palm" and int(current_hand_landmarks[0].y * frame_height) < gesture_threshold and ZOOM_CONFIG["active"]:
                    if current_time - last_gesture_time >= GESTURE_COOLDOWN:
                        toggle_zoom_mode(direction="off")
                        send_websocket("zoom_reset")
                        last_gesture_time = current_time

                elif gesture_text == "two_up":
                    index_tip = current_hand_landmarks[8]
                    position_data = {"x": index_tip.x, "y": index_tip.y}
                    send_websocket("two_up", position_data)
                    DRAWING_MODE = False

                elif gesture_text == "one" and last_tracked_gesture == "two_up":
                    DRAWING_MODE = True
                
                if DRAWING_MODE and current_hand_landmarks:
                    index_tip = current_hand_landmarks[8]
                    x = int(index_tip.x * frame_width)
                    y = int(index_tip.y * frame_height)
                    position_data = {"x": x, "y": y}
                    send_websocket("one", position_data)
                
                # Process other gestures with cooldown
                if current_time - last_gesture_time >= GESTURE_COOLDOWN:
                    if ZOOM_CONFIG["active"]:
                        # Zoom mode gesture mappings
                        if gesture_text == "like":
                            send_websocket("up")
                        elif gesture_text == "dislike":
                            send_websocket("down")
                        elif gesture_text == "three_gun":
                            thumb_tip = current_hand_landmarks[4]
                            index_tip = current_hand_landmarks[8]
                            
                            if thumb_tip.x < index_tip.x:
                                send_websocket("right")
                            else:
                                send_websocket("left")
                    else:
                        # Normal mode - original gesture handling
                        gesture_commands = {
                            "call": "call",
                            "rock": "rock",
                            "three": "three",
                            "three2": "three2",
                            "timeout": "timeout",
                            "take_picture": "take_picture",
                            "hand_heart": "heart",
                            "hand_heart2": "hand_heart2",
                            "middle_finger": "mid_finger",
                            "thumb_index": "thumb_index",
                            "holy": "holy",
                            "three_3": "three_3"
                        }
                        
                        if gesture_text in gesture_commands:
                            send_websocket(gesture_commands[gesture_text])
                        elif gesture_text == "dislike":
                            angle = calculate_angle(current_hand_landmarks[4], current_hand_landmarks[2], current_hand_landmarks[5])
                            
                            if is_right_hand:
                                index_closed = current_hand_landmarks[8].x > current_hand_landmarks[5].x
                                middle_closed = current_hand_landmarks[12].x > current_hand_landmarks[9].x
                                ring_closed = current_hand_landmarks[16].x > current_hand_landmarks[13].x
                                pinky_closed = current_hand_landmarks[20].x > current_hand_landmarks[17].x
                            else:
                                index_closed = current_hand_landmarks[8].x < current_hand_landmarks[5].x
                                middle_closed = current_hand_landmarks[12].x < current_hand_landmarks[9].x
                                ring_closed = current_hand_landmarks[16].x < current_hand_landmarks[13].x
                                pinky_closed = current_hand_landmarks[20].x < current_hand_landmarks[17].x
                            
                            if 75 < angle < 130 and index_closed and middle_closed and ring_closed and pinky_closed:
                                send_websocket("dislike")
                        elif gesture_text == "like":
                            angle = calculate_angle(current_hand_landmarks[4], current_hand_landmarks[2], current_hand_landmarks[5])
                            if 75 < angle < 120:
                                send_websocket("like")
                        elif gesture_text == "three_gun":
                            thumb_tip = current_hand_landmarks[4]
                            index_tip = current_hand_landmarks[8]
                            
                            if thumb_tip.x < index_tip.x:
                                send_websocket("inv_three_gun")
                            else:
                                send_websocket("three_gun")
                        elif gesture_text == "palm" and int(current_hand_landmarks[1].y * frame_height) < gesture_threshold:
                            send_websocket("palm")
                            
                    last_tracked_gesture = gesture_text
                    last_gesture_time = current_time

            # Update shared gesture data
            with gesture_results_lock:
                gesture_data['gesture_text'] = gesture_text
                gesture_data['landmarks'] = current_hand_landmarks
                gesture_data['is_right_hand'] = is_right_hand
                gesture_data['hand_detected'] = hand_detected
            
            time.sleep(0.05)  # ~20 FPS for gesture recognition
            
    except Exception as e:
        print(f"Gesture recognition thread error: {e}")
    finally:
        hands.close()
        print("Gesture recognition thread stopped")

# --- Initialize System ---
clear_websocket_file()
setup_pipe()

# Set initial servo positions
if PCA9685_AVAILABLE:
    print(f"Setting initial pan: {INITIAL_PAN_ANGLE}°, tilt: {INITIAL_TILT_ANGLE}°")
    current_pan_angle = set_servo_angle_pca9685(PAN_SERVO_CHANNEL, INITIAL_PAN_ANGLE, current_pan_angle, PAN_MIN_ANGLE, PAN_MAX_ANGLE)
    current_tilt_angle = set_servo_angle_pca9685(TILT_SERVO_CHANNEL, INITIAL_TILT_ANGLE, current_tilt_angle, TILT_MIN_ANGLE, TILT_MAX_ANGLE)
    time.sleep(0.5)

print("Starting threaded face tracking and gesture recognition system...")
print("Face tracking with servo control enabled" if PCA9685_AVAILABLE else "Face tracking display only (no servo control)")
print("Press 'q' to quit")

# --- Start Threads ---
face_thread = threading.Thread(target=face_tracking_thread, daemon=True)
gesture_thread = threading.Thread(target=gesture_recognition_thread, daemon=True)

face_thread.start()
gesture_thread.start()

# --- Main Loop ---
try:
    while running:
        # Capture frame
        frame_raw = picam2.capture_array()
        frame = cv2.flip(frame_raw, 1)  # Flip for display
        
        # Update shared frame
        with frame_lock:
            current_frame = frame_raw
        
        # Get face tracking results
        with face_results_lock:
            face_info = face_data.copy()
        
        # Get gesture recognition results
        with gesture_results_lock:
            gesture_info = gesture_data.copy()
        
        # Draw face tracking results
        if face_info['target_found']:
            cv2.circle(frame, (face_info['target_center_x'], face_info['target_center_y']), 7, (0, 255, 0), -1)
            cv2.putText(frame, "FACE", (face_info['target_center_x'] + 10, face_info['target_center_y']),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
            
            if face_info['detection']:
                mp_drawing.draw_detection(frame, face_info['detection'])
        
        # Draw gesture recognition results
        if gesture_info['hand_detected'] and gesture_info['landmarks']:
            # Draw hand landmarks
            if DEBUG:
                # Convert landmarks to MediaPipe format for drawing
                landmarks = gesture_info['landmarks']
                hand_landmarks = type('HandLandmarks', (), {})()
                hand_landmarks.landmark = landmarks
                mp_drawing.draw_landmarks(frame, hand_landmarks, mp_hands.HAND_CONNECTIONS)
            
            # Draw gesture-specific visualizations
            if gesture_info['gesture_text'] == "two_up" and gesture_info['landmarks']:
                index_tip = gesture_info['landmarks'][8]
                x = int(index_tip.x * FRAME_WIDTH)
                y = int(index_tip.y * FRAME_HEIGHT)
                cv2.circle(frame, (x, y), 15, (255, 165, 0), -1)
                cv2.putText(frame, "TWO UP POINTING", (10, FRAME_HEIGHT - 80),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 165, 0), 2, cv2.LINE_AA)
            
            elif DRAWING_MODE and gesture_info['landmarks']:
                index_tip = gesture_info['landmarks'][8]
                x = int(index_tip.x * FRAME_WIDTH)
                y = int(index_tip.y * FRAME_HEIGHT)
                cv2.circle(frame, (x, y), 15, (0, 255, 255), -1)
                cv2.putText(frame, "DRAWING MODE", (10, FRAME_HEIGHT - 20),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2, cv2.LINE_AA)
                cv2.putText(frame, f"X: {index_tip.x:.3f}, Y: {index_tip.y:.3f}", (10, FRAME_HEIGHT - 50),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2, cv2.LINE_AA)
        
        # Draw center crosshairs for face tracking
        cv2.line(frame, (FRAME_CENTER_X - 10, FRAME_CENTER_Y), (FRAME_CENTER_X + 10, FRAME_CENTER_Y), (0, 0, 255), 1)
        cv2.line(frame, (FRAME_CENTER_X, FRAME_CENTER_Y - 10), (FRAME_CENTER_X, FRAME_CENTER_Y + 10), (0, 0, 255), 1)
        
        # Debug information display
        if DEBUG:
            # Face tracking status
            face_status = "TRACKING" if face_info['target_found'] else "SEARCHING"
            cv2.putText(frame, f"Face: {face_status}", (10, 30), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0) if face_info['target_found'] else (0, 0, 255), 2, cv2.LINE_AA)
            
            # Hand detection status
            hands_count = 1 if gesture_info['hand_detected'] else 0
            cv2.putText(frame, f"Hands: {hands_count}/1", (10, 60), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2, cv2.LINE_AA)
            
            # Gesture text
            cv2.putText(frame, f"Gesture: {gesture_info['gesture_text']}", (10, 90), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2, cv2.LINE_AA)

            if gesture_info['hand_detected']:
                hand_text = "Right Hand" if gesture_info['is_right_hand'] else "Left Hand"
                cv2.putText(frame, hand_text, (10, 120), 
                           cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 0, 0), 2, cv2.LINE_AA)

            # Gesture threshold line
            cv2.line(frame, (0, gesture_threshold), (frame.shape[1], gesture_threshold), (255, 255, 0), 1)
            
            # Servo angles (if available)
            if PCA9685_AVAILABLE:
                cv2.putText(frame, f"Pan: {current_pan_angle:.1f}° Tilt: {current_tilt_angle:.1f}°", 
                           (10, frame.shape[0] - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1, cv2.LINE_AA)
            
            # Zoom mode status
            if ZOOM_CONFIG["active"]:
                mode_text = "ZOOM MODE ACTIVE"
                mode_color = (0, 0, 255)
                cv2.putText(frame, mode_text, (FRAME_WIDTH - 300, 50), 
                            cv2.FONT_HERSHEY_SIMPLEX, 0.8, mode_color, 2, cv2.LINE_AA)

        cv2.imshow('Threaded Face Tracking + Gesture Recognition', frame)
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
            running = False
            break

except KeyboardInterrupt:
    print("\nExiting program.")
finally:
    running = False
    # Wait for threads to finish
    if face_thread.is_alive():
        face_thread.join(timeout=1.0)
    if gesture_thread.is_alive():
        gesture_thread.join(timeout=1.0)
    cleanup_handler(None, None)d
