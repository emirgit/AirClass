from adafruit_servokit import ServoKit
import time

# Create ServoKit instance for 16-channel board
kit = ServoKit(channels=16)

# Configuration: define channels
PAN_CHANNEL = 0
TILT_CHANNEL = 1

# Default safe angle limits
MIN_ANGLE = 0
MAX_ANGLE = 180
STEP = 5
DELAY = 0.25

LAST_POSITION_PAN = 120
LAST_POSITION_TILT = 95

def sweep_servo(channel, label, last):
    print(f"\nSweeping {label} servo on channel {channel}...")
    servo = kit.servo[channel]
    """
    for angle in range(MIN_ANGLE, MAX_ANGLE + 1, STEP):
        servo.angle = angle
        time.sleep(DELAY)
    
    for angle in range(MAX_ANGLE, MIN_ANGLE - 1, -STEP):
        servo.angle = angle
        time.sleep(DELAY)
    """
    servo.angle = last - 20
    time.sleep(DELAY)    
    
    servo.angle = last
    print(f"Finished sweeping {label}.")

def manual_positioning(channel, label):
    servo = kit.servo[channel]
    print(f"\nManual control for {label} (channel {channel})")
    print("Enter angles (0-180), or 'q' to quit manual mode.")
    
    while True:
        user_input = input(f"{label} angle: ")
        if user_input.strip().lower() == 'q':
            break
        try:
            angle = int(user_input)
            if 0 <= angle <= 180:
                servo.angle = angle
            else:
                print("Please enter an angle between 0 and 180.")
        except ValueError:
            print("Invalid input. Enter a number or 'q' to quit.")

try:
    print("Starting servo configuration test...")
    #sweep_servo(PAN_CHANNEL, "Pan", LAST_POSITION_PAN)
    time.sleep(DELAY)
    #sweep_servo(TILT_CHANNEL, "Tilt", LAST_POSITION_TILT)
    
    manual_positioning(PAN_CHANNEL, "Pan")
    manual_positioning(TILT_CHANNEL, "Tilt")
    
    print("\nTest complete. Adjust limits and initial values in your main script accordingly.")

except KeyboardInterrupt:
    print("\nInterrupted by user. Exiting.")
