import asyncio
import websockets
import json
import random
import time

# Constants
MAX_RECONNECT_ATTEMPTS = 50
RECONNECT_INTERVAL = 5  # seconds
PING_INTERVAL = 10  # seconds
SERVER_URL = "ws://localhost:8082"  # Replace with your WebSocket server URL

# WebSocket Client
class WebSocketClient:
    def __init__(self, url):
        self.url = url
        self.connected = False
        self.reconnect_attempts = 0

    async def connect(self):
        while not self.connected and self.reconnect_attempts < MAX_RECONNECT_ATTEMPTS:
            try:
                print(f"Connecting to {self.url}...")
                self.websocket = await websockets.connect(self.url)
                self.connected = True
                print("Connected to WebSocket server.")
                await self.send_ping()  # Start pinging after connecting
                await self.listen_for_messages()  # Start listening for messages
            except Exception as e:
                print(f"Error connecting: {e}")
                self.reconnect_attempts += 1
                print(f"Reconnect attempt {self.reconnect_attempts} of {MAX_RECONNECT_ATTEMPTS}")
                await asyncio.sleep(RECONNECT_INTERVAL)
        if not self.connected:
            print("Max reconnect attempts reached. Stopping connection.")

    async def send_message(self, message):
        if self.connected:
            print(f"Sending message: {message}")
            await self.websocket.send(message)
        else:
            print("Not connected to server.")

    async def send_gesture(self):
        gesture_types = ["swipe_left", "swipe_right", "tap", "scroll_up", "scroll_down"]
        gesture_type = random.choice(gesture_types)
        client_id = f"client_{random.randint(1, 100)}"
        timestamp = time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime())
        
        message = {
            "type": "gesture",
            "gesture_type": gesture_type,
            "client_id": client_id,
            "timestamp": timestamp
        }
        
        await self.send_message(json.dumps(message))

    async def send_ping(self):
        while self.connected:
            await asyncio.sleep(PING_INTERVAL)
            if self.connected:
                print("Sending ping to keep connection alive.")
                await self.send_message(json.dumps({"type": "ping"}))

    async def listen_for_messages(self):
        while self.connected:
            try:
                message = await self.websocket.recv()
                print(f"Message received: {message}")
                await self.process_message(message)
            except Exception as e:
                print(f"Error receiving message: {e}")
                break

    async def process_message(self, message):
        try:
            data = json.loads(message)
            msg_type = data.get("type")

            if msg_type == "gesture":
                gesture_type = data.get("gesture_type")
                client_id = data.get("client_id")
                timestamp = data.get("timestamp")
                print(f"Gesture received: {gesture_type} from {client_id} at {timestamp}")
            elif msg_type == "page_navigation":
                action = data.get("action")
                client_id = data.get("client_id")
                timestamp = data.get("timestamp")
                print(f"Page navigation received: {action} from {client_id} at {timestamp}")
            else:
                print(f"Unknown message type: {msg_type}")
        except json.JSONDecodeError:
            print("Invalid JSON received.")

    async def close(self):
        if self.connected:
            print("Closing connection.")
            self.connected = False
            await self.websocket.close()

# also listen message from server
    async def listen_for_server_messages(self):
        while self.connected:
            try:
                message = await self.websocket.recv()
                print(f"Server message received: {message}")
                await self.process_message(message)
            except websockets.ConnectionClosed:
                print("Connection closed by server.")
                self.connected = False
            except Exception as e:
                print(f"Error receiving server message: {e}")
                break

# Running the WebSocket client
async def main():
    client = WebSocketClient(SERVER_URL)
    await client.connect()
    await asyncio.sleep(1)  # Allow some time for connection to establish

    # Start listening for server messages in the background
    asyncio.create_task(client.listen_for_server_messages())
    # Simulate sending initial messages
    await client.send_message(json.dumps({"type": "init", "client_id": "test_client"}))
    await asyncio.sleep(1)  # Allow some time for the initial message to be sent
    # Start sending gestures periodically
    # await client.send_gesture()  # Send an initial gesture
    # await asyncio.sleep(1)  # Allow some time for the gesture to be sent

    # Simulate sending gesture messages periodically
    for _ in range(10):  # Sends 10 gestures
        if client.connected:
            await client.send_gesture()
            await asyncio.sleep(2)  # Wait 2 seconds between sending gestures

    await client.close()

# Start the asyncio loop
if __name__ == "__main__":
    asyncio.run(main())
