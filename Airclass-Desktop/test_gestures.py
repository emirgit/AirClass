import asyncio
import websockets
import json
import datetime

async def send_gesture(websocket, gesture_type):
    message = {
        "type": "gesture",
        "gesture_type": gesture_type,
        "client_id": "test-client",
        "timestamp": datetime.datetime.now().isoformat()
    }
    await websocket.send(json.dumps(message))
    print(f"Sent gesture: {gesture_type}")

async def test_gestures():
    uri = "ws://localhost:8082"
    async with websockets.connect(uri) as websocket:
        # Test all gesture types
        gestures = [
            "HAND_RAISE",
            "THUMB_UP",
            "THUMB_DOWN",
            "WAVE",
            "ZOOM_IN",
            "ZOOM_OUT"
        ]
        
        for gesture in gestures:
            await send_gesture(websocket, gesture)
            await asyncio.sleep(1)  # Wait 1 second between gestures

if __name__ == "__main__":
    asyncio.run(test_gestures())