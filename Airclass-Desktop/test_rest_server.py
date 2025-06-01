from flask import Flask, jsonify, request
from flask_cors import CORS
import time

app = Flask(__name__)
CORS(app)  # Enable CORS for all routes

# Simulated database
users = {
    "test@example.com": {
        "password": "123456",
        "name": "Test User"
    }
}

@app.route('/api/v1/auth/login', methods=['POST'])
def login():
    data = request.json
    email = data.get('email')
    password = data.get('password')
    
    if email in users and users[email]['password'] == password:
        return jsonify({
            "success": True,
            "data": {
                "name": users[email]['name'],
                "token": f"test_token_{int(time.time())}"
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
    
    return jsonify({
        "success": True,
        "data": {
            "name": name,
            "token": f"test_token_{int(time.time())}"
        }
    })

@app.route('/api/v1/attendance/list', methods=['GET'])
def list_attendance():
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

@app.route('/api/v1/attendance/generate', methods=['POST'])
def generate_code():
    data = request.json
    room_id = data.get('room_id')
    duration = data.get('duration', 5)
    
    return jsonify({
        "success": True,
        "data": {
            "code": "ABC123",
            "qr_code": "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==",  # Base64 encoded sample QR
            "expiry": "2024-01-23 11:00:00"
        }
    })

if __name__ == '__main__':
    app.run(host='localhost', port=9090, debug=True) 