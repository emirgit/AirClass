from flask import Flask, jsonify, request
from flask_cors import CORS
import time
import uuid
from datetime import datetime, timedelta
import random
import string

app = Flask(__name__)
CORS(app)  # Enable CORS for all routes

# Simulated database
users = {
    "test@example.com": {
        "password": "123456",
        "name": "Test User"
    }
}

# Session storage
sessions = {}  # Changed from predefined dict to empty dict for dynamic session storage

# Track user tokens and active sessions
user_tokens = {}
active_sessions = {}  # Track active sessions separately from user tokens

@app.route('/api/v1/auth/login', methods=['POST'])
def login():
    data = request.json
    email = data.get('email')
    password = data.get('password')
    
    if email in users and users[email]['password'] == password:
        token = f"test_token_{int(time.time())}"
        user_tokens[token] = email
        
        # Check if user has an active session
        active_session = None
        for session in sessions.values():
            if session['created_by'] == email and session['status'] == 1:
                active_session = session
                break
        
        return jsonify({
            "success": True,
            "data": {
                "name": users[email]['name'],
                "email": email,
                "token": token,
                "active_session": active_session
            }
        })
    
    return jsonify({
        "success": False,
        "message": "Invalid email or password"
    }), 401

@app.route('/api/v1/auth/register', methods=['POST'])
def register():
    data = request.json
    email = data.get('email')
    password = data.get('password')
    name = data.get('name')
    
    if email in users:
        return jsonify({
            "success": False,
            "message": "Email already registered"
        }), 400
    
    users[email] = {
        "password": password,
        "name": name
    }
    
    token = f"test_token_{int(time.time())}"
    user_tokens[token] = email
    
    return jsonify({
        "success": True,
        "data": {
            "name": name,
            "email": email,
            "token": token
        }
    })

def get_user_from_token(request):
    auth_header = request.headers.get('Authorization')
    if not auth_header or not auth_header.startswith('Bearer '):
        return None
    
    token = auth_header.split(' ')[1]
    return user_tokens.get(token)

@app.route('/api/v1/session/list', methods=['GET'])
def list_sessions():
    user_email = get_user_from_token(request)
    if not user_email:
        return jsonify({"success": False, "message": "Unauthorized"}), 401
    
    # Return all sessions for the user
    user_sessions = [session for session in sessions.values() 
                     if session['created_by'] == user_email]
    
    return jsonify({
        "success": True,
        "data": user_sessions
    })

@app.route('/api/v1/session/active', methods=['GET'])
def get_active_session():
    user_email = get_user_from_token(request)
    if not user_email:
        return jsonify({"success": False, "message": "Unauthorized"}), 401
    
    # Find active session (status = 1)
    active_session = None
    for session in sessions.values():
        if session['created_by'] == user_email and session['status'] == 1:
            active_session = session
            break
    
    return jsonify({
        "success": True,
        "data": active_session  # Will be null if no active session
    })

def generate_session_code():
    """Generate a random 6-digit code"""
    return ''.join(random.choices(string.digits, k=6))

@app.route('/api/v1/session/create', methods=['POST'])
def create_session():
    try:
        user_email = get_user_from_token(request)
        if not user_email:
            return jsonify({"success": False, "message": "Unauthorized"}), 401

        data = request.get_json()
        
        if not data or 'name' not in data:
            return jsonify({'success': False, 'message': 'Session name is required'}), 400
        
        session_name = data['name']
        
        # Generate a unique 6-digit code
        while True:
            session_code = generate_session_code()
            # Check if code already exists
            if not any(s['code'] == session_code for s in sessions.values()):
                break
        
        session_id = str(uuid.uuid4())
        new_session = {
            'id': session_id,
            'code': session_code,
            'name': session_name,
            'status': 1,  # 0: inactive, 1: active
            'student_count': 0,
            'created_by': user_email,
            'created_at': datetime.now().isoformat()
        }
        
        sessions[session_id] = new_session
        
        return jsonify({
            'success': True,
            'message': 'Session created successfully',
            'data': new_session
        })
        
    except Exception as e:
        return jsonify({'success': False, 'message': str(e)}), 500

@app.route('/api/v1/session/activate/<session_id>', methods=['PUT'])
def activate_session(session_id):
    user_email = get_user_from_token(request)
    if not user_email:
        return jsonify({"success": False, "message": "Unauthorized"}), 401
    
    if session_id not in sessions:
        return jsonify({
            "success": False,
            "message": "Session not found"
        }), 404
    
    session = sessions[session_id]
    if session['created_by'] != user_email:
        return jsonify({
            "success": False,
            "message": "You don't have permission to modify this session"
        }), 403
    
    # Activate this session
    session['status'] = 1
    active_sessions[user_email] = session_id
    
    return jsonify({
        "success": True,
        "data": session
    })

@app.route('/api/v1/session/close/<session_id>', methods=['PUT'])
def deactivate_session(session_id):
    user_email = get_user_from_token(request)
    if not user_email:
        return jsonify({"success": False, "message": "Unauthorized"}), 401
    
    if session_id not in sessions:
        return jsonify({
            "success": False,
            "message": "Session not found"
        }), 404
    
    session = sessions[session_id]
    if session['created_by'] != user_email:
        return jsonify({
            "success": False,
            "message": "You don't have permission to modify this session"
        }), 403
    
    # Deactivate the session and remove from active sessions
    session['status'] = 0
    if user_email in active_sessions:
        del active_sessions[user_email]
    
    return jsonify({
        "success": True,
        "data": session
    })

@app.route('/api/v1/attendance/list', methods=['GET'])
def list_attendance():
    room_id = request.args.get('room_id')
    return jsonify({
        "success": True,
        "data": [
            {
                "student_id": "1",
                "student_name": "John Doe",
                "timestamp": "2024-01-23 10:30:00",
                "code": "ABC123"
            },
            {
                "student_id": "2",
                "student_name": "Jane Smith",
                "timestamp": "2024-01-23 10:31:00",
                "code": "ABC123"
            }
        ]
    })

@app.route('/api/v1/attendance/code', methods=['POST'])
def generate_code():
    data = request.json
    room_id = data.get('room_id')
    duration = data.get('duration_minutes', 5)
    
    return jsonify({
        "success": True,
        "data": {
            "code": "ABC123",
            "qr_code": "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==",
            "expiry": "2024-01-23 11:00:00"
        }
    })

@app.route('/api/v1/request', methods=['GET'])
def list_requests():
    room_id = request.args.get('room_id')
    return jsonify({
        "success": True,
        "data": []
    })

if __name__ == '__main__':
    app.run(host='localhost', port=9090, debug=True)