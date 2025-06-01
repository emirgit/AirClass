# AirClass - Interactive Classroom System

[![GTU](https://img.shields.io/badge/Institution-Gebze%20Teknik%20Üniversitesi-blue)](https://www.gtu.edu.tr/en/)
[![Course](https://img.shields.io/badge/Course-CSE%20396%20Computer%20Engineering%20Project-green)](https://www.gtu.edu.tr/en/)
[![Term](https://img.shields.io/badge/Term-Spring%202025-brightgreen)](https://www.gtu.edu.tr/en/)
[![Group](https://img.shields.io/badge/Group-14-lightgrey)](https://www.gtu.edu.tr/en/)

AirClass is an integrated system designed to create an interactive and intelligent classroom environment. It allows instructors to control presentations via hand gestures and face tracking, manage student attendance, and enable real-time student interaction. The project consists of three core modules: Hardware, Desktop Application, and Mobile Application, all working in synergy.

## Table of Contents

1.  [Overview](#overview)
2.  [Features](#features)
3.  [System Architecture](#system-architecture)
    *   [Hardware Module](#hardware-module)
    *   [Desktop Application Module](#desktop-application-module)
    *   [Mobile Application Module](#mobile-application-module)
    *   [Inter-Module Communication](#inter-module-communication)
4.  [Technology Stack](#technology-stack)
5.  [Project Team](#project-team)
6.  [User Manual](#user-manual)
    *   [Hardware Module: Gesture Commands](#hardware-module-gesture-commands)
    *   [Desktop Application Usage](#desktop-application-usage)
    *   [Mobile Application Usage](#mobile-application-usage)
7.  [Repository Links](#repository-links)
8.  [Future Development](#future-development)

## Overview

The AirClass project aims to enhance the traditional classroom experience by leveraging modern technology. Instructors can deliver presentations more dynamically using gesture controls, while students can engage more actively through a dedicated mobile application. The system handles:
*   **Gesture-based presentation control:** Navigate slides, zoom, and manage interactions hands-free.
*   **Automated attendance:** QR code-based attendance tracking.
*   **Student interaction:** Speak requests and real-time feedback.
*   **Session management:** Secure and persistent sessions for instructors and students.

## Features

**General:**
*   Seamless integration of hardware, desktop, and mobile components.
*   Real-time communication and synchronization across modules.
*   Intuitive user interfaces for both instructors and students.

**Hardware Module:**
*   Real-time hand detection and gesture recognition (powered by MediaPipe).
*   Face detection-based camera tracking to keep the instructor in frame.
*   Robust performance on Raspberry Pi 5.

**Desktop Application Module (Qt Framework):**
*   Comprehensive classroom management interface.
*   Multi-format presentation rendering (PDF, PPTX with auto-conversion).
*   QR code generation for attendance and session joining.
*   Real-time attendance list and export functionality.
*   Speak request management from students.
*   Gesture guide and configuration.
*   Timeout management based on instructor presence.

**Mobile Application Module (Cross-Platform):**
*   Secure student authentication and user management.
*   QR code scanning for session joining and attendance.
*   Real-time speak request submission.
*   PDF presentation viewer to follow along.
*   Notifications for classroom events and instructor feedback.

## System Architecture

AirClass is composed of three main modules that communicate to provide a cohesive experience.

### Hardware Module

Responsible for real-time gesture recognition and face detection.
*   **Components:** Raspberry Pi 5, Camera Module v2, PCA9685 Servo Motor Driver, MG90S servo motors.
*   **Core Logic:** Uses Google MediaPipe for hand landmark extraction and gesture classification. A pre-trained model (using the Hagrid 512p dataset) identifies various hand gestures. Face detection ensures the camera tracks the instructor.
*   **Output:** Recognized gestures are sent as commands to the Desktop Application.

### Desktop Application Module

The central hub for the instructor, built using the Qt Framework.
*   **Core Logic:** Manages presentations (displaying PDFs, converting PPTX), generates session-specific QR codes for attendance, displays a real-time list of attendees, and handles speak requests from students. It receives gesture commands from the Hardware module and interacts with the Mobile Application via a server.
*   **Key Features:** Dashboard with recent presentations, student interaction panel, QR code generator, presentation view with drawing/annotation tools.

### Mobile Application Module

The student-facing component, designed for cross-platform compatibility.
*   **Core Logic:** Allows students to register/login, join a class session by scanning a QR code or entering a code, mark attendance, view the presentation, and send speak requests to the instructor.
*   **Key Features:** Authentication, QR scanner, real-time connection to the desktop session, PDF viewer, speak request interface.

### Inter-Module Communication

*   **Hardware-to-Desktop:** The Raspberry Pi (Hardware) broadcasts a private key. The Desktop application listens for this key. Upon detection, the Desktop sends its IP to the Pi. The Pi then establishes a WebSocket connection to the Desktop and continuously transmits recognized hand gestures.
*   **Desktop-to-Mobile (via Server):** The Desktop application acts as a local server or communicates through a central server. It generates session codes and QR codes. Student speak requests from the Mobile app are routed via the server to the corresponding Desktop instance. Attendance information is synchronized.
*   **Mobile-to-Desktop (via Server):** Students use the Mobile app to send speak requests and attendance confirmations to the Desktop application via the central server infrastructure.

## Technology Stack

*   **Hardware:** Raspberry Pi 5, Python, Google MediaPipe, OpenCV, PCA9685.
*   **Desktop:** C++, Qt Framework, QPdfDocument, WebSockets.
*   **Mobile:** Cross-Platform Framework (e.g., Flutter, React Native, or native Android/iOS with shared KMM/C++ core - *document doesn't specify, assuming a general cross-platform approach*), REST APIs, WebSockets.
*   **Dataset (Gesture Recognition):** Hagrid 512p.
*   **Machine Learning:** Keras/TensorFlow (for gesture model training).

## Project Team (Group-14)

| Student Name          | Student Number | Primary Module(s)         |
| --------------------- | -------------- | ------------------------- |
| Muhammed Emir Kara    | 210104004071   | Hardware                  |
| Ahmet Mücahit Gündüz  | 210104004092   | Hardware                  |
| Veysel Cemaloğlu      | 200104004107   | Hardware                  |
| Ahmet Can Hatipoğlu   | 220104004921   | Hardware, Mobile          |
| Helin Saygılı         | 240104004980   | Desktop                   |
| Ahmet Sadri Güler     | 200104004015   | Desktop                   |
| Kenan Eren Ayyılmaz   | 200104004068   | Desktop                   |
| Selin Bardakçı        | 220104004015   | Mobile                    |
| Umut Hüseyin Satır    | 210104004074   | Mobile                    |

## User Manual

### Hardware Module: Gesture Commands

The hardware module recognizes the following gestures, which control the Desktop Application:

| Gesture Name       | Action                                                                    | In Zoom Mode                                   |
| :----------------- | :------------------------------------------------------------------------ | :--------------------------------------------- |
| "three_gun"        | Next Slide                                                                | Move Page Left                                 |
| "inv_three_gun"    | Previous Slide                                                            | Move Page Right                                |
| "ok"               | Changes mode to "Zoom Mode" (first detection), then "Zoom In" action occurs. | N/A                                            |
| "like"             | Give Permission to Student to Speak                                       | Move Page Up                                   |
| "dislike"          | Take Permission from Student to Speak                                     | Move Page Down                                 |
| "palm"             | Play Warning Sound to Silence Students                                    | Reset Zoom and Deactivate Zoom Mode            |
| "rock"             | Give 10 Minutes Break to Lecture                                          | N/A                                            |
| "take_picture"     | Open QR Code for Attendance                                               | N/A                                            |

*(Visuals for gestures can be found in the "Test and Conclusion Report" document, pages 34-35)*

### Desktop Application Usage

1.  **Dashboard (Figure 2.a):**
    *   Upon launching, the instructor sees the dashboard.
    *   A session code is displayed (e.g., "Code: 968822") for students to join.
    *   Recent presentations can be quickly accessed.
    *   Option to "Open Presentation..." to load a new PDF/PPTX file.
    *   "Gesture Controls" section shows registered gestures.

2.  **Student View Page (Figure 2.b):**
    *   Displays current attendance.
    *   Allows downloading attendance reports.
    *   Shows incoming "Speak Requests" from students, which can be approved or rejected.

3.  **QR Code Generator (Figure 2.c):**
    *   Accessible via the "take_picture" gesture or a UI button.
    *   Generates a unique QR code for the current session.
    *   Students scan this QR code using the mobile app to mark attendance.
    *   Timer functionality can be set for the QR code validity.

4.  **Presentation Page (Figure 2.d):**
    *   Displays the active presentation (PDF/PPTX).
    *   Supports navigation (next/previous slide), zooming, and drawing/annotations.
    *   Can be controlled by hardware gestures or direct UI interaction.

### Mobile Application Usage

1.  **Main Page / Welcome (Figure 3.a):**
    *   New users select "Register".
    *   Existing users select "Login".

2.  **Register Page (Figure 3.b):**
    *   Users enter their Name, Email, and Password to create an account.

3.  **Login Page (Figure 3.c):**
    *   Users enter their registered Email and Password to sign in.

4.  **Class Code Page (Figure 3.d):**
    *   After logging in, students are prompted to "Enter Class Code".
    *   This code is provided by the instructor (displayed on the Desktop App Dashboard).

5.  **In-Session View (Figure 3.f):**
    *   Once the class code is entered, the student joins the session.
    *   The current presentation slide (PDF) shared by the instructor is visible.
    *   **Request to Speak:** Button to send a request to the instructor.
    *   **Mark Attendance:** Button to scan the QR code displayed by the instructor for attendance. Upon clicking, the QR code scanner screen will appear.
    *   Navigation tabs might show "Class" (current view) and "Files" (if other shared resources are available).

*(Screenshots for Desktop and Mobile UI can be found in the "Test and Conclusion Report" document, pages 37-46)*

## Repository Links

*   **Main AirClass Combined Repository:** [https://github.com/emirgit/AirClass](https://github.com/emirgit/AirClass) (This repository)
*   **Hardware Module:** [https://github.com/emirgit/AirClass-Hardware](https://github.com/emirgit/AirClass-Hardware)
*   **Desktop Module:** [https://github.com/emirgit/AirClass-Desktop](https://github.com/emirgit/AirClass-Desktop)
*   **Mobile Module:** [https://github.com/emirgit/AirClass-Mobile](https://github.com/emirgit/AirClass-Mobile)

## Future Development

Based on the "Test and Conclusion Report":
*   **Desktop & General:**
    *   Enhanced analytics and reporting capabilities.
    *   Advanced presentation annotation tools.
    *   Integration with Learning Management Systems (LMS).
    *   Extended multi-format document support.
    *   Advanced gesture customization and machine learning adaptation.
*   **Mobile:**
    *   Enhanced offline capabilities.
    *   Expanded notification features.
    *   Additional student engagement tools leveraging mobile platform capabilities.
