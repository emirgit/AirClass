import asyncio
import websockets
import json
import logging
import sys
import socket
from enum import Enum
from typing import Dict, Set, Any, List
import qrcode
import base64
from io import BytesIO
from datetime import datetime, timedelta
import uuid
from aiohttp import web
import aiohttp_cors
import hashlib
import secrets
from collections import deque
import time

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Function to find a free port
def find_free_port(start_port=9090, max_attempts=10):
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
    HAND_RAISE = "HAND_RAISE"
    THUMB_UP = "THUMB_UP"
    THUMB_DOWN = "THUMB_DOWN"
    WAVE = "WAVE"
    ZOOM_IN = "ZOOM_IN"
    ZOOM_OUT = "ZOOM_OUT"
    UNKNOWN = "UNKNOWN"

# Session status enum
class SessionStatus(Enum):
    ACTIVE = 1
    INACTIVE = 0

# Request status enum
class RequestStatus(Enum):
    PENDING = "PENDING"
    APPROVED = "APPROVED"
    REJECTED = "REJECTED"

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
    def __init__(self, room_id, name="", description=""):
        self.room_id = room_id
        self.name = name
        self.description = description
        self.clients = set()  # Set of clients in the room
        self.gesture_history = []  # List to store gesture events
        self.created_at = datetime.now().isoformat()
        self.owner_id = None
        self.attendance_codes = {}  # code -> {expiry, students}
        self.attendance_records = []  # List of attendance records
        self.speak_requests = deque()  # Queue of speak requests
        self.current_speaker = None

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
    
    def add_speak_request(self, student_id, student_name):
        request = {
            "id": str(uuid.uuid4()),
            "student_id": student_id,
            "student_name": student_name,
            "timestamp": datetime.now().isoformat(),
            "status": RequestStatus.PENDING.value
        }
        self.speak_requests.append(request)
        return request
    
    def get_next_speak_request(self):
        while self.speak_requests:
            request = self.speak_requests[0]
            if request["status"] == RequestStatus.PENDING.value:
                return request
            self.speak_requests.popleft()
        return None
    
    def approve_speak_request(self, request_id):
        for request in self.speak_requests:
            if request["id"] == request_id:
                request["status"] = RequestStatus.APPROVED.value
                self.current_speaker = request["student_id"]
                return True
        return False
    
    def reject_speak_request(self, request_id):
        for request in self.speak_requests:
            if request["id"] == request_id:
                request["status"] = RequestStatus.REJECTED.value
                return True
        return False
    
    def to_dict(self):
        return {
            "room_id": self.room_id,
            "name": self.name,
            "description": self.description,
            "created_at": self.created_at,
            "owner_id": self.owner_id,
            "participants": len(self.clients),
            "active_attendance_codes": len([c for c, data in self.attendance_codes.items() 
                                           if data["expiry"] > datetime.now()])
        }

# User database for authentication (mock implementation)
class UserDatabase:
    def __init__(self):
        # Format: {email: {"password": password_hash, "name": name, "user_id": id}}
        self.users = {}
        self.auth_tokens = {}  # token -> user_id
        
        # Add a test user
        self.users["test@example.com"] = {
            "password": self.hash_password("123456"),
            "name": "Test User",
            "user_id": "user-1"
        }
    
    def hash_password(self, password):
        return hashlib.sha256(password.encode()).hexdigest()
    
    def generate_token(self):
        return secrets.token_urlsafe(32)
        
    def register_user(self, name, email, password):
        if email in self.users:
            return False, "User with this email already exists", None
        
        user_id = f"user-{len(self.users) + 1}"
        self.users[email] = {
            "password": self.hash_password(password),
            "name": name,
            "user_id": user_id
        }
        
        token = self.generate_token()
        self.auth_tokens[token] = user_id
        
        logging.info(f"Registered new user: {name}, {email}, ID: {user_id}")
        return True, "Registration successful", {"user_id": user_id, "token": token}
    
    def authenticate_user(self, email, password):
        if email not in self.users:
            # For testing, allow name-based login
            for user_email, user_data in self.users.items():
                if user_data["name"] == email and user_data["password"] == self.hash_password(password):
                    token = self.generate_token()
                    self.auth_tokens[token] = user_data["user_id"]
                    return True, "Authentication successful", {"user_id": user_data["user_id"], "token": token}
            
            # Mock authentication for testing
            return True, "Mock authentication successful", {"user_id": f"user-{len(self.users) + 1}", "token": self.generate_token()}
        
        user_data = self.users[email]
        if user_data["password"] == self.hash_password(password):
            token = self.generate_token()
            self.auth_tokens[token] = user_data["user_id"]
            return True, "Authentication successful", {"user_id": user_data["user_id"], "token": token}
        
        return False, "Invalid credentials", None
    
    def get_user_by_token(self, token):
        user_id = self.auth_tokens.get(token)
        if not user_id:
            return None
        
        for email, user_data in self.users.items():
            if user_data["user_id"] == user_id:
                return {**user_data, "email": email}
        return None

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

