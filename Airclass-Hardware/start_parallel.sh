#!/bin/bash

# Change to working directory
cd /home/ceng14/Desktop

# Log file
LOG_FILE="/home/ceng14/Desktop/airboot.log"
echo "$(date): Starting Airclass parallel execution" >> $LOG_FILE

/usr/bin/python3 picamhumanrec.py

./server >> $LOG_FILE 2>&1 &
SERVER_PID=$!

/usr/bin/python3 use_model.py >> $LOG_FILE 2>&1 &
MODEL_PID=$!

./hardware_client >> $LOG_FILE 2>&1 &
CLIENT_PID=$!

# Log the PIDs
echo "$(date): Started processes - Servo: $SERVO_PID, Server: $SERVER_PID, Model: $MODEL_PID, Client: $CLIENT_PID" >> $LOG_FILE

# Create PID file for systemd to track
echo $SERVO_PID > /tmp/airclass.pid

# Optional: Wait for all processes to finish (remove if you want them to run indefinitely)
# wait $SERVO_PID $SERVER_PID $MODEL_PID $CLIENT_PID

echo "$(date): All processes started successfully" >> $LOG_FILE
