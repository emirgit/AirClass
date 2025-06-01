import React, { useState, useEffect } from "react";
import { View, TouchableOpacity, Text, StyleSheet, Alert } from "react-native";
import AsyncStorage from "@react-native-async-storage/async-storage";
import { MarkAttendance } from "./MarkAttendance";
import { SlideControl } from "./SlideControl";

interface RequestToSpeakProps {
    studentId: string;
    onRequestAccepted: () => void;
}

// Mock data for request status
interface RequestStatus {
    id?: string;
    status: "pending" | "accepted" | "rejected";
    timestamp: string;
    position?: number;
    estimatedWaitTime?: string;
}

export function RequestToSpeak({
    studentId,
    onRequestAccepted,
}: RequestToSpeakProps) {
    const [isRequestSent, setIsRequestSent] = useState(false);
    const [requestStatus, setRequestStatus] = useState<RequestStatus | null>(
        null
    );
    const [isLoading, setIsLoading] = useState(false);
    const [classroomId, setClassroomId] = useState<string | null>(null);

    useEffect(() => {
        const getClassroomId = async () => {
            try {
                const id = await AsyncStorage.getItem("classroomId");
                setClassroomId(id);
            } catch (error) {
                console.error("Error getting classroom ID:", error);
            }
        };
        getClassroomId();
    }, []);

    // Add new useEffect to check request status on mount
    useEffect(() => {
        const checkInitialRequestStatus = async () => {
            if (!classroomId) return;

            try {
                const token = await AsyncStorage.getItem("jwtToken");
                if (!token) return;

                console.log(
                    "Checking initial request status for classroom:",
                    classroomId
                );
                const response = await fetch(
                    `http://159.89.19.111/airclass-api/request/check?classroom_id=${classroomId}`,
                    {
                        method: "GET",
                        headers: {
                            Authorization: `Bearer ${token}`,
                        },
                    }
                );

                const data = await response.json();
                console.log("Initial request check response:", data);

                if (response.ok && data.status === true) {
                    if (data.data.hasActiveRequest === true) {
                        console.log(
                            "Found active request, showing SlideControl"
                        );
                        setIsRequestSent(true);
                        setRequestStatus({
                            id: data.data.request_id?.toString() || undefined,
                            status: "accepted",
                            timestamp: new Date().toISOString(),
                        });
                        onRequestAccepted();
                    } else {
                        console.log("Found pending request");
                        setIsRequestSent(true);
                        setRequestStatus({
                            id: data.data.request_id?.toString() || undefined,
                            status: "pending",
                            timestamp: new Date().toISOString(),
                        });
                    }
                } else {
                    console.log("No active request found");
                    setIsRequestSent(false);
                    setRequestStatus(null);
                }
            } catch (error) {
                console.error("Error checking initial request status:", error);
                setIsRequestSent(false);
                setRequestStatus(null);
            }
        };

        checkInitialRequestStatus();
    }, [classroomId, onRequestAccepted]);

    const handleRequest = async () => {
        if (!classroomId) {
            Alert.alert("Error", "No classroom selected");
            return;
        }

        setIsLoading(true);
        try {
            const token = await AsyncStorage.getItem("jwtToken");
            if (!token) {
                Alert.alert("Error", "Please log in to request to speak");
                return;
            }

            const response = await fetch(
                "http://159.89.19.111/airclass-api/request",
                {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json",
                        Authorization: `Bearer ${token}`,
                    },
                    body: JSON.stringify({
                        classroom_id: classroomId,
                    }),
                }
            );

            const data = await response.json();
            console.log("Create request response:", data);
            if (response.ok && data.data?.request_id) {
                setIsRequestSent(true);
                setRequestStatus({
                    id: data.data.request_id.toString(),
                    status: "pending",
                    timestamp: new Date().toISOString(),
                });
            } else {
                Alert.alert("Error", data.message || "Failed to send request");
            }
        } catch (error) {
            console.error("Error sending request:", error);
            Alert.alert("Error", "Network error. Please try again.");
        } finally {
            setIsLoading(false);
        }
    };

    const handleCheckRequest = async () => {
        if (!classroomId) {
            Alert.alert("Error", "No classroom selected");
            return;
        }

        setIsLoading(true);
        try {
            const token = await AsyncStorage.getItem("jwtToken");
            if (!token) {
                Alert.alert("Error", "Please log in to check request status");
                return;
            }

            console.log("Checking request status for classroom:", classroomId);
            const response = await fetch(
                `http://159.89.19.111/airclass-api/request/check?classroom_id=${classroomId}`,
                {
                    method: "GET",
                    headers: {
                        Authorization: `Bearer ${token}`,
                    },
                }
            );

            const data = await response.json();
            console.log("Request check response:", data);

            if (response.ok) {
                if (
                    data.status === true &&
                    data.data.hasActiveRequest === true
                ) {
                    console.log("Request is accepted, showing SlideControl");
                    setRequestStatus({
                        id: data.data.request_id?.toString() || undefined,
                        status: "accepted",
                        timestamp: new Date().toISOString(),
                    });
                    onRequestAccepted();
                } else if (
                    data.status === true &&
                    data.data.hasActiveRequest === false
                ) {
                    console.log("Request is pending");
                    setRequestStatus({
                        id: data.data.request_id?.toString() || undefined,
                        status: "pending",
                        timestamp: new Date().toISOString(),
                    });
                } else {
                    console.log("Request is rejected");
                    setRequestStatus({
                        id: data.data.request_id?.toString() || undefined,
                        status: "rejected",
                        timestamp: new Date().toISOString(),
                    });
                }
            } else {
                console.log("Request check failed:", data.message);
                Alert.alert(
                    "Error",
                    data.message || "Failed to check request status"
                );
            }
        } catch (error) {
            console.error("Error checking request:", error);
            Alert.alert("Error", "Network error. Please try again.");
        } finally {
            setIsLoading(false);
        }
    };

    const handleCancelRequest = async () => {
        if (!requestStatus?.id) {
            // If we don't have a request ID, we can't cancel the request
            // Just reset the state since the request might have been cancelled already
            setIsRequestSent(false);
            setRequestStatus(null);
            return;
        }

        setIsLoading(true);
        try {
            const token = await AsyncStorage.getItem("jwtToken");
            if (!token) {
                Alert.alert("Error", "Please log in to cancel request");
                return;
            }

            const response = await fetch(
                "http://159.89.19.111/airclass-api/request",
                {
                    method: "DELETE",
                    headers: {
                        "Content-Type": "application/json",
                        Authorization: `Bearer ${token}`,
                    },
                    body: JSON.stringify({ id: requestStatus.id }),
                }
            );

            const data = await response.json();
            console.log("Cancel request response:", data);
            if (response.ok) {
                setIsRequestSent(false);
                setRequestStatus(null);
            } else {
                Alert.alert(
                    "Error",
                    data.message || "Failed to cancel request"
                );
            }
        } catch (error) {
            console.error("Error canceling request:", error);
            Alert.alert("Error", "Network error. Please try again.");
        } finally {
            setIsLoading(false);
        }
    };

    return (
        <View style={styles.container}>
            {!isRequestSent ? (
                <>
                    <TouchableOpacity
                        style={[
                            styles.button,
                            isLoading && styles.buttonDisabled,
                        ]}
                        onPress={handleRequest}
                        disabled={isLoading}
                    >
                        <Text style={styles.buttonText}>
                            {isLoading ? "Sending..." : "Request to Speak"}
                        </Text>
                    </TouchableOpacity>
                    <MarkAttendance />
                </>
            ) : requestStatus?.status === "accepted" ? (
                <SlideControl />
            ) : (
                <View style={styles.feedbackContainer}>
                    <Text style={styles.feedbackText}>
                        {requestStatus?.status === "pending"
                            ? "Request Pending"
                            : "Request Status"}
                    </Text>
                    <View style={styles.buttonRow}>
                        <TouchableOpacity
                            style={[
                                styles.secondaryButton,
                                isLoading && styles.buttonDisabled,
                            ]}
                            onPress={handleCheckRequest}
                            disabled={isLoading}
                        >
                            <Text style={styles.secondaryButtonText}>
                                {isLoading ? "Checking..." : "Check Request"}
                            </Text>
                        </TouchableOpacity>
                        <TouchableOpacity
                            style={[
                                styles.dangerButton,
                                isLoading && styles.buttonDisabled,
                            ]}
                            onPress={handleCancelRequest}
                            disabled={isLoading}
                        >
                            <Text style={styles.dangerButtonText}>
                                {isLoading ? "Canceling..." : "Cancel Request"}
                            </Text>
                        </TouchableOpacity>
                    </View>
                    {requestStatus?.status === "pending" && (
                        <Text style={styles.statusText}>
                            Waiting for instructor's approval...
                        </Text>
                    )}
                </View>
            )}
        </View>
    );
}