# WebSocket and REST API server class
class ClassroomServer:
    def __init__(self):
        self.connections = {}  # Map of websocket -> ClientInfo
        self.classrooms = {}   # Map of room_id -> Classroom
        self.next_id = 1
        self.user_db = UserDatabase()  # User database for authentication
        self.session_db = SessionDatabase()  # Session database for session management
        self.resources = {
            "images": {},  # id -> image_data
            "slides": {}   # id -> slide_data
        }

    # REST API Handlers
    
    # Authentication endpoints
    async def login(self, request):
        try:
            data = await request.json()
            email = data.get("email", data.get("name", ""))  # Support both email and name
            password = data.get("password", "")
            
            success, message, auth_data = self.user_db.authenticate_user(email, password)
            
            if success:
                return web.json_response({
                    "success": True,
                    "message": message,
                    "data": auth_data
                })
            else:
                return web.json_response({
                    "success": False,
                    "message": message
                }, status=401)
        except Exception as e:
            return web.json_response({
                "success": False,
                "message": str(e)
            }, status=400)
    
    async def register(self, request):
        print("Registering user")
        print("Request:", request)
        try:
            data = await request.json()
            name = data.get("name", "")
            email = data.get("email", "")
            password = data.get("password", "")
            
            if not all([name, email, password]):
                return web.json_response({
                    "success": False,
                    "message": "Missing required fields"
                }, status=400)
            
            success, message, auth_data = self.user_db.register_user(name, email, password)
            
            if success:
                return web.json_response({
                    "success": True,
                    "message": message,
                    "data": auth_data
                })
            else:
                return web.json_response({
                    "success": False,
                    "message": message
                }, status=400)
        except Exception as e:
            return web.json_response({
                "success": False,
                "message": str(e)
            }, status=400)
    
    # Classroom endpoints
    async def list_classrooms(self, request):
        classrooms = [room.to_dict() for room in self.classrooms.values()]
        return web.json_response({
            "success": True,
            "data": classrooms
        })
    
    async def create_classroom(self, request):
        try:
            data = await request.json()
            name = data.get("name", "")
            description = data.get("description", "")
            
            # Get user from token
            token = request.headers.get("Authorization", "").replace("Bearer ", "")
            user = self.user_db.get_user_by_token(token)
            if not user:
                return web.json_response({
                    "success": False,
                    "message": "Unauthorized"
                }, status=401)
            
            room_id = f"room-{len(self.classrooms) + 1}-{int(time.time())}"
            classroom = Classroom(room_id, name, description)
            classroom.owner_id = user["user_id"]
            self.classrooms[room_id] = classroom
            
            return web.json_response({
                "success": True,
                "data": classroom.to_dict()
            })
        except Exception as e:
            return web.json_response({
                "success": False,
                "message": str(e)
            }, status=400)
    
    async def update_classroom(self, request):
        try:
            data = await request.json()
            room_id = data.get("room_id", "")
            
            if room_id not in self.classrooms:
                return web.json_response({
                    "success": False,
                    "message": "Classroom not found"
                }, status=404)
            
            classroom = self.classrooms[room_id]
            
            if "name" in data:
                classroom.name = data["name"]
            if "description" in data:
                classroom.description = data["description"]
            
            return web.json_response({
                "success": True,
                "data": classroom.to_dict()
            })
        except Exception as e:
            return web.json_response({
                "success": False,
                "message": str(e)
            }, status=400)
    
    async def delete_classroom(self, request):
        try:
            data = await request.json()
            room_id = data.get("room_id", "")
            
            if room_id not in self.classrooms:
                return web.json_response({
                    "success": False,
                    "message": "Classroom not found"
                }, status=404)
            
            del self.classrooms[room_id]
            
            return web.json_response({
                "success": True,
                "message": "Classroom deleted successfully"
            })
        except Exception as e:
            return web.json_response({
                "success": False,
                "message": str(e)
            }, status=400)
    
    # Attendance endpoints
    async def list_attendance(self, request):
        room_id = request.query.get("room_id", "")
        
        if not room_id:
            return web.json_response({
                "success": False,
                "message": "room_id is required"
            }, status=400)
        
        if room_id not in self.classrooms:
            return web.json_response({
                "success": False,
                "message": "Classroom not found"
            }, status=404)
        
        classroom = self.classrooms[room_id]
        
        return web.json_response({
            "success": True,
            "data": classroom.attendance_records
        })
    
    async def mark_attendance(self, request):
        try:
            data = await request.json()
            room_id = data.get("room_id", "")
            code = data.get("code", "")
            student_id = data.get("student_id", "")
            student_name = data.get("student_name", "")
            
            if not all([room_id, code, student_id, student_name]):
                return web.json_response({
                    "success": False,
                    "message": "Missing required fields"
                }, status=400)
            
            if room_id not in self.classrooms:
                return web.json_response({
                    "success": False,
                    "message": "Classroom not found"
                }, status=404)
            
            classroom = self.classrooms[room_id]
            
            # Check if code is valid
            if code not in classroom.attendance_codes:
                return web.json_response({
                    "success": False,
                    "message": "Invalid attendance code"
                }, status=400)
            
            code_data = classroom.attendance_codes[code]
            if datetime.now() > code_data["expiry"]:
                return web.json_response({
                    "success": False,
                    "message": "Attendance code has expired"
                }, status=400)
            
            # Mark attendance
            attendance_record = {
                "student_id": student_id,
                "student_name": student_name,
                "timestamp": datetime.now().isoformat(),
                "code": code
            }
            
            classroom.attendance_records.append(attendance_record)
            code_data["students"].add(student_id)
            
            # Broadcast attendance update via WebSocket
            await self.broadcast_attendance_update(room_id, attendance_record)
            
            return web.json_response({
                "success": True,
                "message": "Attendance marked successfully",
                "data": attendance_record
            })
        except Exception as e:
            return web.json_response({
                "success": False,
                "message": str(e)
            }, status=400)
    
    async def generate_attendance_code(self, request):
        try:
            data = await request.json()
            room_id = data.get("room_id", "")
            duration_minutes = data.get("duration_minutes", 5)
            
            if not room_id:
                return web.json_response({
                    "success": False,
                    "message": "room_id is required"
                }, status=400)
            
            if room_id not in self.classrooms:
                return web.json_response({
                    "success": False,
                    "message": "Classroom not found"
                }, status=404)
            
            classroom = self.classrooms[room_id]
            
            # Generate unique code
            code = f"ATT-{str(uuid.uuid4())[:8].upper()}"
            expiry = datetime.now() + timedelta(minutes=duration_minutes)
            
            classroom.attendance_codes[code] = {
                "expiry": expiry,
                "students": set(),
                "created_at": datetime.now().isoformat()
            }
            
            # Generate QR code
            qr_data = {
                "type": "attendance",
                "room_id": room_id,
                "code": code,
                "expiry": expiry.isoformat()
            }
            
            qr = qrcode.QRCode(version=1, box_size=10, border=5)
            qr.add_data(json.dumps(qr_data))
            qr.make(fit=True)
            img = qr.make_image(fill_color="black", back_color="white")
            
            buffered = BytesIO()
            img.save(buffered, format="PNG")
            qr_base64 = base64.b64encode(buffered.getvalue()).decode("utf-8")
            
            # Broadcast QR code via WebSocket
            await self.broadcast_qr_code(room_id, qr_base64, code, expiry)
            
            return web.json_response({
                "success": True,
                "data": {
                    "code": code,
                    "expiry": expiry.isoformat(),
                    "qr_code": qr_base64
                }
            })
        except Exception as e:
            return web.json_response({
                "success": False,
                "message": str(e)
            }, status=400)
    
    # Request endpoints
    async def list_requests(self, request):
        room_id = request.query.get("room_id", "")
        
        if not room_id:
            return web.json_response({
                "success": False,
                "message": "room_id is required"
            }, status=400)
        
        if room_id not in self.classrooms:
            return web.json_response({
                "success": False,
                "message": "Classroom not found"
            }, status=404)
        
        classroom = self.classrooms[room_id]
        
        # Return only pending requests
        pending_requests = [req for req in classroom.speak_requests if req["status"] == RequestStatus.PENDING.value]
        
        return web.json_response({
            "success": True,
            "data": pending_requests
        })
    
    async def create_request(self, request):
        try:
            data = await request.json()
            room_id = data.get("room_id", "")
            student_id = data.get("student_id", "")
            student_name = data.get("student_name", "")
            
            if not all([room_id, student_id, student_name]):
                return web.json_response({
                    "success": False,
                    "message": "Missing required fields"
                }, status=400)
            
            if room_id not in self.classrooms:
                return web.json_response({
                    "success": False,
                    "message": "Classroom not found"
                }, status=404)
            
            classroom = self.classrooms[room_id]
            speak_request = classroom.add_speak_request(student_id, student_name)
            
            # Broadcast new speak request via WebSocket
            await self.broadcast_speak_request(room_id, speak_request)
            
            return web.json_response({
                "success": True,
                "data": speak_request
            })
        except Exception as e:
            return web.json_response({
                "success": False,
                "message": str(e)
            }, status=400)
    
    async def update_request(self, request):
        try:
            data = await request.json()
            room_id = data.get("room_id", "")
            request_id = data.get("request_id", "")
            action = data.get("action", "")  # "approve" or "reject"
            
            if not all([room_id, request_id, action]):
                return web.json_response({
                    "success": False,
                    "message": "Missing required fields"
                }, status=400)
            
            if room_id not in self.classrooms:
                return web.json_response({
                    "success": False,
                    "message": "Classroom not found"
                }, status=404)
            
            classroom = self.classrooms[room_id]
            
            if action == "approve":
                success = classroom.approve_speak_request(request_id)
            elif action == "reject":
                success = classroom.reject_speak_request(request_id)
            else:
                return web.json_response({
                    "success": False,
                    "message": "Invalid action"
                }, status=400)
            
            if success:
                # Broadcast request update via WebSocket
                await self.broadcast_request_update(room_id, request_id, action)
                
                return web.json_response({
                    "success": True,
                    "message": f"Request {action}d successfully"
                })
            else:
                return web.json_response({
                    "success": False,
                    "message": "Request not found"
                }, status=404)
        except Exception as e:
            return web.json_response({
                "success": False,
                "message": str(e)
            }, status=400)
    
    # WebSocket broadcast methods
    async def broadcast_attendance_update(self, room_id, attendance_record):
        if room_id in self.classrooms:
            classroom = self.classrooms[room_id]
            message = {
                "type": "attendance_update",
                "data": attendance_record
            }
            
            for client in classroom.clients:
                if client.type == ClientType.DESKTOP:
                    await client.websocket.send(json.dumps(message))
    
    async def broadcast_qr_code(self, room_id, qr_base64, code, expiry):
        if room_id in self.classrooms:
            classroom = self.classrooms[room_id]
            message = {
                "type": "attendance_qr_code",
                "data": {
                    "qr_code": qr_base64,
                    "code": code,
                    "expiry": expiry.isoformat()
                }
            }
            
            for client in classroom.clients:
                if client.type == ClientType.DESKTOP:
                    await client.websocket.send(json.dumps(message))
    
    async def broadcast_speak_request(self, room_id, speak_request):
        if room_id in self.classrooms:
            classroom = self.classrooms[room_id]
            message = {
                "type": "speak_request",
                "data": speak_request
            }
            
            for client in classroom.clients:
                if client.type == ClientType.DESKTOP:
                    await client.websocket.send(json.dumps(message))
    
    async def broadcast_request_update(self, room_id, request_id, action):
        if room_id in self.classrooms:
            classroom = self.classrooms[room_id]
            message = {
                "type": "request_update",
                "data": {
                    "request_id": request_id,
                    "action": action
                }
            }
            
            for client in classroom.clients:
                await client.websocket.send(json.dumps(message))

    # WebSocket handlers (existing code)
    async def register(self, websocket):
        """Register a new client connection"""
        logging.info("New connection opened")
        self.connections[websocket] = ClientInfo(websocket)

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
        success, message, auth_data = self.user_db.authenticate_user(name, password)
        
        if success:
            user_id = auth_data["user_id"]
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
        success, message, auth_data = self.user_db.register_user(name, email, password)
        
        response = {
            "type": "register_response",
            "success": success,
            "message": message
        }
        
        if success:
            response["user_id"] = auth_data["user_id"]
        
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
                    
                    # Handle speak request gestures
                    if gesture_type == "THUMB_UP":
                        # Approve the current speak request
                        current_request = classroom.get_next_speak_request()
                        if current_request:
                            classroom.approve_speak_request(current_request["id"])
                            await self.broadcast_request_update(room_id, current_request["id"], "approve")
                    elif gesture_type == "THUMB_DOWN":
                        # Reject the current speak request
                        current_request = classroom.get_next_speak_request()
                        if current_request:
                            classroom.reject_speak_request(current_request["id"])
                            await self.broadcast_request_update(room_id, current_request["id"], "reject")
                    
                    # Broadcast gesture to all clients in the room
                    gesture_message = {
                        "type": "gesture",
                        "client_id": client.id,
                        "gesture_type": gesture_type,
                        "timestamp": timestamp
                    }
                    
                    for room_client in classroom.clients:
                        if room_client.websocket != websocket:  # Don't send back to sender
                            await room_client.websocket.send(json.dumps(gesture_message))
                    
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
                
                confirmation = {
                    "type": "room_joined",
                    "room_id": room_id,
                    "client_id": client.id
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
            while True:
                try:
                    message = await websocket.recv()
                    await self.handle_message(websocket, message)
                except websockets.ConnectionClosed as e:
                    logging.info(f"Connection closed: {e.code} - {e.reason}")
                    break
                except Exception as e:
                    logging.error(f"Error handling message: {str(e)}")
                    error = {"type": "error", "message": f"Error processing message: {str(e)}"}
                    await websocket.send(json.dumps(error))
        finally:
            await self.unregister(websocket)

    async def run_websocket_server(self, host="0.0.0.0", port=9090):
        """Run the WebSocket server"""
        try:
            logging.info(f"Starting WebSocket server on {host}:{port}")
            async with websockets.serve(self.handle_connection, host, port):
                logging.info(f"WebSocket server started successfully on {host}:{port}")
                await asyncio.Future()  # Run forever
        except OSError as e:
            logging.error(f"Failed to start WebSocket server on port {port}: {e}")
            raise

    def create_rest_app(self):
        """Create the REST API application"""
        app = web.Application()
        
        # Setup CORS
        cors = aiohttp_cors.setup(app, defaults={
            "*": aiohttp_cors.ResourceOptions(
                allow_credentials=True,
                expose_headers="*",
                allow_headers="*",
                allow_methods="*"
            )
        })
        
        # Authentication routes
        app.router.add_post('/auth/login', self.login)
        app.router.add_post('/auth/register', self.register)
        
        # Classroom routes
        app.router.add_get('/classroom', self.list_classrooms)
        app.router.add_post('/classroom', self.create_classroom)
        app.router.add_put('/classroom', self.update_classroom)
        app.router.add_delete('/classroom', self.delete_classroom)
        
        # Attendance routes
        app.router.add_get('/attendance', self.list_attendance)
        app.router.add_post('/attendance', self.mark_attendance)
        app.router.add_post('/attendance/code', self.generate_attendance_code)
        
        # Request routes
        app.router.add_get('/request', self.list_requests)
        app.router.add_post('/request', self.create_request)
        app.router.add_put('/request', self.update_request)
        
        # Configure CORS on all routes
        for route in list(app.router.routes()):
            cors.add(route)
        
        return app

async def run_servers():
    """Run both WebSocket and REST API servers"""
    server = ClassroomServer()
    
    # Find free ports for both servers
    ws_port = find_free_port(start_port=9091)
    rest_port = find_free_port(start_port=9090)
    
    if not ws_port or not rest_port:
        logging.error("Could not find free ports for servers")
        sys.exit(1)
    
    # Create REST API app
    rest_app = server.create_rest_app()
    
    # Create tasks for both servers
    ws_task = asyncio.create_task(server.run_websocket_server(port=ws_port))
    
    # Run REST API server
    runner = web.AppRunner(rest_app)
    await runner.setup()
    site = web.TCPSite(runner, '0.0.0.0', rest_port)
    await site.start()
    
    logging.info(f"REST API server started on http://0.0.0.0:{rest_port}")
    logging.info(f"WebSocket server started on ws://0.0.0.0:{ws_port}")
    
    # Wait for WebSocket server
    await ws_task

if __name__ == "__main__":
    try:
        asyncio.run(run_servers())
    except KeyboardInterrupt:
        logging.info("Servers stopped by user")
    except Exception as e:
        logging.error(f"Error: {e}")
        sys.exit(1)