import asyncio
import websockets
import json
import logging
import sys
import socket
from enum import Enum
from typing import Dict, Set, Any
import qrcode
import base64
from io import BytesIO
from datetime import datetime

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Function to find a free port
def find_free_port(start_port=8080, max_attempts=10):
    """Find a free port starting from start_port"""
    for port in range(start_port, start_port + max_attempts):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.bind(('', port))
                return port
        except OSError:
            continue
    return None

# Client type enum
class ClientType(Enum):
    HARDWARE = "HARDWARE"
    DESKTOP = "DESKTOP"
    MOBILE = "MOBILE"
    UNKNOWN = "UNKNOWN"

# Gesture types enum
class GestureType(Enum):
    # Required gestures
    THREE_GUN = "three_gun"          # Next slide
    INV_THREE_GUN = "inv_three_gun"  # Previous slide
    ONE = "one"                      # Drawing mode
    TWO_UP = "two_up"               # Point mode
    LIKE = "like"                   # Accept
    DISLIKE = "dislike"             # Decline
    TAKE_PICTURE = "take_picture"   # Attendance
    PALM = "palm"                   # Attention
    TIMEOUT = "timeout"             # Break time
    
    # Extra/Custom gestures
    CALL = "call"
    OK = "ok"
    HEART = "heart"
    HEART2 = "heart2"
    MID_FINGER = "mid_finger"
    FOUR = "four"
    ROCK = "rock"
    THUMB_INDEX = "thumb_index"
    HOLY = "holy"
    THREE = "three"
    THREE2 = "three2"
    THREE3 = "three3"
    
    UNKNOWN = "UNKNOWN"

# Session status enum
class SessionStatus(Enum):
    ACTIVE = 1
    INACTIVE = 0

# Client info class
class ClientInfo:
    def __init__(self, websocket):
        self.websocket = websocket
        self.type = ClientType.UNKNOWN
        self.id = ""
        self.name = ""
        self.email = ""
        self.current_gesture = GestureType.UNKNOWN
        self.last_gesture_time = None
        self.current_session_id = None

# Session class to manage user sessions
class Session:
    def __init__(self, session_id, name, owner_id):
        self.session_id = session_id
        self.name = name
        self.owner_id = owner_id
        self.status = SessionStatus.ACTIVE
        self.created_at = datetime.now().isoformat()
        self.participants = set()  # Set of client IDs

    def add_participant(self, client_id):
        self.participants.add(client_id)

    def remove_participant(self, client_id):
        self.participants.discard(client_id)
        
    def to_dict(self):
        return {
            "session_id": self.session_id,
            "name": self.name,
            "owner_id": self.owner_id,
            "status": self.status.value,
            "created_at": self.created_at,
            "participants": list(self.participants)
        }

# Classroom class to manage rooms
class Classroom:
    def __init__(self, room_id):
        self.room_id = room_id
        self.clients = set()  # Set of clients in the room
        self.gesture_history = []  # List to store gesture events

    def add_client(self, client):
        self.clients.add(client)

    def remove_client(self, client):
        self.clients.discard(client)

    def is_empty(self):
        return len(self.clients) == 0

    def get_client_ids(self):
        return [client.id for client in self.clients]

    def add_gesture_event(self, client_id, gesture_type, timestamp):
        self.gesture_history.append({
            "client_id": client_id,
            "gesture": gesture_type,
            "timestamp": timestamp
        })
        logging.info(f"Gesture recorded in room {self.room_id}: {client_id} performed {gesture_type}")

