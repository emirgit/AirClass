import asyncio
import websockets
import json
import datetime
import argparse
import sys
import requests
import time

async def test_single_gesture(websocket, gesture_data):
    """Test a single gesture and wait for feedback"""
    print(f"\nTesting gesture: {gesture_data['gesture_type']}")
    print("Action: ", end='', flush=True)
    
    if gesture_data['gesture_type'] == "NEXT_SLIDE":
        print("Should move to next slide")
    elif gesture_data['gesture_type'] == "PREV_SLIDE":
        print("Should move to previous slide")
    elif gesture_data['gesture_type'] == "ZOOM_IN":
        print("Should zoom in the presentation")
    elif gesture_data['gesture_type'] == "ZOOM_OUT":
        print("Should zoom out the presentation")
    elif gesture_data['gesture_type'] == "THUMB_UP":
        print("Should approve speak request")
    elif gesture_data['gesture_type'] == "THUMB_DOWN":
        print("Should reject speak request")
    elif gesture_data['gesture_type'] == "WAVE":
        print("Should play attention sound")
    elif gesture_data['gesture_type'] == "HAND_RAISE":
        print("Should create speak request")
    
    await websocket.send(json.dumps(gesture_data))
    
    try:
        response = await asyncio.wait_for(websocket.recv(), timeout=2.0)
        print(f"Server response: {response}")
    except asyncio.TimeoutError:
        print("No response received (timeout)")
    except Exception as e:
        print(f"Error receiving response: {str(e)}")
    
    # Wait for visual confirmation
    print("Please check if the gesture had the expected effect...")
    await asyncio.sleep(3)  # Give time to observe the effect

async def login_and_get_session(server_url, email="test@example.com", password="123456"):
    """Login to the REST API and get active session"""
    try:
        # Login
        login_url = f"http://{server_url}/api/v1/auth/login"
        print(f"Attempting to login at: {login_url}")
        login_response = requests.post(login_url, 
            json={"email": email, "password": password})
        
        if not login_response.ok:
            print("Login failed:", login_response.json().get("message", "Unknown error"))
            return None, None, None
        
        login_data = login_response.json()["data"]
        token = login_data["token"]
        
        # Get active session
        headers = {"Authorization": f"Bearer {token}"}
        session_url = f"http://{server_url}/api/v1/session/active"
        print(f"Getting active session from: {session_url}")
        session_response = requests.get(session_url, headers=headers)
        
        if not session_response.ok:
            print("Failed to get active session:", session_response.json().get("message", "Unknown error"))
            return None, None, None
            
        session_data = session_response.json()["data"]
        if not session_data:
            print("No active session found. Please create and activate a session first.")
            return None, None, None
            
        return token, session_data["id"], session_data["code"]
        
    except Exception as e:
        print(f"Error during authentication: {str(e)}")
        return None, None, None