const styles = StyleSheet.create({
    container: {
        padding: 4,
        gap: 12,
    },
    button: {
        backgroundColor: "#4F46E5",
        paddingVertical: 12,
        paddingHorizontal: 24,
        borderRadius: 8,
        alignItems: "center",
    },
    buttonDisabled: {
        opacity: 0.7,
    },
    buttonText: {
        color: "#fff",
        fontSize: 16,
        fontWeight: "600",
    },
    feedbackContainer: {
        backgroundColor: "#EEF2FF",
        padding: 10,
        borderRadius: 8,
        alignItems: "center",
        gap: 12,
    },
    feedbackText: {
        color: "#4F46E5",
        fontSize: 16,
        fontWeight: "500",
    },
    statusText: {
        color: "#818CF8",
        fontSize: 14,
         paddingTop: 6, // reduce from default
    paddingBottom: 14, 

    },
    buttonRow: {
        flexDirection: "row",
        gap: 8,
        width: "100%",
    },
    secondaryButton: {
        flex: 1,
        backgroundColor: "#EEF2FF",
        paddingVertical: 12,
        paddingHorizontal: 16,
        borderRadius: 8,
        alignItems: "center",
        borderWidth: 1,
        borderColor: "#4F46E5",
    },
    secondaryButtonText: {
        color: "#4F46E5",
        fontSize: 14,
        fontWeight: "600",
    },
    dangerButton: {
        flex: 1,
        backgroundColor: "#FEE2E2",
        paddingVertical: 12,
        paddingHorizontal: 16,
        borderRadius: 8,
        alignItems: "center",
        borderWidth: 1,
        borderColor: "#DC2626",
    },
    dangerButtonText: {
        color: "#DC2626",
        fontSize: 14,
        fontWeight: "600",
    },
});