# User database for authentication (mock implementation)
class UserDatabase:
    def __init__(self):
        # Format: {email: {"password": password, "name": name}}
        self.users = {}
        
        # Add a test user
        self.users["test@example.com"] = {
            "password": "123456",
            "name": "Test User",
            "user_id": "user-1"
        }
        
    def register_user(self, name, email, password):
        if email in self.users:
            return False, "User with this email already exists"
        
        user_id = f"user-{len(self.users) + 1}"
        self.users[email] = {
            "password": password,
            "name": name,
            "user_id": user_id
        }
        logging.info(f"Registered new user: {name}, {email}, ID: {user_id}")
        return True, "Registration successful", user_id
    
    def authenticate_user(self, name, password):
        # For this mock implementation, check if any user has this name and password
        for email, user_data in self.users.items():
            if user_data["name"] == name and user_data["password"] == password:
                return True, "Authentication successful", user_data["user_id"]
        
        # For testing, allow any login
        return True, "Mock authentication successful", f"user-{len(self.users) + 1}"

# Session database for session management
class SessionDatabase:
    def __init__(self):
        self.sessions = {}  # session_id -> Session
        self.user_sessions = {}  # user_id -> [session_id]
        
    def create_session(self, name, owner_id):
        session_id = f"session-{len(self.sessions) + 1}-{int(datetime.now().timestamp())}"
        new_session = Session(session_id, name, owner_id)
        self.sessions[session_id] = new_session
        
        # Associate this session with the user
        if owner_id not in self.user_sessions:
            self.user_sessions[owner_id] = []
        self.user_sessions[owner_id].append(session_id)
        
        logging.info(f"Created new session: {name}, ID: {session_id}, Owner: {owner_id}")
        return session_id, new_session
    
    def get_active_session_for_user(self, user_id):
        if user_id not in self.user_sessions:
            return None
        
        # Get session IDs for this user
        session_ids = self.user_sessions[user_id]
        
        # Find the first active session
        for session_id in session_ids:
            if session_id in self.sessions and self.sessions[session_id].status == SessionStatus.ACTIVE:
                return self.sessions[session_id]
                
        return None
    
    def get_session(self, session_id):
        return self.sessions.get(session_id)
    
    def deactivate_session(self, session_id):
        if session_id in self.sessions:
            self.sessions[session_id].status = SessionStatus.INACTIVE
            logging.info(f"Deactivated session: {session_id}")
            return True
        return False