async def test_gesture_client(api_url="localhost:5000", ws_url="ws://localhost:5000/ws"):
    try:
        # First authenticate and get session info
        print("Authenticating with REST API...")
        token, session_id, session_code = await login_and_get_session(api_url)
        
        if not all([token, session_id, session_code]):
            print("Failed to get required session information")
            return
            
        print(f"Successfully authenticated. Session ID: {session_id}, Code: {session_code}")
        
        # Connect to WebSocket server
        print(f"Connecting to WebSocket server at {ws_url}...")
        try:
            async with websockets.connect(ws_url, ping_interval=None) as websocket:
                print("Successfully connected to WebSocket server")

                # Send identification message
                identify_message = {
                    "type": "identify",
                    "role": "mobile",
                    "token": token,
                    "sessionId": session_id,
                    "sessionCode": session_code,
                    "clientId": "test-mobile-1"
                }
                
                print("Sending identification message...")
                await websocket.send(json.dumps(identify_message))
                response = await websocket.recv()
                print(f"Identification response: {response}")

                # Test scenarios
                test_scenarios = [
                    # Navigation gestures
                    {
                        "type": "gesture",
                        "gesture_type": "NEXT_SLIDE",
                        "session_id": session_id,
                        "client_id": "test-mobile-1",
                        "timestamp": datetime.datetime.now().isoformat()
                    },
                    {
                        "type": "gesture",
                        "gesture_type": "PREV_SLIDE",
                        "session_id": session_id,
                        "client_id": "test-mobile-1",
                        "timestamp": datetime.datetime.now().isoformat()
                    },
                    # Zoom gestures
                    {
                        "type": "gesture",
                        "gesture_type": "ZOOM_IN",
                        "session_id": session_id,
                        "client_id": "test-mobile-1",
                        "timestamp": datetime.datetime.now().isoformat()
                    },
                    {
                        "type": "gesture",
                        "gesture_type": "ZOOM_OUT",
                        "session_id": session_id,
                        "client_id": "test-mobile-1",
                        "timestamp": datetime.datetime.now().isoformat()
                    },
                    # Interaction gestures
                    {
                        "type": "gesture",
                        "gesture_type": "THUMB_UP",
                        "session_id": session_id,
                        "client_id": "test-mobile-1",
                        "timestamp": datetime.datetime.now().isoformat()
                    },
                    {
                        "type": "gesture",
                        "gesture_type": "THUMB_DOWN",
                        "session_id": session_id,
                        "client_id": "test-mobile-1",
                        "timestamp": datetime.datetime.now().isoformat()
                    },
                    {
                        "type": "gesture",
                        "gesture_type": "WAVE",
                        "session_id": session_id,
                        "client_id": "test-mobile-1",
                        "timestamp": datetime.datetime.now().isoformat()
                    },
                    {
                        "type": "gesture",
                        "gesture_type": "HAND_RAISE",
                        "session_id": session_id,
                        "client_id": "test-mobile-1",
                        "timestamp": datetime.datetime.now().isoformat()
                    }
                ]

                print("\nStarting gesture tests...")
                print("Please make sure a presentation is open in the Qt application.")
                print("Press Enter to start testing gestures...")
                input()

                for gesture in test_scenarios:
                    await test_single_gesture(websocket, gesture)
                    
                    # Ask for confirmation to continue
                    print("\nPress Enter to test next gesture (or 'q' to quit)...")
                    if input().lower() == 'q':
                        break

                print("\nGesture testing completed!")
                
        except ConnectionRefusedError:
            print("\nConnection refused. Please make sure the WebSocket server is running.")
            print("To start the server, run: python kea_server.py")
            sys.exit(1)
        except Exception as e:
            print(f"\nWebSocket connection error: {str(e)}")
            print("Make sure the WebSocket server is running on the correct port.")
            sys.exit(1)

    except Exception as e:
        print(f"Error: {str(e)}")
        print("Make sure both the REST API and WebSocket server are running.")
        sys.exit(1)

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
        # First register as a hardware client
        register_message = {
            "type": "register",
            "client_type": "HARDWARE",
            "name": "Test Hardware Client"
        }
        await websocket.send(json.dumps(register_message))
        print("Sent registration message")
        
        # Wait for registration confirmation
        response = await websocket.recv()
        print(f"Received: {response}")
        
        # Join a room
        join_message = {
            "type": "identify",
            "roomId": "test-room-123"
        }
        await websocket.send(json.dumps(join_message))
        print("Sent room join message")
        
        # Wait for room join confirmation
        response = await websocket.recv()
        print(f"Received: {response}")
        
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
            
            # Wait for any responses
            try:
                response = await asyncio.wait_for(websocket.recv(), timeout=0.5)
                print(f"Received response: {response}")
            except asyncio.TimeoutError:
                pass

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Test gesture client for AirClass')
    parser.add_argument('--api-url', default='localhost:5000', help='REST API URL')
    parser.add_argument('--ws-url', default='ws://localhost:5000/ws', help='WebSocket server URL')
    parser.add_argument('--email', default='test@example.com', help='Login email')
    parser.add_argument('--password', default='123456', help='Login password')
    
    args = parser.parse_args()
    
    print(f"Starting gesture test client...")
    print(f"REST API URL: {args.api_url}")
    print(f"WebSocket URL: {args.ws_url}")
    print(f"Email: {args.email}")
    
    try:
        asyncio.run(test_gesture_client(args.api_url, args.ws_url))
    except KeyboardInterrupt:
        print("\nTest client stopped by user.")
    except Exception as e:
        print(f"\nError: {str(e)}")
        sys.exit(1) 