# WebSocket server class
class ClassroomWebSocketServer:
    def __init__(self):
        self.connections = {}  # Map of websocket -> ClientInfo
        self.classrooms = {}   # Map of room_id -> Classroom
        self.next_id = 1
        self.user_db = UserDatabase()  # User database for authentication
        self.session_db = SessionDatabase()  # Session database for session management

    async def register(self, websocket):
        """Register a new client connection"""
        logging.info("New connection opened")
        self.connections[websocket] = ClientInfo(websocket)

        import time
        # Send all gesture types as test messages
        test_gestures = [
            #"HAND_RAISE",
            #"THUMB_UP",
            #"THUMB_DOWN",
            #"WAVE",
            #"ZOOM_IN",
            #"ZOOM_OUT",
            #"three_gun",
            #"inv_three_gun",
            #"one",
            #"two_up",
            #"like",
            #"dislike",
            #"take_picture",
            #"palm",
            "timeout",
            #"call",
            #"ok",
            #"zoom_in",
            "down",
            "up",
            "right",
            "left",

        ]
        
        for gesture in test_gestures:
            gesture_message = {
                "type": "gesture",
                "gesture_type": gesture,
                "client_id": "test_client",
                "timestamp": time.strftime("%Y-%m-%d %H:%M:%S")
            }
            await websocket.send(json.dumps(gesture_message))
            await asyncio.sleep(1)  # 1 saniye bekle

    async def unregister(self, websocket):
        """Unregister a client connection"""
        logging.info("Connection closed")
        if websocket in self.connections:
            client = self.connections[websocket]
            # Remove client from any associated classroom
            for room_id, classroom in list(self.classrooms.items()):
                classroom.remove_client(client)
                if classroom.is_empty():
                    del self.classrooms[room_id]
                    logging.info(f"Room {room_id} deleted (empty)")
            del self.connections[websocket]

    def client_type_to_string(self, client_type):
        """Convert client type enum to string"""
        return client_type.value

    async def handle_login(self, websocket, data):
        """Handle login requests"""
        logging.info(f"Login attempt: {data}")
        
        client = self.connections[websocket]
        name = data.get("name", "")
        password = data.get("password", "")
        
        if not name or not password:
            await websocket.send(json.dumps({
                "type": "login_response",
                "success": False,
                "message": "Missing name or password"
            }))
            return
        
        # Authenticate user
        success, message, user_id = self.user_db.authenticate_user(name, password)
        
        if success:
            client.name = name
            client.type = ClientType.DESKTOP
            client.id = user_id
            
            logging.info(f"User logged in: {name} with ID {client.id}")
            
            # Check if user has an active session
            active_session = self.session_db.get_active_session_for_user(user_id)
            
            response = {
                "type": "login_response",
                "success": success,
                "message": message,
                "client_id": client.id
            }
            
            # Include session info if available
            if active_session:
                client.current_session_id = active_session.session_id
                response["session_id"] = active_session.session_id
                response["session_name"] = active_session.name
                logging.info(f"User {client.id} has active session: {active_session.session_id}")
            else:
                logging.info(f"User {client.id} has no active session")
        else:
            response = {
                "type": "login_response",
                "success": success,
                "message": message
            }
        
        logging.info(f"Login response: {response}")
        await websocket.send(json.dumps(response))

    async def handle_registration(self, websocket, data):
        """Handle registration requests"""
        logging.info(f"Registration attempt: {data}")
        
        client = self.connections[websocket]
        name = data.get("name", "")
        email = data.get("email", "")
        password = data.get("password", "")
        
        if not name or not email or not password:
            await websocket.send(json.dumps({
                "type": "register_response",
                "success": False,
                "message": "Missing required fields"
            }))
            return
        
        # Register user
        success, message, user_id = self.user_db.register_user(name, email, password)
        
        response = {
            "type": "register_response",
            "success": success,
            "message": message
        }
        
        if success:
            response["user_id"] = user_id
        
        logging.info(f"Registration response: {response}")
        await websocket.send(json.dumps(response))

    async def handle_create_session(self, websocket, data):
        """Handle session creation requests"""
        logging.info(f"Create session request: {data}")
        
        client = self.connections[websocket]
        
        # Only authenticated users can create sessions
        if client.id == "":
            await websocket.send(json.dumps({
                "type": "session_response",
                "success": False,
                "message": "Authentication required to create a session"
            }))
            return
        
        name = data.get("name", "")
        user_id = client.id
        
        if not name:
            await websocket.send(json.dumps({
                "type": "session_response",
                "success": False,
                "message": "Session name is required"
            }))
            return
        
        # Create the session
        session_id, session = self.session_db.create_session(name, user_id)
        client.current_session_id = session_id
        
        response = {
            "type": "session_response",
            "success": True,
            "message": "Session created successfully",
            "session_id": session_id,
            "session_name": name
        }
        
        logging.info(f"Session creation response: {response}")
        await websocket.send(json.dumps(response))

    async def register_client(self, websocket, data):
        """Handle client registration"""
        client = self.connections[websocket]
        if data["register"] == "hardware":
            client.type = ClientType.HARDWARE
            client.id = data.get("id", f"hardware-{self.next_id}")
        elif data["register"] == "desktop":
            client.type = ClientType.DESKTOP
            client.id = data.get("id", f"desktop-{self.next_id}")
        elif data["register"] == "mobile":
            client.type = ClientType.MOBILE
            client.id = data.get("id", f"mobile-{self.next_id}")
        self.next_id += 1

        logging.info(f"Client registered as {self.client_type_to_string(client.type)} with ID: {client.id}")
        confirmation = {
            "type": "registration_success",
            "client_type": self.client_type_to_string(client.type),
            "client_id": client.id
        }
        await websocket.send(json.dumps(confirmation))

    async def handle_gesture(self, websocket, data):
        """Handle incoming gesture data"""
        client = self.connections[websocket]
        try:
            gesture_type = data.get("gesture_type")
            timestamp = data.get("timestamp")

            if not gesture_type or not timestamp:
                raise ValueError("Missing gesture_type or timestamp")

            # Update client's current gesture
            client.current_gesture = GestureType(gesture_type)
            client.last_gesture_time = timestamp

            # Find the classroom this client belongs to
            for room_id, classroom in self.classrooms.items():
                if client in classroom.clients:
                    classroom.add_gesture_event(client.id, gesture_type, timestamp)

                    # Broadcast gesture to all clients in the room
                    gesture_message = {
                        "type": "gesture",
                        "client_id": client.id,
                        "gesture_type": gesture_type,
                        "timestamp": timestamp
                    }

                    print(f"Broadcasting gesture {gesture_type} from {client.id} to room {room_id}")
                    logging.info(f"Broadcasting gesture {gesture_type} from {client.id} to room {room_id}")
                    
                    for room_client in classroom.clients:
                        if room_client.websocket != websocket:  # Don't send back to sender
                            await room_client.websocket.send(json.dumps(gesture_message))

                    # Send gesture data to Qt application
                    qt_message = {
                        "type": "qt_gesture",
                        "client_id": client.id,
                        "gesture_type": gesture_type,
                        "timestamp": timestamp
                    }
                    for room_client in classroom.clients:
                        if room_client.type == ClientType.DESKTOP:  # Assuming Qt app is a desktop client
                            await room_client.websocket.send(json.dumps(qt_message))

                    logging.info(f"Gesture {gesture_type} from {client.id} processed and broadcasted")
                    break

        except ValueError as e:
            logging.error(f"Invalid gesture data: {str(e)}")
            error = {"type": "error", "message": "Invalid gesture data format"}
            await websocket.send(json.dumps(error))

    async def handle_page_navigation(self, websocket, data):
        """Handle page navigation commands"""
        client = self.connections[websocket]
        try:
            action = data.get("action")
            timestamp = data.get("timestamp")
            
            if not action or not timestamp:
                raise ValueError("Missing action or timestamp")

            # Find the classroom this client belongs to
            for room_id, classroom in self.classrooms.items():
                if client in classroom.clients:
                    # Broadcast navigation command to all clients in the room
                    navigation_message = {
                        "type": "page_navigation",
                        "client_id": client.id,
                        "action": action,
                        "timestamp": timestamp
                    }
                    
                    for room_client in classroom.clients:
                        if room_client.websocket != websocket:  # Don't send back to sender
                            await room_client.websocket.send(json.dumps(navigation_message))
                    
                    logging.info(f"Page navigation {action} from {client.id} processed and broadcasted")
                    break

        except ValueError as e:
            logging.error(f"Invalid navigation data: {str(e)}")
            error = {"type": "error", "message": "Invalid navigation data format"}
            await websocket.send(json.dumps(error))

    async def handle_message(self, websocket, message):
        """Process incoming message"""
        try:
            client = self.connections[websocket]
            data = json.loads(message)
            logging.info(f"Received message: {data}")

            if "type" not in data:
                if client.type == ClientType.UNKNOWN and "register" in data:
                    await self.register_client(websocket, data)
                    return
                else:
                    error = {"type": "error", "message": "Missing message type"}
                    await websocket.send(json.dumps(error))
                    return

            msg_type = data["type"]
            logging.info(f"Processing message type: {msg_type}")

            # Handle authentication/registration requests
            if msg_type == "login":
                await self.handle_login(websocket, data)
                return
            elif msg_type == "register":
                await self.handle_registration(websocket, data)
                return
            elif msg_type == "create_session":
                await self.handle_create_session(websocket, data)
                return
            elif msg_type == "gesture":
                await self.handle_gesture(websocket, data)
                return

            # For other message types, client should be authenticated or identified
            if client.type == ClientType.UNKNOWN:
                error = {"type": "error", "message": "Client not authenticated"}
                await websocket.send(json.dumps(error))
                return

            logging.info(f"Handling message type: {msg_type} from {self.client_type_to_string(client.type)} ID: {client.id}")

            if msg_type == "identify" and "roomId" in data:
                room_id = data["roomId"]
                if room_id not in self.classrooms:
                    self.classrooms[room_id] = Classroom(room_id)
                classroom = self.classrooms[room_id]
                classroom.add_client(client)
                logging.info(f"Client {client.id} joined room {room_id}")
                
                qr = qrcode.QRCode(version=1, box_size=10, border=5)
                qr.add_data(room_id)
                qr.make(fit=True)
                img = qr.make_image(fill_color="black", back_color="white")
                
                # If session is available, include it in the QR data
                if client.current_session_id:
                    session = self.session_db.get_session(client.current_session_id)
                    if session:
                        qr_data = {
                            "room_id": room_id,
                            "session_id": client.current_session_id,
                            "session_name": session.name
                        }
                        qr = qrcode.QRCode(version=1, box_size=10, border=5)
                        qr.add_data(json.dumps(qr_data))
                        qr.make(fit=True)
                        img = qr.make_image(fill_color="black", back_color="white")

                img.save(f"room_{room_id}.png")

                logging.info(f"QR code for room {room_id} generated and saved as room_{room_id}.png")

                buffered = BytesIO()
                img.save(buffered, format="PNG")
                base64_image = base64.b64encode(buffered.getvalue()).decode("utf-8")

                confirmation = {
                    "type": "room_joined",
                    "room_id": room_id,
                    "client_id": client.id,
                    "qr_code": base64_image
                }
                
                # Include session info if available
                if client.current_session_id:
                    session = self.session_db.get_session(client.current_session_id)
                    if session:
                        confirmation["session_id"] = client.current_session_id
                        confirmation["session_name"] = session.name
                
                await websocket.send(json.dumps(confirmation))
            elif msg_type == "gesture":
                await self.handle_gesture(websocket, data)
            elif msg_type == "page_navigation":
                await self.handle_page_navigation(websocket, data)
            else:
                logging.warning(f"Unhandled message type: {msg_type}")
                
        except json.JSONDecodeError:
            logging.error("Error decoding JSON message")
            error = {"type": "error", "message": "Invalid message format"}
            await websocket.send(json.dumps(error))
        except Exception as e:
            logging.error(f"Error processing message: {str(e)}")
            error = {"type": "error", "message": f"Error processing message: {str(e)}"}
            await websocket.send(json.dumps(error))

    async def handle_connection(self, websocket):
        """Handle a WebSocket connection"""
        await self.register(websocket)
        try:
            async for message in websocket:
                await self.handle_message(websocket, message)
        except websockets.ConnectionClosed:
            logging.info("Connection closed")
        finally:
            await self.unregister(websocket)

    async def run(self, host="0.0.0.0", port=8080):
        """Run the WebSocket server"""
        try:
            logging.info(f"Starting server on {host}:{port}")
            # send basic message to websocket client
                        
            
            async with websockets.serve(self.handle_connection, host, port):
                logging.info(f"Server started successfully on {host}:{port}")
                await asyncio.Future()  # Run forever
        except OSError as e:
            logging.error(f"Failed to start server on port {port}: {e}")
            if "address already in use" in str(e).lower() or "10048" in str(e):
                logging.info("Trying to find an available port...")
                new_port = find_free_port(port + 1)
                logging.info(f"Retrying with port {new_port}")
                await self.run(host, new_port)
            else:
                # If it's a different error, re-raise it
                raise

if __name__ == "__main__":
    # Get port from command line if provided
    port = 8082  # Changed to match the client port
    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except ValueError:
            print(f"Invalid port: {sys.argv[1]}, using default: {port}")
    
    try:
        # Check if the port is available before starting the server
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.bind(('0.0.0.0', port))
        except OSError:
            logging.warning(f"Port {port} is already in use")
            port = find_free_port(port + 1)
            logging.info(f"Using alternative port: {port}")
        
        server = ClassroomWebSocketServer()
        asyncio.run(server.run(port=port))
    except KeyboardInterrupt:
        logging.info("Server stopped by user")
    except Exception as e:
        logging.error(f"Error: {e}")
        sys.exit(1)